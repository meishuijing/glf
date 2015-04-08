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
            // ����malloc / free��ʽԤ�����ڴ� nijie 20090428
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

        // Ϊ�˿�����trace���٣�Ϊÿ��ģ���������һ���
        // ���·���ʱ���ü�����Ϊ0��˵���ö����ϴ�ʹ���ͷ��������⣬��������ص�ʹ�ó�BUG��
        if (0 != poShellT->m_byRef)
        {
            //char szMsg[512];
            //sprintf(szMsg, "poShellT remalloc Error %u\n", poShellT->m_byRef);
            m_pFreeLock->UnLock();
            return NULL;
        }
        // ÿ�η����Ҫ�������ü���
        poShellT->m_byRef++;

        // placement new
        // new(poShellT) FxShellT;
        // �������乹�캯��

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
        
        // �ȼ���õ��ǵ�λ�ã�����ÿ�����������ܶ��ֽڶ����֧�ֲ�һ�������ﲻ����ֱ�Ӷ��룬
        // ����ֱ�Ӽ���ƫ�����õ��ǵ�ָ��
        FxShellT* poShellT = static_cast<FxShellT*>(pObj);     
        if (NULL == poShellT)
        {
            m_pFreeLock->UnLock();
            return;
        }

        // �����ʱ���ü�����Ϊ1��Ҳ������ʹ�ó���
        // ��Ӧ�ö�������ͷţ������������������ͷ����
        if (0 != poShellT->m_byRef - 1)
        {
//             char szMsg[512];
//             sprintf(szMsg, "poShellT Release Error %u\n", poShellT->m_byRef);
            m_pFreeLock->UnLock();
            return;
        }

        // ÿ���ͷžͶ����ü�����1
        poShellT->m_byRef--;

        // ��ʾ��������������
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

        // ����malloc / free��ʽԤ�����ڴ� nijie 20090428
        // FxShellT* poShellT = (FxShellT*) malloc (sizeof(FxShellT) * dwCount);
        FxShellT* poShellT = new FxShellT[dwCount];

        if(poShellT == NULL)
        {
            m_pFreeLock->UnLock();
            return false;
        }

        // �����������ƫ�����������ͷ�ʱ����õ��ǵ�ָ��
        //if (0 == m_nOffset)
        //{
        //    m_nOffset = int((char*)&(poShellT->m_oT) - (char*)poShellT);
        //}

        for(UINT32 i = 0; i < dwCount; ++i)
        {
            // ���ü�����Ϊ0
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
    // ���������ü����Ŀ�
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
    // ��������ƫ����������linux��g++�����Ľṹ�ֽڶ���(����ģ��templateʱ)��vc��ͬ��
    // ���Ըɴ�ͳһ���ü���ƫ�����ķ�ʽ��������һ�ָ�ֱ�۰�ȫ�ļ���
    //int     m_nOffset;
};

#endif  // __DYNAMICPOOLEX_H_2009_1203__
