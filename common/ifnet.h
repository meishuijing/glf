#ifndef __IFNET_H_2009_0901__
#define __IFNET_H_2009_0901__


#ifdef WIN32
#include <WinSock2.h>

#ifdef   FXN_DLLCLASS
#define  DLLCLASS_DECL           __declspec(dllexport)   
#else
#define  DLLCLASS_DECL           __declspec(dllexport)   
#endif

#ifdef FXN_STATICLIB
#undef  DLLCLASS_DECL
#define DLLCLASS_DECL
#endif

#else
#define SOCKET UINT32
#define DLLCLASS_DECL
#define INVALID_SOCKET UINT32(-1)
#endif

#include "fxmeta.h"

class FxConnection;

#define LINUX_NETTHREAD_COUNT   2   // linux默认网络线程数,Windows默认采用cpu个数的2倍
#define MAX_CONNECTION_COUNT    1024
#define MAX_NETEVENT_PERSOCK    1024

#define MAX_MESSAGE_LEN	65535

enum ENetErrCode{
    NET_RECVBUFF_ERROR  = -7,
    NET_CONNECT_FAIL    = -6,
    NET_SYSTEM_ERROR    = -5, 
    NET_RECV_ERROR      = -4, 
    NET_SEND_ERROR      = -3, 
    NET_SEND_OVERFLOW   = -2,
    NET_PACKET_ERROR    = -1,
    NET_SUCCESS         = 0
};

enum ESessionOpt
{
    ESESSION_SENDLINGER = 1,    // 发送延迟，直到成功，或者30次后，默认不打开
};

enum ENetOpt
{
    ENET_MAX_CONNECTION = 1,    // 最大连接数
    ENET_MAX_TOTALEVENT,        // 每个Socket的最大事件数量
};

class CConnection;
class DLLCLASS_DECL FxSession
{
public:
	FxSession();
    virtual ~FxSession();

    virtual void        OnConnect(void) = 0;

    virtual void        OnClose(void) = 0;

    virtual void        OnError(UINT32 dwErrorNo) = 0;

    virtual void        OnRecv(const char* pBuf, UINT32 dwLen) = 0;

    virtual void        Release(void) = 0;

    virtual bool        Send(const char* pBuf,UINT32 dwLen);

    virtual bool        SetSessionOpt(ESessionOpt eOpt, bool bSetting);

	void				Close();

	bool				IsConnect();


	CConnection*		GetConnection();
	void				SetConnection(CConnection* pConnection);

private:
	CConnection*	m_pConnection;
};

class IFxSessionFactory
{
public:
	virtual ~IFxSessionFactory() {}
    virtual FxSession*   CreateSession() = 0;
};

class IFxNet
{
public:
    virtual ~IFxNet() {}
    virtual bool        SetNetOpt(ENetOpt eOpt, int nValue) = 0;
    virtual bool        Init() = 0;
    virtual bool        Run(UINT32 dwCount) = 0;
    virtual void        Release() = 0;

	virtual bool      Connect(FxSession* poSession, UINT32 dwIP, UINT16 wPort) = 0;
    virtual bool      Listen(IFxSessionFactory* poFactory, UINT32 dwIP, UINT16 wPort) = 0;

private:

};

IFxNet* FxNetGetModule();

typedef IFxNet* (*PFN_FxNetGetModule)();


#endif  // __IFNET_H_2009_0901__

