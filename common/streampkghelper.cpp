#include "streampkghelper.h"

#ifdef WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

void CStreamPkgHelper::Write(UINT8 byUint8)
{
	write((char*)&byUint8, sizeof(byUint8));
}

void CStreamPkgHelper::Write(UINT16 wUint16)
{
	wUint16 = htons(wUint16);
	write((char*)&wUint16, sizeof(wUint16));
}

void CStreamPkgHelper::Write(UINT32 dwUint32)
{
	dwUint32 = htonl(dwUint32);
	write((char*)&dwUint32, sizeof(dwUint32));
}

void CStreamPkgHelper::Write(const char* pBuff, int nLen)
{
	write(pBuff, nLen);
}

bool CStreamPkgHelper::Read(UINT8* pUint8)
{
	UINT8& rfUint8 = *pUint8;
	read((char*)pUint8, sizeof(rfUint8));
	if (gcount() == sizeof(rfUint8))
		return true;
	rfUint8 = 0;
	return false;
}

bool CStreamPkgHelper::Read(UINT16* pUint16)
{
	UINT16& rfUint16 = *pUint16;
	read((char*)pUint16, sizeof(rfUint16));
	rfUint16 = ntohs(rfUint16);
	if (gcount() == sizeof(rfUint16))
		return true;
	rfUint16 = 0;
	return false;
}

bool CStreamPkgHelper::Read(UINT32* pUint32)
{
	UINT32& rfUint32 = *pUint32;
	read((char*)pUint32, sizeof(rfUint32));
	rfUint32 = ntohl(rfUint32);
	if (gcount() == sizeof(rfUint32))
		return true;
	rfUint32 = 0;
	return false;
}

bool CStreamPkgHelper::Read(char* pBuff, int nLen)
{
	read((char*)pBuff, nLen);
	if (gcount() == nLen)
		return true;
	return false;
}
