#ifndef __STREAMPKGHELPER_MEISHUIJING_2015_03_24__
#define __STREAMPKGHELPER_MEISHUIJING_2015_03_24__

#include <sstream>
#include "fxmeta.h"

using namespace std;

class CStreamPkgHelper : public stringstream
{
public:
	void Write(UINT8 byUint8);
	void Write(UINT16 wUint16);
	void Write(UINT32 dwUint32);
	void Write(const char* pBuff, int nLen);

	bool Read(UINT8* pUint8);
	bool Read(UINT16* pUint16);
	bool Read(UINT32* pUint32);
	bool Read(char* pBuff, int nLen);

};

#endif// __STREAMPKGHELPER_MEISHUIJING_2015_03_24__