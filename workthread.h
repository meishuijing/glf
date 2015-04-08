#ifndef __WORKTHREAD_MEISHUIJING_2015_03_05__
#define __WORKTHREAD_MEISHUIJING_2015_03_05__

#include "stdafx.h"
#include "common/eventqueue.h"
#include "common/ifnet.h"

enum
{
	WORK_THREAD_LISTEN,
	WORK_THREAD_CONNECT,
};

class FxNetModule;
class CWorkThread
{
public:

	bool	Init(IFxSessionFactory* poFactory, UINT32 dwIP, UINT16 wPort, FxNetModule* pNetModule);
	bool	Init(FxSession* poSession, UINT32 dwIP, UINT16 wPort, FxNetModule* pNetModule);
	void	Uninit();

#ifdef WIN32
	static unsigned WINAPI ThreadFunc(LPVOID pParam);

#endif

	void    OnExecute();

	bool	Run();

	bool	Listen();

	bool	Connect();

	FxNetModule* GetNetModule();

private:

private:
	IFxSessionFactory* m_poFactory;
	FxSession*	m_poSession;
	UINT32	m_dwIP;
	UINT16	m_wPort;
	FxNetModule* m_pNetModule;
};

#endif// __WORKTHREAD_MEISHUIJING_2015_03_05__