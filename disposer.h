#ifndef __DISPOSER_MEISHUIJING_2015_03_30__
#define __DISPOSER_MEISHUIJING_2015_03_30__

#include "common/fxmeta.h"

class FxNetModule;
class IDisposer
{
public:
	virtual ~IDisposer(){};
	virtual	bool	DisposerInit(FxNetModule* pNetMoudel) = 0;
	virtual bool	DisposerBegin(UINT32 dwIP, UINT16 wPort) = 0;
	virtual bool	DisposerWork() = 0;
	virtual bool	DisposerEnd() = 0;
};

#endif// __DISPOSER_MEISHUIJING_2015_03_30__