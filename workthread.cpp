#include "workthread.h"
#include "connection.h"
#ifdef WIN32
#include "iocpdisposer.h"
#include "gsocketmgr.h"
#else
#include "epolldisposer.h"
#include "lsocketmgr.h"
#endif
#include "net.h"


bool CWorkThread::Init(IFxSessionFactory* poFactory, UINT32 dwIP, UINT16 wPort, FxNetModule* pNetModule)
{
	m_poFactory = poFactory;
	m_dwIP = dwIP;
	m_wPort = wPort;
	m_pNetModule = pNetModule;
	m_poSession = NULL;

	return CConnectionMgr::Instance()->Init(poFactory);
}

bool CWorkThread::Init(FxSession* poSession, UINT32 dwIP, UINT16 wPort, FxNetModule* pNetModule)
{
	m_dwIP = dwIP;
	m_wPort = wPort;
	m_pNetModule = pNetModule;
	m_poSession = poSession;
	m_poFactory = NULL;

	return true;
}

#ifdef WIN32
unsigned WINAPI CWorkThread::ThreadFunc(LPVOID pParam)
{
	CWorkThread* poCtrl = (CWorkThread*)pParam;
	poCtrl->OnExecute();
	return 0;
}
#else
void  *ThreadWordThreadFunc(void* pParam)
{
	CWorkThread* poCtrl = (CWorkThread*)pParam;
	poCtrl->OnExecute();
}
#endif

void CWorkThread::OnExecute()
{
	if (m_poFactory)
	{
		Listen();
	}
	else
	{
		Connect();
	}
}

bool CWorkThread::Run()
{
#ifdef WIN32
	UINT32 dwThreadID;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL,					// Security
			0,						// Stack size - use default
			ThreadFunc,     		// Thread fn entry point
			(void*)this,			// Param for thread
			0,						// Init flag
			&dwThreadID);			// Thread address

#else
	pthread_t id;
	pthread_create(&id, NULL, ThreadWordThreadFunc, (void*)this);
#endif

	return true;
}

bool CWorkThread::Listen()
{

#ifdef WIN32
	CIOCPDisposer oDisposer;
#else
	CEpollDisposer oDisposer;
#endif
	oDisposer.DisposerInit(m_pNetModule);

	oDisposer.DisposerBegin(m_dwIP, m_wPort);
	
	oDisposer.DisposerWork();


	return true;
}

bool CWorkThread::Connect()
{
#ifdef WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);

	CConnection* pConnection = new CConnection;
	if (NULL == pConnection)
	{
		WSACleanup();
		return false;
	}
	if (!pConnection->Init(m_pNetModule))
	{
		delete pConnection;
		pConnection = NULL;
		WSACleanup();
		return false;
	}

	CGSocket* pSocket = new CGSocket;
	if (NULL == pSocket)
	{
		delete pConnection;
		pConnection = NULL;
		WSACleanup();
		return false;
	}
	if (!pSocket->Init())
	{
		delete pConnection;
		pConnection = NULL;
		delete pSocket;
		pSocket = NULL;
		WSACleanup();
		return false;
	}
	pConnection->SetSocket(pSocket);
	pConnection->SetSession(m_poSession);
	m_poSession->SetConnection(pConnection);
	pSocket->SetConnection(pConnection);

	sockaddr_in addrClient;
	memset(&addrClient, 0, sizeof(sockaddr_in));
	addrClient.sin_family = AF_INET;
	addrClient.sin_port = m_wPort;
	addrClient.sin_addr.S_un.S_addr = m_dwIP;

	if (SOCKET_ERROR == connect(pSocket->GetSock(), (sockaddr*)&addrClient, sizeof(addrClient)))
	{
		if (WSAEWOULDBLOCK != WSAGetLastError())
		{
			pConnection->PushEvent(NETEVT_ERROR, (UINT32)WSAGetLastError());
			pConnection->Close();
			return false;
		}
	}

	CIOCPDisposer* pDisposer = new CIOCPDisposer;
	pDisposer->DisposerInit(m_pNetModule);
	CIOCPThread* pThread = new CIOCPThread(m_pNetModule, pDisposer);
	pThread->AllocateIoPortAndRun(pSocket);
	return true;
#endif


}

FxNetModule* CWorkThread::GetNetModule()
{
	return m_pNetModule;
}

void CWorkThread::Uninit()
{

}
