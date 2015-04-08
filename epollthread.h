#ifndef __EPOLLTHREAD_MEISHUIJING_2015_04_03__
#define __EPOLLTHREAD_MEISHUIJING_2015_04_03__

#include "stdafx.h"
#include "common/ifnet.h"

class FxNetModule;
class CEpollDisposer;
class CLSocket;
class CEpollThread
{
public:
	CEpollThread(FxNetModule* pNetModule, CEpollDisposer* pDisposer);

	bool Dispatch();

	int	GetEpollFD();
	epoll_event& GetEvent();
	epoll_event* GetEvents();


	void	AllocateEpollThread(CLSocket* poSocket);
	bool	SendEpollMess(CLSocket* poSocket);

private:
	int	m_nMark;
	int m_nEfd;
	epoll_event	m_epEvent;
	epoll_event	m_epEvents[MAX_CONNECTION_COUNT * MAX_NETEVENT_PERSOCK];
	FxNetModule* m_pNetModule;
	CEpollDisposer*	m_pDisposer;
};

#endif// __EPOLLTHREAD_MEISHUIJING_2015_04_03__