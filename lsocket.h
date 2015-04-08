#ifndef __LSOCKET_MEISHUIJING_2015_04_03__
#define __LSOCKET_MEISHUIJING_2015_04_03__

#include "stdafx.h"
#include "connection.h"


class CEpollDisposer;
class CLSocket
{

public:

	bool	Init();
	void	Uninit();

	void	SetMark(UINT32 uMark);
	UINT32	GetMark();

	void	SetSock(int nSock);
	int		GetSock();

	void	SetDisposer(CEpollDisposer* pDisposer);

	void	SetConnection(CConnection* pConnection);
	CConnection*	GetConnection();

	bool	Send(const char* pBuf, UINT32 uLen);
	void	Close();

	bool	IsConnected();

private:


	UINT32 m_uMark;
	int m_nSocket;

	CConnection* m_pConnection;

	CEpollDisposer*	m_pDisposer;

};

#endif// __LSOCKET_MEISHUIJING_2015_04_03__