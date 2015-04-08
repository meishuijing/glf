
#include "stdafx.h"
#include "common/ifnet.h"
#include "net.h"
#include "connection.h"

#pragma comment(lib,"ws2_32.lib") 

IFxNet* FxNetGetModule()
{
	return new FxNetModule;
}

FxSession::FxSession()
{
	m_pConnection = NULL;
}	

FxSession::~FxSession()
{
		
}

bool FxSession::Send(const char* pBuf, UINT32 dwLen)
{
	if (!m_pConnection)
		return false;

	m_pConnection->Send(pBuf, dwLen);
	return true;
}

bool FxSession::SetSessionOpt(ESessionOpt eOpt, bool bSetting)
{
	return true;
}

void FxSession::Close()
{
	if (m_pConnection)
	{
		m_pConnection->Close();
	}
}

bool FxSession::IsConnect()
{
	if (m_pConnection)
	{
		return m_pConnection->IsConnected();
	}
	return false;
}

CConnection* FxSession::GetConnection()
{
	return m_pConnection;
}

void FxSession::SetConnection(CConnection* pConnection)
{
	m_pConnection = pConnection;
}
