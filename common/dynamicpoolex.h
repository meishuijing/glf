#ifndef __DYNAMICPOOLEX_H_2009_1203__
#define __DYNAMICPOOLEX_H_2009_1203__

#include "stdafx.h"
#include "fxmeta.h"
#include <list>

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
		pthread_mutex_init(&m_lock, NULL);
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

	virtual void            Release() { if (NULL != this) delete this; }

private:
#ifdef WIN32
	CRITICAL_SECTION m_lock;
#else
	pthread_mutex_t m_lock;
#endif
};

class FxFakeLock : public IFxLock
{
public:
    FxFakeLock(){}
    ~FxFakeLock(){}
    
    virtual void Lock(){}
    virtual void UnLock(){}
    virtual void Release(){ delete this; }
};

// nijie modify for placement new 20090428

template <class T>
class TDynamicPoolEx
{
public:
    explicit TDynamicPoolEx(void)
	{
	   	m_dwInitCount = 0;
	   	m_dwGrowCount =0;
        m_pAllocLock = NULL;
		m_pFreeLock = NULL;
    }

    virtual ~TDynamicPoolEx(void)
    {
        for(typename std::list<FxShellT*>::iterator itr = m_ListFree.begin(); itr != m_ListFree.end(); itr++)
            *itr = NULL;

		m_ListFree.clear();
        
		for(typename std::list<FxShellT*>::iterator it = m_ListAll.begin(); it != m_ListAll.end(); it++)
        {
            // 该用malloc / free方式预分配内存 nijie 20090428
            // (*itr)->~FxShellT();
            // free(*itr);
            delete [] *it;
            *it = NULL;
        }
		m_ListAll.clear();
        
		if (m_pFreeLock)
		{
			m_pFreeLock->Release();
			m_pFreeLock = NULL;
		}
		
		if (m_pAllocLock)
		{
			m_pAllocLock->Release();
			m_pAllocLock = NULL;
		}
    }

    bool Init(UINT32 dwInitCount, UINT32 dwGrowCount, bool bSingleThread = false)
    {
        m_dwInitCount = dwInitCount;
        m_dwGrowCount = dwGrowCount;
        
        if (!bSingleThread)
        {
			m_pFreeLock = new FxCriticalLock;
			m_pAllocLock = new FxCriticalLock;
        }
		else
		{
			m_pAllocLock = new FxFakeLock;
			m_pFreeLock  = new FxFakeLock;
		}

		if (!m_pFreeLock || !m_pAllocLock)
			return false;

        return _AllocT(m_dwInitCount);
    }

    T* FetchObj(void)
    {
        FxShellT* poShellT = NULL;
        m_pFreeLock->Lock();
        if(m_ListFree.empty())
        {
            m_pFreeLock->UnLock();
            if(!_AllocT(m_dwGrowCount))
            {
                return NULL;
            }
            m_pFreeLock->Lock();
        }

        poShellT = m_ListFree.front();
        if (NULL == poShellT)
        {
            m_pFreeLock->UnLock();
            return NULL;
        }

        // 为了可以做trace跟踪，为每个模板对象增加一层壳
        // 当新分配时引用计数不为0，说明该对象上次使用释放上有问题，或者这个池的使用出BUG了
        if (0 != poShellT->m_byRef)
        {
            //char szMsg[512];
            //sprintf(szMsg, "poShellT remalloc Error %u\n", poShellT->m_byRef);
            m_pFreeLock->UnLock();
            return NULL;
        }
        // 每次分配后要增加引用计数
        poShellT->m_byRef++;

        // placement new
        // new(poShellT) FxShellT;
        // 不调用其构造函数

        m_ListFree.pop_front();
        m_pFreeLock->UnLock();

        return poShellT;
    }

    void ReleaseObj(T* pObj)
    {
        if (NULL == pObj)
        {
            return;
        }

        m_pFreeLock->Lock();
        
        // 先计算得到壳的位置，用于每个编译器可能对字节对齐的支持不一样，这里不采用直接对齐，
        // 而是直接计算偏移量得到壳的指针
        FxShellT* poShellT = static_cast<FxShellT*>(pObj);     
        if (NULL == poShellT)
        {
            m_pFreeLock->UnLock();
            return;
        }

        // 如果此时引用计数不为1，也即对其使用出错，
        // 则不应该对其进行释放，调用他的析构函数和放入池
        if (0 != poShellT->m_byRef - 1)
        {
//             char szMsg[512];
//             sprintf(szMsg, "poShellT Release Error %u\n", poShellT->m_byRef);
            m_pFreeLock->UnLock();
            return;
        }

        // 每次释放就对引用计数减1
        poShellT->m_byRef--;

        // 显示调用其析构函数
        // poShellT->~FxShellT();
        m_ListFree.push_back(poShellT);
        m_pFreeLock->UnLock();
    }

    size_t GetFreeCount(void)
    {
        size_t n = 0;
        m_pFreeLock->Lock();
        n = m_ListFree.size();
        m_pFreeLock->UnLock();
        return n;
    }

protected:
    bool _AllocT(UINT32 dwCount)
    {
        if (0 == dwCount)
        {
            return false;
        }

        m_pFreeLock->Lock();
        if(!m_ListFree.empty())
        {
            m_pFreeLock->UnLock();
            return true;
        }

        // 该用malloc / free方式预分配内存 nijie 20090428
        // FxShellT* poShellT = (FxShellT*) malloc (sizeof(FxShellT) * dwCount);
        FxShellT* poShellT = new FxShellT[dwCount];

        if(poShellT == NULL)
        {
            m_pFreeLock->UnLock();
            return false;
        }

        // 计算壳与对象的偏移量，用于释放时计算得到壳的指针
        //if (0 == m_nOffset)
        //{
        //    m_nOffset = int((char*)&(poShellT->m_oT) - (char*)poShellT);
        //}

        for(UINT32 i = 0; i < dwCount; ++i)
        {
            // 引用计算置为0
            poShellT[i].m_byRef = 0;
            m_ListFree.push_back(&poShellT[i]);
        }
        m_pFreeLock->UnLock();

        m_pAllocLock->Lock();
        m_ListAll.push_back(poShellT);
        m_pAllocLock->UnLock();

#ifdef _DEBUG
// 		char szMsg[512];
//		sprintf(szMsg, "Free: %u, All: %u\n", m_ListFree.size(), m_ListAll.size());
#endif
        return true;
    }

private:
    // 增加了引用计数的壳
    class FxShellT : public T
    {
    public:
        FxShellT(){}
        ~FxShellT(){}
        UINT8   m_byRef;
        //T       m_oT;
    };

	std::list<FxShellT*> m_ListFree;
    IFxLock* m_pFreeLock;

	std::list<FxShellT*> m_ListAll;
    IFxLock* m_pAllocLock;

    UINT32  m_dwInitCount;
    UINT32  m_dwGrowCount;
    // 壳与对象的偏移量，由于linux下g++产生的结构字节对齐(包含模板template时)与vc不同，
    // 所以干脆统一采用计算偏移量的方式，※渴望一种更直观安全的技巧
    //int     m_nOffset;
};

#endif  // __DYNAMICPOOLEX_H_2009_1203__
