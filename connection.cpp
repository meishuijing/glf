#include "connection.h"
#include "net.h"
#ifdef WIN32
#include "gsocket.h"
#else 
#include "lsocket.h"
#endif

CConnectionMgr* CConnectionMgr::Instance()
{
	static CConnectionMgr instance;
	return &instance;
}

static bool bInit = false;
bool CConnectionMgr::Init(IFxSessionFactory* pFactory)
{
	bInit = true;
	if (!m_poolConnection.Init(128, 64))
		return false;
	m_pFactory = pFactory;
	return true;
}

CConnection* CConnectionMgr::CreateConnection(FxNetModule* pNetModule)
{
	CConnection* pConnection = m_poolConnection.FetchObj();
	if (pConnection)
	{
		if (!pConnection->Init(pNetModule))
		{
			m_poolConnection.ReleaseObj(pConnection);
			pConnection = NULL;
			return NULL;
		}
		FxSession* pFxSession = m_pFactory->CreateSession();
		if (pFxSession)
		{
			pConnection->SetSession(pFxSession);
			pFxSession->SetConnection(pConnection);
			return pConnection;
		}
		else
		{
			m_poolConnection.ReleaseObj(pConnection);
			pConnection = NULL;
		}
	}
	return NULL;
}

void CConnectionMgr::Release(CConnection* pConnection)
{
	if (bInit)
	{
		m_poolConnection.ReleaseObj(pConnection);
		pConnection = NULL;
	}
	else
	{
		delete pConnection;
		pConnection = NULL;
	}
}

bool CConnection::Init(FxNetModule* pNetModule)
{
	m_poSocket = NULL;
	m_pFxSession = NULL;
	if (!m_eventNet.Init(MAX_NETEVENT_PERSOCK))
		return false;

	m_bClose = true;
	m_pNetModule = pNetModule;
	return true;
}

void CConnection::SetSession(FxSession* pFxSession)
{
	m_pFxSession = pFxSession;
}

FxSession* CConnection::GetSession()
{
	return m_pFxSession;
}

class CFuncLock
{
public:
	CFuncLock(FxLock* pLock)
	{
		pLock->Lock();
		m_pLock = pLock;
	}
	~CFuncLock()
	{
		m_pLock->Unlock();
	}

private:
	FxLock* m_pLock;

};

bool CConnection::ProcEvent()
{
	SNetEvent* pEvent = m_eventNet.PopFront();
	if (pEvent)
	{
		switch (pEvent->eType)
		{
		case NETEVT_CONNECTED:
			_ProcConnected();
			break;

		case NETEVT_RECV:
			_ProcRecv(pEvent->dwValue);
			break;

		case NETEVT_CLOSE:
			_ProcClose();
			break;

		case NETEVT_ERROR:
			_ProcError(pEvent->dwValue);
			break;

		default:
			return false;
		}
	}

	return true;
}

bool CConnection::PushEvent(ENetEvtType eType, UINT32 dwValue, bool bWaitFor)
{
	//不同线程都有可能pushevent ,所以要锁住m_bClose的修改
	m_oCloseLock.Lock();
	if (eType == NETEVT_CONNECTED)
	{
		m_bClose = false;
	}
	if (m_bClose)
	{
		m_oCloseLock.Unlock();
		return true;
	}
	if (eType == NETEVT_CLOSE)
	{
		m_bClose = true;
	}
	m_oCloseLock.Unlock();

	SNetEvent stEvent;
	stEvent.eType = eType;
	stEvent.dwValue = dwValue;
	while (!m_eventNet.PushBack(stEvent) && bWaitFor)
		Sleep(1);

	while (!m_pNetModule->PushEventQueue(this) && bWaitFor)
		Sleep(1);

	return true;
}

bool CConnection::_ProcConnected()
{
	m_pFxSession->OnConnect();
	return true;
}

bool CConnection::_ProcRecv(UINT32 dwLen)
{
	if (!m_oRecvBuffer.MemcpyBuf(m_pNetModule->GetMessBuffer(), dwLen))
	{
		//PushEvent(NETEVT_ERROR, NET_RECV_ERROR);
		m_oRecvBuffer.DiscardLen(dwLen);
	}
	else
	{
		m_pFxSession->OnRecv(m_pNetModule->GetMessBuffer(), dwLen);
		m_oRecvBuffer.DiscardLen(dwLen);
		m_oRecvBuffer.DealBuffer2Packet(this);
	}
	return true;
}

bool CConnection::_ProcError(UINT32 dwError)
{
	m_pFxSession->OnError(dwError);
	return true;
}

bool CConnection::_ProcClose()
{
	if (m_pFxSession)
	{
		m_pFxSession->OnClose();
		m_pFxSession->SetConnection(NULL);
	}
	CConnectionMgr::Instance()->Release(this);

	return true;
}

bool CConnection::Send(const char* pBuf, UINT32 dwLen)
{
	if (m_poSocket)
		return m_poSocket->Send(pBuf, dwLen);
	return true;
}

void CConnection::Close()
{
	if (m_poSocket)
	{
		m_poSocket->Close();
	}
}

bool CConnection::IsConnected()
{
	m_oCloseLock.Lock();
	bool bRet = !m_bClose;
	m_oCloseLock.Unlock();
	return bRet;
}

#ifdef WIN32
void CConnection::SetSocket(CGSocket* poSocket)
{
	m_poSocket = poSocket;
}

CGSocket* CConnection::GetSocket()
{
	return m_poSocket;
}

#else
void CConnection::SetSocket(CLSocket* poSocket)
{
	m_poSocket = poSocket;
}

CLSocket* CConnection::GetSocket()
{
	return m_poSocket;
}

#endif

CRingBuffer* CConnection::GetRecvBuffer()
{
	return &m_oRecvBuffer;
}

CRingBuffer* CConnection::GetSendBuffer()
{
	return &m_oSendBuffer;
}

void CConnection::Uninit()
{
	CConnectionMgr::Instance()->Release(this);
}
