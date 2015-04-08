#include "lsocket.h"
#include "lsocketmgr.h"
#include "epolldisposer.h"

void CLSocket::SetMark(UINT32 uMark)
{
	m_uMark = uMark;
}

UINT32 CLSocket::GetMark()
{
	return m_uMark;
}

bool CLSocket::Init()
{
	return true;
}

void CLSocket::SetSock(int nSock)
{
	m_nSocket = nSock;
}

int CLSocket::GetSock()
{
	return m_nSocket;
}

void CLSocket::Uninit()
{
	if (m_nSocket != 0)
	{
		close(m_nSocket);
		m_nSocket = 0;
	}

	m_pConnection->SetSocket(NULL);
	m_pConnection->Uninit();
	CLSocketMgr::Instance()->ReleaseSocket(this);
}

void CLSocket::SetConnection(CConnection* pConnection)
{
	m_pConnection = pConnection;
}

CConnection* CLSocket::GetConnection()
{
	return m_pConnection;
}

void CLSocket::SetDisposer(CEpollDisposer* pDisposer)
{
	m_pDisposer = pDisposer;
}

void CLSocket::Close()
{
	GetConnection()->PushEvent(NETEVT_CLOSE, 0);
	Uninit();
}

bool CLSocket::Send(const char* pBuf, UINT32 uLen)
{
	if (!IsConnected())
		return false;

	int n = 1;//Í¶µÝ´ÎÊý
	while (n < 30)
	{
		char* pPush = NULL;
		UINT16 wPushLen = 0;
		GetConnection()->GetSendBuffer()->GetBufPushPtrAndLen(pPush, wPushLen);
		if (wPushLen > 0)
		{
			if (wPushLen >= uLen)
			{
				memcpy(pPush, pBuf, uLen);
				GetConnection()->GetSendBuffer()->AddBufLen(uLen);
				break;
			}
			else
			{
				uLen -= wPushLen;
				memcpy(pPush, pBuf, wPushLen);
				GetConnection()->GetSendBuffer()->AddBufLen(wPushLen);
			}
		}
		++n;
	}

	return m_pDisposer->SendEpollMess(this);
}

bool CLSocket::IsConnected()
{
	if (GetConnection())
	{
		return GetConnection()->IsConnected();
	}
	return false;
}