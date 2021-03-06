#ifndef __THRDQUEUE_H_NICK_2009_1203__
#define __THRDQUEUE_H_NICK_2009_1203__

#include "lock.h"

template <class T>
class TEventQueue
{
public:
    TEventQueue(void)
    {
        m_nOutCursor    = 0;
        m_nInCursor     = 0;
        m_nOutCursor    = 0;
        m_poArrayT      = NULL;
    };

    virtual ~TEventQueue(void)
    {
        if (NULL != m_poArrayT)
        {
            free(m_poArrayT);
            // delete [] m_poArrayT;
            m_poArrayT = NULL;
        }
    };

	inline bool Init(int nMax)
    {
        if (nMax != m_nSize)
        {
            m_poArrayT = (T*)realloc(m_poArrayT, sizeof(T) * nMax);
            if (NULL == m_poArrayT)
            {
                return false;
            }
        }

//         if (NULL != m_poArrayT)
//         {
//             delete [] m_poArrayT;
//             m_poArrayT = NULL;
//         }
// 
//         m_poArrayT = new T[nMax];
//         if (NULL == m_poArrayT)
//         {
//             return false;
//         }

        m_nInCursor     = 0;
        m_nOutCursor    = 0;
        m_nSize         = nMax;
        return true;
    }

    inline int  GetSize()
    {
        return m_nSize;
    }

	inline bool PushBack(T& ot)
    {
        m_oLock.Lock();
        int nSize = m_nInCursor + m_nSize - m_nOutCursor;
        nSize = nSize >= m_nSize ? (nSize - m_nSize) : nSize;
        if (m_nSize - 1 <= nSize)
        {
            m_oLock.Unlock();
            return false;
        }
        m_poArrayT[m_nInCursor] = ot;
        if(++m_nInCursor >= m_nSize)
            m_nInCursor = 0;

        m_oLock.Unlock();
        return true;
    }

    inline T* PopFront()
    {
        // 
        // 取事件都是在单线程中进行，即时以后使用多线程处理事件，
        // 那么也能保证该事件队列同时只会有一个线程来取事件
        // 所以这里只要保证推入推出不会错乱即可
        // 而采用循环队列的方式，线计算计数，不会出现推入推出错误问题
        // 所以不用加锁，而且因为有很多线程会推入，也同时往往有一个线程正在 取事件
        // 如果加锁，会很影响性能
        // 
        //m_oLock.Lock();
        int nSize = m_nInCursor + m_nSize - m_nOutCursor;
        nSize = nSize >= m_nSize ? (nSize - m_nSize) : nSize;
        if(0 == nSize)
        {
            //m_oLock.Unlock();
            return NULL;
        }

        T* ptr = &m_poArrayT[m_nOutCursor];
        if(++m_nOutCursor >= m_nSize)
            m_nOutCursor = 0;

        //m_oLock.Unlock();

        return ptr;
    }


private:
    T*          m_poArrayT;
    int         m_nSize;
    int         m_nInCursor;
    int         m_nOutCursor;
    FxLock      m_oLock;
};

#endif  // __THRDQUEUE_H_NICK_2009_1203__
