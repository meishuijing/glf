#include "epollthread.h"
#include "lsocketmgr.h"
#include "connection.h"
#include "epolldisposer.h"

CEpollThread::CEpollThread(FxNetModule* pNetModule, CEpollDisposer* pDisposer)
{
	m_pNetModule = pNetModule;
	m_pDisposer = pDisposer;
}

void *ThreadEpollFunc(void* pParam)
{
	CEpollThread* pCtrl = (CEpollThread*)pParam;
	//服务循环
	int nFds = 0;
	while (true)
	{
		//等待epoll事件

		epoll_event* pEvents = pCtrl->GetEvents();
		nFds = epoll_wait(pCtrl->GetEpollFD(), pEvents, MAX_CONNECTION_COUNT , 0);
		//处理epoll事件
		for (int i = 0; i < nFds; i++)
		{
			//接收数据和写数据
			if (pEvents[i].data.ptr != NULL)
			{
				CLSocket* poSocket = (CLSocket*)pEvents[i].data.ptr;
				int fd = poSocket->GetSock();
				if (pEvents[i].events & EPOLLIN)
				{
					
					char* pOut = NULL;
					UINT16 wOutLen = 0;
					poSocket->GetConnection()->GetRecvBuffer()->GetBufPushPtrAndLen(pOut, wOutLen);
					if (wOutLen > 0)
					{
						int nReadRet = 0;
						while ((nReadRet = read(fd, pOut, wOutLen)) > 0)
						{
							poSocket->GetConnection()->GetRecvBuffer()->AddBufLen(nReadRet);
							poSocket->GetConnection()->GetRecvBuffer()->DealBuffer2Packet(poSocket->GetConnection());
							poSocket->GetConnection()->GetRecvBuffer()->GetBufPushPtrAndLen(pOut, wOutLen);
							if (wOutLen == 0)
								break;
						}

						if (nReadRet < 0)
						{
							if (errno != EAGAIN)
								poSocket->GetConnection()->PushEvent(NETEVT_ERROR, errno);
						}
						else if (0 == nReadRet)
						{
							poSocket->Close();
						}


					}
				}
				else if (pEvents[i].events & EPOLLOUT)
				{
					char* pPop = NULL;
					UINT16 wPopLen = 0;
					poSocket->GetConnection()->GetSendBuffer()->GetBufPopPtrAndLen(pPop, wPopLen);
					if (wPopLen > 0)
					{
						int nWriteRet = 0;
						while ((nWriteRet = write(fd, pPop, wPopLen)) > 0)
						{
							poSocket->GetConnection()->GetSendBuffer()->DiscardLen(nWriteRet);
							poSocket->GetConnection()->GetSendBuffer()->GetBufPopPtrAndLen(pPop, wPopLen);
							if (wPopLen == 0)
								break;
						}
						if (nWriteRet < 0)
						{
							if (errno != EAGAIN)
								poSocket->GetConnection()->PushEvent(NETEVT_ERROR, errno);
						}
						else if (0 == nWriteRet)
						{
							poSocket->Close();
						}

						epoll_event	epEvent;
						epEvent.data.fd = poSocket->GetSock();
						epEvent.data.ptr = poSocket;
						epEvent.events = EPOLLIN | EPOLLET;//写入,边缘触发方式  
						epoll_ctl(pCtrl->GetEpollFD(), EPOLL_CTL_MOD, fd, &epEvent);
					}
				}
			}
		}
	}
}

bool CEpollThread::Dispatch()
{
	m_nEfd = epoll_create(MAX_CONNECTION_COUNT );
	if (m_nEfd < 0)
		return false;

	pthread_t id;
	int ret = pthread_create(&id, NULL, ThreadEpollFunc, (void*)this);
	if (ret != 0)
		return false;
	return true;
}

int CEpollThread::GetEpollFD()
{
	return m_nEfd;
}

epoll_event& CEpollThread::GetEvent()
{
	return m_epEvent;
}

epoll_event* CEpollThread::GetEvents()
{
	return m_epEvents;
}

void CEpollThread::AllocateEpollThread(CLSocket* poSocket)
{
	m_epEvent.data.ptr = poSocket;
	m_epEvent.events = EPOLLIN | EPOLLET;//读入,边缘触发方式  
	if (epoll_ctl(m_nEfd, EPOLL_CTL_ADD, poSocket->GetSock(), &m_epEvent) == -1)
	{
		printf("%u", errno);
		return;
	}
}

bool CEpollThread::SendEpollMess(CLSocket* poSocket)
{
	m_epEvent.data.ptr = poSocket;
	m_epEvent.events = EPOLLIN | EPOLLOUT | EPOLLET;//写入,边缘触发方式  
	if (epoll_ctl(m_nEfd, EPOLL_CTL_MOD, poSocket->GetSock(), &m_epEvent) == -1)
	{
		return false;
	}
	return true;
}
