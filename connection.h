#ifndef __SESSION_MEISHUIJING_2015_03_05__
#define __SESSION_MEISHUIJING_2015_03_05__

#include "stdafx.h"
#include "common/dynamicpoolex.h"
#include "common/ifnet.h"
#include "common/packetparser.h"
#include "common/eventqueue.h"
#include "common/lock.h"
#include "ringbuffer.h"


enum ENetEvtType
{
	NETEVT_INVALID = 0,
	NETEVT_CONNECTED,
	NETEVT_ERROR,
	NETEVT_RECV,
	NETEVT_CLOSE,
};

struct SNetEvent
{
	ENetEvtType		eType;
	UINT32          dwValue;
};

class FxNetModule;
#ifdef WIN32
class CGSocket;
#else 
class CLSocket;
#endif
class CConnection
{
public:

	bool	Init(FxNetModule* pNetModule);
	void	Uninit();

#ifdef WIN32
	void	SetSocket(CGSocket* poSocket);
	CGSocket*	GetSocket();
#else
	void	SetSocket(CLSocket* poSocket);
	CLSocket*	GetSocket();
#endif

	void	SetSession(FxSession* pFxSession);
	FxSession*	GetSession();

	bool	ProcEvent();

	bool	PushEvent(ENetEvtType eType, UINT32 dwValue, bool bWaitFor = true);


	bool	Send(const char* pBuf, UINT32 dwLen);


	void	Close();

	bool	IsConnected();
	
	CRingBuffer*	GetRecvBuffer();
	CRingBuffer*	GetSendBuffer();

private:
	
	bool	_ProcConnected();
	bool	_ProcRecv(UINT32 dwLen);
	bool	_ProcError(UINT32 dwError);
	bool	_ProcClose();

private:
#ifdef WIN32
	CGSocket*	m_poSocket;
#else
	CLSocket*	m_poSocket	;
#endif
	FxSession*	m_pFxSession;

	CRingBuffer	m_oRecvBuffer;
	CRingBuffer m_oSendBuffer;


	TEventQueue<SNetEvent>	m_eventNet;

	FxLock	m_oCloseLock;
	bool	m_bClose;

	FxNetModule* m_pNetModule;
};

class CConnectionMgr
{
public:

	static CConnectionMgr* Instance();

	bool	Init(IFxSessionFactory* pFactory);
	CConnection*	CreateConnection(FxNetModule* pNetModule);

	void	Release(CConnection* pConnection);

private:
	TDynamicPoolEx<CConnection> m_poolConnection;
	IFxSessionFactory* m_pFactory;
};

#endif// __SESSION_MEISHUIJING_2015_03_05__