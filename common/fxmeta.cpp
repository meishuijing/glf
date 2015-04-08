#include "common/stdafx.h"
#include "fxmeta.h"
#include <pthread.h>
#include <set>
#include <list>
#include <vector>

using namespace std;

#ifndef INFINITE
#define INFINITE		    0xffffffff
#endif

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

#define EPOCHFILETIME (116444736000000000i64)

class FxCriticalLock : public IFxLock
{
	// Can't be copied
	void operator = (FxCriticalLock &locker) {}
public:
#ifdef WIN32
	FxCriticalLock() 
	{
		InitializeCriticalSection(&m_lock); 
	}
	~FxCriticalLock() 
	{
		DeleteCriticalSection(&m_lock); 
	}
	void Lock() 
	{
		EnterCriticalSection(&m_lock); 
	}
	void UnLock() 
	{
		LeaveCriticalSection(&m_lock); 
	}
#else
	FxCriticalLock() 
	{
		pthread_mutex_init(&m_lock,NULL);
	}
	~FxCriticalLock() 
	{
		pthread_mutex_destroy(&m_lock);
	}
	void Lock() 
	{
		pthread_mutex_lock(&m_lock);
	}
	void UnLock() 
	{
		pthread_mutex_unlock(&m_lock);
	}
#endif

	virtual void            Release() { if(NULL != this) delete this; }

private:
#ifdef WIN32
	CRITICAL_SECTION m_lock;
#else
	pthread_mutex_t m_lock;
#endif
};

class FxFakeLock: public IFxLock
{
	// Can't be copied
	void operator = (FxFakeLock &locker) {}
public:
	FxFakeLock() {}
	~FxFakeLock() {}
	void Lock() {}
	void UnLock() {}
	virtual void            Release() { if(NULL != this) delete this; }
};

IFxLock* FxCreateThreadLock()
{
    return new FxCriticalLock;   
}

IFxLock* FxCreateThreadFakeLock()
{
    return new FxFakeLock;   
}

class FxThreadHandler : public IFxThreadHandler
{
public:
    FxThreadHandler(IFxThread *pThread, bool bNeedWaitfor)
    {
        m_bIsStop       = false;
        m_bNeedWaitfor  = bNeedWaitfor;
        m_pThread       = pThread;
    }

    virtual ~FxThreadHandler()
    {
        UINT32 dwErrCode = 0;
        Kill(dwErrCode);
    }

public:
    inline virtual void Resume(void)
    {
    }

    inline virtual void Suspend()
    {
    }

    inline virtual void Stop(void)
    {
        if (NULL != m_pThread)
        {
            m_pThread->Stop();
        }
    }

    inline virtual bool Kill(UINT32 dwExitCode)
    {
        pthread_cancel(m_oPthread);
        return false;
    }

    inline virtual bool WaitFor(UINT32 dwWaitTime = FX_INFINITE)
    {
        if (!m_bNeedWaitfor)
        {
            return false;
        }
        pthread_join(m_oPthread, NULL);
        return true;
    }

    inline virtual void         Release(void)           { delete this;          }
    inline virtual UINT32       GetThreadId(void)       { return m_oPthread;  }
    inline virtual IFxThread*   GetThread(void)         { return m_pThread;     }

    inline bool Start(bool bSuspend)
    {
        if( 0 != pthread_create(&m_oPthread, NULL, (void *(*)(void *))__StaticThreadFunc, this))
        {
            return false;
        }

        return true;
    }

private:
    static unsigned int __StaticThreadFunc(void *arg)
    {
        FxThreadHandler *pThreadCtrl = (FxThreadHandler *)arg;

        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

        sigset_t new_set,old_set;
        sigemptyset(&new_set);
        sigemptyset(&old_set);
        sigaddset(&new_set, SIGHUP);
        sigaddset(&new_set, SIGINT);
        sigaddset(&new_set, SIGQUIT);
        sigaddset(&new_set, SIGTERM);
        sigaddset(&new_set, SIGUSR1);
        sigaddset(&new_set, SIGUSR2);
        sigaddset(&new_set, SIGPIPE);
        pthread_sigmask(SIG_BLOCK, &new_set, &old_set);

        if(false == pThreadCtrl->m_bNeedWaitfor)
        {
            pthread_detach(pthread_self());
        }

        pThreadCtrl->m_pThread->ThrdFunc();
        return 0;
    }

protected:
    bool                    m_bIsStop;
    bool                    m_bNeedWaitfor;
    pthread_t	            m_oPthread;
    IFxThread*              m_pThread;
};

void FxMakeDaemonProcess()
{
    //Do nothing, Daemon是Linux特有的特性
}

IFxThreadHandler* FxCreateThreadHandler(IFxThread* poThread, bool bNeedWaitfor, bool bSuspend)
{
    FxThreadHandler *pThreadCtrl = new FxThreadHandler(poThread, bNeedWaitfor);
    if(NULL == pThreadCtrl)
    {
        return NULL;
    }

    if(false == pThreadCtrl->Start(bSuspend))
    {
        delete pThreadCtrl;
        return NULL;
    }

    return pThreadCtrl;
}

void FxGetTimeEx(STimeVal* pstTime)
{
    struct timezone tz;
    struct timeval tv;

    gettimeofday(&tv, &tz);
    pstTime->m_dwSec=tv.tv_sec;
    pstTime->m_dwUsec=tv.tv_usec;
}

void FxSleep(UINT32 dwMilliseconds)
{
    usleep(dwMilliseconds*1000);
}

bool FxCreateDirectory(const char* pszDir)
{
    char szPath[PATH_MAX];
    size_t nLen = strnlen(pszDir, sizeof(szPath)-1);
    memcpy(szPath, pszDir, nLen);
    szPath[nLen] = '\0';

    char* p1 = szPath;

    while(*p1)
    {
        if('\\' == *p1) *p1 = '/';
        p1++;
    }

    p1 = szPath;

    do
    {
        p1 = strchr(p1, '/');
        if(p1 != NULL)
            *p1 = '\0';

        if(-1 == mkdir(szPath, 0755))
        {
            if( (NULL == p1) && (errno != EEXIST) )
                return false;
        }
        if(p1 != NULL)
            *p1++ = '/';
    }while(p1 != NULL);

    return true;
}

//////////////////////////////////////////////////////////////////////////
struct STimerInfo 
{
    int                 nInterval;
    int                 nAveInterval;
    volatile int        m_nEffectCount;        // 原子操作
    INT64               qwLastTime;
    set<UINT32>         arEvents;
    vector<UINT32>      arOnEvents;
    list<UINT32>        arAddEvents;
    list<UINT32>        arDelEvents;
};

class FxTimerHandler : public IFxTimerHandler
{
public:
    FxTimerHandler()
    {
        m_arTimers.clear();
        m_bStop = false;
        m_poLock = NULL;
    }

    virtual ~FxTimerHandler()
    {
        if (NULL != m_poLock)
        {
            m_poLock->Release();
            m_poLock = NULL;
        }
    }

    virtual bool            Init(IFxTimer* poTimer)
    {
        m_arTimers.clear();

        m_bStop = false;
        m_poLock = FxCreateThreadLock();
        pthread_cond_init(&m_TickCond, NULL);
        pthread_mutex_init(&m_TickLock, NULL);
        
        pthread_create(&m_oPthread, NULL, (void *(*)(void *))__ThreadFunc, this);
        m_poTimer = poTimer;
        return true;
    }

    virtual void            Uninit()
    {
        m_arTimers.clear();
        pthread_cond_destroy(&m_TickCond);
        pthread_mutex_destroy(&m_TickLock);
    }

    virtual bool            Run()
    {
        bool bRet = false;
        INT64 qwNow = GetMicroSecond();
        int nCount = (int)m_arTimers.size();
        for (int i = 0; i < nCount; i++)
        {
            STimerInfo& timer = m_arTimers[i];
            if (0 == timer.m_nEffectCount)
            {
                continue;
            }

            INT32 nMicroSecond = int(qwNow - timer.qwLastTime);
            int nEventCount = (int)timer.arOnEvents.size();
            for (int j = 0; j < nEventCount; ++j)
            {
                m_poTimer->OnTimer(nMicroSecond, timer.arOnEvents[j]);
            }
            timer.qwLastTime = qwNow;
            bRet = true;
            timer.m_nEffectCount = 0;
        }

        WakeUp();
        return bRet;
    }

    virtual void            Stop()
    {
        m_bStop = true;
        pthread_join(m_oPthread, NULL);
    }
 
    virtual bool            AddTimer(int nMicroSecond)
    {
        int nCount = (int)m_arTimers.size();
        for (int i = 0; i < nCount; i++)
        {
            STimerInfo& timer = m_arTimers[i];
            if (nMicroSecond == timer.nInterval)
            {
                return false;
            }
        }

        STimerInfo timer;
        timer.nInterval = nMicroSecond;
        timer.nAveInterval = 0;
        timer.m_nEffectCount = 0;
        timer.qwLastTime = GetMicroSecond();
        m_arTimers.push_back(timer);
        return true;
    }

    virtual bool            AddTimer(int nMicroSecond, UINT32 dwEventId)
    {
        bool bFind = false;
        int nCount = (int)m_arTimers.size();
        for (int i = 0; i < nCount; i++)
        {
            STimerInfo& timer = m_arTimers[i];
            if (nMicroSecond == timer.nInterval)
            {
                m_poLock->Lock();
                timer.arAddEvents.push_back(dwEventId);
                m_poLock->UnLock();
                bFind = true;
                break;
            }
        }

        if (!bFind)
        {
            STimerInfo timer;
            timer.nInterval = nMicroSecond;
            timer.nAveInterval = 0;
            timer.arAddEvents.push_back(dwEventId);
            timer.m_nEffectCount = 0;
            timer.qwLastTime = GetMicroSecond();
            m_arTimers.push_back(timer);
        }
        return true;
    }

    virtual bool            DelTimer(int nMicroSecond, UINT32 dwEventId)
    {
        int nCount = (int)m_arTimers.size();
        for (int i = 0; i < nCount; i++)
        {
            STimerInfo& timer = m_arTimers[i];
            if (nMicroSecond == timer.nInterval)
            {
                m_poLock->Lock();
                timer.arDelEvents.push_back(dwEventId);
                m_poLock->UnLock();
                break;
            }
        }
        return true;
    }

    virtual void            Release()
    {
        if (NULL != m_poLock)
        {
            m_poLock->Release();
            m_poLock = NULL;
        }

        m_arTimers.clear();
    }

    virtual INT64           GetMicroSecond()
    {
        STimeVal val;
        FxGetTimeEx(&val);
        return INT64(val.m_dwSec * 1000) + INT64(val.m_dwUsec / 1000);
    }

    virtual void            Sleep(int nMicroSecond)
    {
	   	timeval now;
	   	timespec timeout;
	   	gettimeofday(&now, 0);
	   	if (now.tv_usec < 999000)
	   	{
		  	timeout.tv_sec = now.tv_sec;
		  	timeout.tv_nsec = (now.tv_usec +  nMicroSecond * 1000) * 1000;
	   	}
	   	else
	   	{
		  	timeout.tv_sec = now.tv_sec + nMicroSecond / 1000;
		  	timeout.tv_nsec = (now.tv_usec + nMicroSecond % 1000 * 1000) * 1000;
	  	}

        pthread_mutex_lock(&m_TickLock);
        pthread_cond_timedwait(&m_TickCond, &m_TickLock, &timeout);
        pthread_mutex_unlock(&m_TickLock);
    }

    virtual void            WakeUp()
    {
        pthread_cond_signal(&m_TickCond);
    }

private:
    void                    __Refresh(INT64 qwNow)
    {
        int nCount = (int)m_arTimers.size();
        for (int i = 0; i < nCount; i++)
        {
            STimerInfo& timer = m_arTimers[i];
            if (0 == timer.m_nEffectCount)
            {
                int nInterval = timer.nInterval - timer.nAveInterval - 2;
                INT32 nMicroSecond = int(qwNow - timer.qwLastTime);
                if (nMicroSecond >= nInterval)
                {
                    int nEventCount = (int)timer.arEvents.size();
                    timer.arOnEvents.resize(nEventCount);
                    int index = 0;
                    for (set<UINT32>::iterator it = timer.arEvents.begin(); timer.arEvents.end() != it; ++it)
                    {
                        timer.arOnEvents[index++] = *it;
                    }

                    if (0 == index)
                    {// 空的，这里帮他重新设置一下时间
                        timer.qwLastTime = GetMicroSecond();
                    }
                    
                    timer.m_nEffectCount = index;
                    timer.nAveInterval = (timer.nAveInterval * 9 + int(GetMicroSecond() - qwNow)) / 10;     // 10的移动平均数
                }
            }

            if (timer.arAddEvents.empty() && timer.arDelEvents.empty())
            {
                continue;
            }

            // 先加后删。不考虑一个时间同时在删和加的表中，这种情况可能出错
            while (true)
            {
                m_poLock->Lock();
                if (timer.arAddEvents.empty())
                {
                    m_poLock->UnLock();
                    break;
                }
                
                UINT32 eventId = timer.arAddEvents.front();
                timer.arAddEvents.pop_front();
                m_poLock->UnLock();
                timer.arEvents.insert(eventId);
            }

            while (true)
            {
                m_poLock->Lock();
                if (timer.arDelEvents.empty())
                {
                    m_poLock->UnLock();
                    break;
                }
                
                UINT32 eventId = timer.arDelEvents.front();
                timer.arDelEvents.pop_front();
                m_poLock->UnLock();
                timer.arEvents.erase(eventId);
            }
        }
    }

    static unsigned int __ThreadFunc(void* pParam)
    {
        FxTimerHandler* poTimerHandler = (FxTimerHandler*)pParam;

        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

        sigset_t new_set,old_set;
        sigemptyset(&new_set);
        sigemptyset(&old_set);
        sigaddset(&new_set, SIGHUP);
        sigaddset(&new_set, SIGINT);
        sigaddset(&new_set, SIGQUIT);
        sigaddset(&new_set, SIGTERM);
        sigaddset(&new_set, SIGUSR1);
        sigaddset(&new_set, SIGUSR2);
        sigaddset(&new_set, SIGPIPE);
        pthread_sigmask(SIG_BLOCK, &new_set, &old_set);

        pthread_detach(pthread_self());

        while (!poTimerHandler->m_bStop)
        {
            INT64 qwNow = poTimerHandler->GetMicroSecond();
            poTimerHandler->__Refresh(qwNow);
            if (5 > (poTimerHandler->GetMicroSecond() - qwNow))
            {
                poTimerHandler->Sleep(1);
            }
        }

        return 0;
    }

private:
    bool                m_bStop;
    IFxLock*            m_poLock;
    IFxTimer*           m_poTimer;
    pthread_t	        m_oPthread;
    pthread_cond_t      m_TickCond;
    pthread_mutex_t     m_TickLock;

    vector<STimerInfo>  m_arTimers;     // 时钟队列
};

IFxTimerHandler*            FxCreateTimer(IFxTimer* poTimer)
{
    FxTimerHandler* poTimerHandler = new FxTimerHandler;
    if (!poTimerHandler->Init(poTimer))
    {
        poTimerHandler->Uninit();
        poTimerHandler->Release();
        return NULL;
    }
    
    return poTimerHandler;
}

