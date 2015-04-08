
#include "stdafx.h"
#include "common/ifnet.h"
#include "common/ifxdb.h"
#include "common/ifhttp.h"
#ifdef WIN32
#pragma comment(lib,"ws2_32.lib") 
#endif // WIN32

#include <map>
#include "workthread.h"
#include "connection.h"
#include "net.h"

class CSession : public FxSession
{
public:
	CSession(){};
	virtual ~CSession(){};

	virtual void        OnConnect(void){};

	virtual void        OnClose(void){};

	virtual void        OnError(UINT32 dwErrorNo){};

	virtual void        OnRecv(const char* pBuf, UINT32 dwLen)
	{
		std::string str;
		str.append(pBuf + 6, dwLen - 6);
		printf("%s\n", str.c_str());

		Send();
		//Send(pBuf, dwLen);
	};

	bool        Send()
	{
		char sz[11];
#ifdef WIN32
		sprintf_s(sz, "%d", 1);
#else 
		snprintf(sz, 2, "1");
#endif

		std::string str = sz;

		FxPacketParser oParse;
		SPacketHeader pstHeader;
		oParse.BuildPkgHeader(&pstHeader, str.length());

		char b[MAX_MESSAGE_LEN];

		//pstHeader.wCheckSum = htons(pstHeader.wCheckSum);
		//pstHeader.wMark = htons(pstHeader.wMark);
		//pstHeader.wDataLen = htons(pstHeader.wDataLen);
		memcpy(b, (char*)&pstHeader.wCheckSum, 2);
		memcpy(b + 2, (char*)&pstHeader.wMark, 2);
		memcpy(b + 4, (char*)&pstHeader.wDataLen, 2);
		memcpy(b + 6, str.c_str(), str.length());

		return FxSession::Send(b, 6 + str.length());
	}

	virtual void        Release(void){};

};

class CSessionFactory : public IFxSessionFactory
{
public:
	static CSessionFactory* Instance()
	{
		static CSessionFactory instance;
		return &instance;
	}

	bool Init()
	{
		m_poolSession.Init(128, 64);
		m_dwSessionID = 0;
		return true;
	}

	virtual FxSession* CreateSession()
	{
		CSession* pSession = m_poolSession.FetchObj();
		m_mapSessions[m_dwSessionID++] = pSession;
		return pSession;
	}

	CSession* FindSession(UINT32 dwID)
	{
		return m_mapSessions[dwID];
	}

private:
	TDynamicPoolEx<CSession> m_poolSession;
	UINT32	m_dwSessionID;
	std::map<UINT32, CSession*> m_mapSessions;
};
int main()
{
	IFxNet* pNet = FxNetGetModule();
	pNet->Init();
	pNet->SetNetOpt(ENET_MAX_CONNECTION, 1000);
	CSessionFactory::Instance()->Init();
	pNet->Listen(CSessionFactory::Instance(), inet_addr("0"), htons(1111));

	while (1)
	{
		if (pNet->Run(100))
		{
		}
		Sleep(1);
	}

}
