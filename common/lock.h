#ifndef __LOCK_H_2009_0901__
#define __LOCK_H_2009_0901__

#include "stdafx.h"

class FxLock
{
	void operator = (FxLock &locker) {}
public:
#ifdef WIN32
	FxLock()
	{
		InitializeCriticalSection(&m_lock);
	}
	~FxLock()
	{
		DeleteCriticalSection(&m_lock);
	}
	void Lock()
	{
		EnterCriticalSection(&m_lock);
	}
	void Unlock()
	{
		LeaveCriticalSection(&m_lock);
	}
#else
	FxLock()
	{
		pthread_mutex_init(&m_lock, NULL);
	}
	~FxLock()
	{
		pthread_mutex_destroy(&m_lock);
	}
	void Lock()
	{
		pthread_mutex_lock(&m_lock);
	}
	void Unlock()
	{
		pthread_mutex_unlock(&m_lock);
	}
#endif

protected:
#ifdef WIN32
	CRITICAL_SECTION m_lock;
#else
	pthread_mutex_t m_lock;
#endif
};


class FxLockImp
{
public:
	FxLockImp(FxLock* pLock)
	{
		m_pLock = pLock;
		pLock->Lock();
	}

	~FxLockImp()
	{
		m_pLock->Unlock();
	}

private:
	FxLock* m_pLock;
};

#endif  // __LOCK_H_2009_0901__
