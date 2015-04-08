#ifndef __EPOLLDISPOSER_MEISHUIJING_2015_04_03__
#define __EPOLLDISPOSER_MEISHUIJING_2015_04_03__

#include "disposer.h"
#include "stdafx.h"
#include "common/ifnet.h"
#include <vector>
#include "common/fxmeta.h"

class FxNetModule;
class CEpollThread;
class CLSocket;
class CEpollDisposer : public IDisposer
{
public:
	virtual	bool	DisposerInit(FxNetModule* pNetMoudel) ;
	virtual bool	DisposerBegin(UINT32 dwIP, UINT16 wPort);
	virtual bool	DisposerWork() ;
	virtual bool	DisposerEnd() ;

	void	AllocateEpollThread(CLSocket* poSocket);
	bool	SendEpollMess(CLSocket* poSocket);

private:
	FxNetModule*	m_pNetModule;
	std::vector<CEpollThread*> m_vecEpollThread;
	int	m_nEfd;
	int m_nSrvSock;
	epoll_event	m_epEvent;
	epoll_event	m_epEvents[MAX_CONNECTION_COUNT];
};

#endif// __EPOLLDISPOSER_MEISHUIJING_2015_04_03__