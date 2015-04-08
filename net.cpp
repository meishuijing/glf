#include "net.h"
#include "connection.h"
#ifdef WIN32
#include "gsocketmgr.h"
#else
#include "lsocketmgr.h"
#endif

FxNetModule::FxNetModule()
{
	m_nThreadCount = 1;
	m_nMaxConnectionCount = MAX_CONNECTION_COUNT;
	m_nTotalEventCount = MAX_NETEVENT_PERSOCK * MAX_CONNECTION_COUNT;
	m_bInit = false;
}

FxNetModule::~FxNetModule()
{
	Uninit();
}

bool FxNetModule::Init()
{
	if (!m_eventQueue.Init(m_nTotalEventCount))
		return false;

#ifdef WIN32
	if (!CGSocketMgr::Instance()->Init())
		return false;
#else 
	if (!CLSocketMgr::Instance()->Init())
		return false;
#endif
	return true;
}

bool FxNetModule::Connect(FxSession* poSession, UINT32 dwIP, UINT16 wPort)
{
	if (!m_oWorkThread.Init(poSession, dwIP, wPort, this))
		return false;

	return m_oWorkThread.Run();
}

bool FxNetModule::Listen(IFxSessionFactory* poFactory, UINT32 dwIP, UINT16 wPort)
{
	if (!m_oWorkThread.Init(poFactory, dwIP, wPort, this))
		return false;

	return m_oWorkThread.Run();
}

bool FxNetModule::PushEventQueue(CConnection* pConnection)
{
	if (!m_eventQueue.PushBack(pConnection))
		return false;

	return true;
}

char* FxNetModule::GetMessBuffer()
{
	return m_szMessBuffer;
}

bool FxNetModule::Run(UINT32 dwCount)
{
	bool bRet = false;

	for (int i = 0; i < dwCount; ++i)
	{
		CConnection** ppCon = m_eventQueue.PopFront();
		if (NULL == ppCon)
		{
			return bRet;
		}

		CConnection* pConnection = *ppCon;
		if (NULL == pConnection)
		{
			return bRet;
		}

		pConnection->ProcEvent();
		bRet = true;
	}
	return bRet;
}

void FxNetModule::Release()
{

}

bool FxNetModule::SetNetOpt(ENetOpt eOpt, int nValue)
{
	if (m_bInit)
	{
		return false;
	}

	switch (eOpt)
	{
	case ENET_MAX_CONNECTION:
	{
		if (0 >= nValue)
		{
			return false;
		}
		m_nMaxConnectionCount = nValue;
	}
		break;

	case ENET_MAX_TOTALEVENT:
	{
		if (0 >= nValue)
		{
			return false;
		}
		m_nTotalEventCount = nValue;
	}
		break;

	default:
		return false;
	}

	return true;
}

void FxNetModule::Uninit()
{
#ifdef WIN32
	CGSocketMgr::Instance()->Uninit();
#else
	CLSocketMgr::Instance()->Uninit();
#endif
	m_oWorkThread.Uninit();
}
