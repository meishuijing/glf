#ifndef __NET_H_2009_0825__
#define __NET_H_2009_0825__

#include "stdafx.h"
#include "common/ifnet.h"
#include "workthread.h"

using namespace std;

class CConnection;
class FxNetModule : public IFxNet
{
public:
	FxNetModule();
	virtual ~FxNetModule();

	virtual bool        Init();
	void	Uninit();
	virtual bool        Run(UINT32 dwCount = 0xffffffff);
	virtual void        Release();
	virtual bool        SetNetOpt(ENetOpt eOpt, int nValue);

	virtual bool		Connect(FxSession* poSession, UINT32 dwIP, UINT16 wPort);
	virtual bool        Listen(IFxSessionFactory* poFactory, UINT32 dwIP, UINT16 wPort);


	bool				PushEventQueue(CConnection* pConnection);

	char*				GetMessBuffer();

private:
	int                 m_nThreadCount;
	int                 m_nMaxConnectionCount;
	int                 m_nTotalEventCount;
	bool                m_bInit;

	TEventQueue<CConnection*> m_eventQueue;

	CWorkThread			m_oWorkThread;

	char	m_szMessBuffer[MAX_MESSAGE_LEN];
};

#endif  // __NET_H_2009_0825__