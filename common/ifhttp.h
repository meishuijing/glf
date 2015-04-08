//  Created by Nijie on 2011/08/26.
//  Copyright (c) 2011 Nijie. All rights reserved.
//

#ifndef __FIHTTP_H_NJ_2011_0826__
#define __FIHTTP_H_NJ_2011_0826__

#include <string>
#include "fxmeta.h"

using namespace std;

#define MAX_HTTP_EVENT 1000

enum ERequestType
{
	RequestType_Get		= 0,
	RequestType_Post	= 1,
	RequestType_File	= 2,
	RequestType_Https	= 3,
};

enum EResponseType
{
	ReponseType_UrlError = -4,
	ReponseType_SendError = -3,
	ReponseType_NoContext = -2,
	ReponseType_TimeOut,
	ReponseType_Success,
};

class IRequest;
struct SOneHttpConn
{
public:
	IRequest* pRequest;
	ERequestType reqType;
	std::string strUrl;
	std::string	strData;
	UINT32	dwTimeOut;//∫¡√Îº∂
};

class IResponseReader
{
public:
	virtual ~IResponseReader(){};
	virtual unsigned int	GetSN() = 0;
	virtual ERequestType	GetRequestType() = 0;
	virtual const char*		GetResponse() = 0;
	virtual const char*		GetUrl() = 0;
	virtual int				GetError() = 0;
};

class IRequest
{
public:
	virtual ~IRequest(){};
	virtual	void	OnResponse(IResponseReader* pReader) = 0;
	virtual	void	Release() = 0;
};

class IHttp
{
public:
	virtual bool	Init() = 0;
	virtual bool	Run(int nCount = 0) = 0;	// ∑µªÿ «∑Ò√¶
	virtual bool	AddRequest(SOneHttpConn* poRequest) = 0;
	virtual bool	Release() = 0;
};
IHttp* CreateHTTPModule();

#endif	// __FIHTTP_H_NJ_2011_0826__
