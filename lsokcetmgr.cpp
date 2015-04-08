#include "lsocketmgr.h"
#include "common/ifnet.h"

CLSocketMgr* CLSocketMgr::Instance()
{
	static CLSocketMgr instance;
	return &instance;
}

static bool bInit = false;
bool CLSocketMgr::Init()
{
	bInit = true;
	if (!m_poGSocket.Init(MAX_CONNECTION_COUNT / 4, MAX_CONNECTION_COUNT / 8))
		return false;
	m_uMarkIndex = 0;
	return true;
}

CLSocket* CLSocketMgr::FetchSocket()
{
	CLSocket* pSocket = m_poGSocket.FetchObj();
	if (NULL == pSocket)
		return NULL;
	if (!pSocket->Init())
	{
		m_poGSocket.ReleaseObj(pSocket);
		return NULL;
	}
	pSocket->SetMark(m_uMarkIndex++);
	return pSocket;
}

void CLSocketMgr::ReleaseSocket(CLSocket* pSocket)
{
	if (bInit)
		m_poGSocket.ReleaseObj(pSocket);
	else
		delete pSocket;
}

void CLSocketMgr::Uninit()
{

}
