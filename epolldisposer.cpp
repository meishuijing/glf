#include "epolldisposer.h"
#include "epollthread.h"
#include "lsocketmgr.h"

bool CEpollDisposer::DisposerInit(FxNetModule* pNetMoudel)
{
	m_pNetModule = pNetMoudel;
	return true;
}

bool CEpollDisposer::DisposerBegin(UINT32 dwIP, UINT16 wPort)
{

	
	struct sockaddr_in addrsrv;
	unsigned int optval;
	struct linger optval1;


	//初始化数据结构
	bzero(&addrsrv, sizeof(addrsrv));
	addrsrv.sin_addr.s_addr = dwIP;
	addrsrv.sin_family = AF_INET;
	addrsrv.sin_port = wPort;

	//创建SOCKET
	m_nSrvSock = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > m_nSrvSock) 
		return false;

	//设置SO_REUSEADDR选项(服务器快速重起)
	optval = 0x1;
	setsockopt(m_nSrvSock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, 4);

	int flags = fcntl(m_nSrvSock, F_GETFL, 0);
	if (flags == -1)
		return false;

	//设置文件状态标志  
	flags |= O_NONBLOCK;
	if (fcntl(m_nSrvSock, F_SETFL, flags) == -1)
		return false;

	//设置SO_LINGER选项(防范CLOSE_WAIT挂住所有套接字)
	optval1.l_onoff = 1;
	optval1.l_linger = 60;
	setsockopt(m_nSrvSock, SOL_SOCKET, SO_LINGER, (char*)&optval1, sizeof(struct linger));

	if (0 > bind(m_nSrvSock, (struct sockaddr *)&addrsrv, sizeof(addrsrv)))
	{
		close(m_nSrvSock);
		return false;
	}

	if (0 > listen(m_nSrvSock, 128))
	{
		close(m_nSrvSock);
		return false;
	}

	m_nEfd = epoll_create(MAX_CONNECTION_COUNT);
	if (m_nEfd < 0)
		return false;

	m_epEvent.data.fd = m_nSrvSock;
	m_epEvent.events = EPOLLIN | EPOLLET;//读入,边缘触发方式  
	if (epoll_ctl(m_nEfd, EPOLL_CTL_ADD, m_nSrvSock, &m_epEvent) == -1)
	{
		perror("epoll_ctl");
		return false;
	}

	int cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
	for (int i = 0; i < cpu_num; ++i)
	{
		CEpollThread* pThread = new CEpollThread(m_pNetModule, this);
		if (NULL == pThread)
			return false;

		if (!pThread->Dispatch())
			return false;
		m_vecEpollThread.push_back(pThread);
	}

}

bool CEpollDisposer::DisposerWork()
{
	int nFds;
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	struct linger optval1;
	//服务循环
	while (true)
	{
		//等待epoll事件
		nFds = epoll_wait(m_nEfd, m_epEvents, MAX_CONNECTION_COUNT, 0);
		//处理epoll事件
		for (int i = 0; i < nFds; i++)
		{
			//接收客户端连接
			if (m_epEvents[i].data.fd == m_nSrvSock)
			{
				//创建FxSocket
				struct sockaddr addrClient;
				socklen_t addrLen = sizeof(addrClient);
				int nClientFd = 0;
				while ((nClientFd = accept(m_nSrvSock, (struct sockaddr *)&addrClient, &addrLen)) > 0)
				{
					int rt = 0;

					rt = getnameinfo(&addrClient, addrLen,
						hbuf, sizeof hbuf,
						sbuf, sizeof sbuf,
						NI_NUMERICHOST | NI_NUMERICSERV);//flag参数:以数字名返回  
					//主机地址和服务地址  

					int flags = fcntl(nClientFd, F_GETFL, 0);
					if (flags == -1)
					{
						close(nClientFd);
						continue;
					}

					flags |= O_NONBLOCK;
					if (fcntl(nClientFd, F_SETFL, flags) == -1)
					{
						close(nClientFd);
						continue;
					}

					//设置SO_LINGER选项(防范CLOSE_WAIT挂住所有套接字)
					optval1.l_onoff = 1;
					optval1.l_linger = 60;
					setsockopt(nClientFd, SOL_SOCKET, SO_LINGER, (char*)&optval1, sizeof(struct linger));


					CLSocket* poSocket = CLSocketMgr::Instance()->FetchSocket();
					if (NULL == poSocket)
					{
						close(nClientFd);
						continue;
					}

					CConnection* pConnection = CConnectionMgr::Instance()->CreateConnection(m_pNetModule);
					if (NULL == pConnection)
					{
						close(nClientFd);
						CLSocketMgr::Instance()->ReleaseSocket(poSocket);
						continue;
					}
					poSocket->SetConnection(pConnection);
					poSocket->SetSock(nClientFd);
					poSocket->SetDisposer(this);
					pConnection->SetSocket(poSocket);
					pConnection->PushEvent(NETEVT_CONNECTED, 0);

					AllocateEpollThread(poSocket);
				}
			}
		}
	}
}

bool CEpollDisposer::DisposerEnd()
{

}

void CEpollDisposer::AllocateEpollThread(CLSocket* poSocket)
{
	int size = m_vecEpollThread.size();
	int nChoose = poSocket->GetMark() % size;
	m_vecEpollThread[nChoose]->AllocateEpollThread(poSocket);
}

bool CEpollDisposer::SendEpollMess(CLSocket* poSocket)
{
	int size = m_vecEpollThread.size();
	int nChoose = poSocket->GetMark() % size;
	m_vecEpollThread[nChoose]->SendEpollMess(poSocket);
}
