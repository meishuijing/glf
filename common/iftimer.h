//  Created by Nijie on 2014/08/15.
//  Copyright (c) 2014 Nijie. All rights reserved.
//

#ifndef __IFTIMER_H_NIJIE_2014_0808__
#define __IFTIMER_H_NIJIE_2014_0808__

#include "fxmeta.h"

#define MAX_TIMER_EVENT 1000

class ITimer
{
public: 
	virtual ~ITimer(){};
	virtual void	onTimer(UINT32 uMilliSecond, UINT32 uEventId)	= 0;
};

//////////////////////////////////////////////////////////////////////////
class ITimerModule
{
public:
	virtual ~ITimerModule(){};

    virtual bool		init(ITimer* poTimer) = 0;
    virtual bool        run()       = 0;
    virtual void        stop()      = 0;
    virtual bool        addTimer(UINT32 uMilliSecond, UINT32 uEventId)    = 0;
    virtual bool        delTimer(UINT32 uMilliSecond, UINT32 uEventId)    = 0;
    virtual void        release()   = 0;
    virtual UINT64      getMicroTime()    = 0;
};

ITimerModule*			FxGetTimerModule();

#endif	// __IFTIMER_H_NIJIE_2014_0808__
