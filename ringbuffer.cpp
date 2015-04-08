#include "ringbuffer.h"
#include "connection.h"

class CFuncLock
{
public:
	CFuncLock(FxLock* pLock)
	{
		pLock->Lock();
		m_pLock = pLock;
	}
	~CFuncLock()
	{
		m_pLock->Unlock();
	}

private:
	FxLock* m_pLock;

};
void CRingBuffer::GetBufPushPtrAndLen(char*& pPush, UINT16& wPushLen)
{
	CFuncLock oFuncLock(&m_oBuffLock);
	if (m_wMessBeginCursor > m_wMessEndCursor)
	{
		pPush = m_szMessBuf + m_wMessEndCursor;
		wPushLen = m_wMessBeginCursor - m_wMessEndCursor - 1;
	}
	else
	{
		pPush = m_szMessBuf + m_wMessEndCursor;
		if (m_wMessBeginCursor == 0)
		{
			//开始标签和结束标签不能重合，不然视为没有数据
			wPushLen = MAX_MESSAGE_LEN - m_wMessEndCursor - 1;
		}
		else
		{
			wPushLen = MAX_MESSAGE_LEN - m_wMessEndCursor;
		}
	}
}

void CRingBuffer::GetBufPopPtrAndLen(char*& pPop, UINT16& wPopLen)
{
	CFuncLock oFuncLock(&m_oBuffLock);
	pPop = m_szMessBuf + m_wMessBeginCursor;
	if (m_wMessBeginCursor > m_wMessEndCursor)
	{
		if (m_wMessEndCursor == 0)
		{
			wPopLen = MAX_MESSAGE_LEN - m_wMessBeginCursor - 1;
		}
		else
		{
			wPopLen = MAX_MESSAGE_LEN - m_wMessBeginCursor;
		}
	}
	else
	{
		wPopLen = m_wMessEndCursor - m_wMessBeginCursor;
	}
}

void CRingBuffer::DealBuffer2Packet(CConnection* pConnection)
{
	UINT16 wRealLen = GetRealLen(true);
	if (wRealLen >= sizeof(SPacketHeader))
	{
		//判定协议头是否存在
		char szPacket[sizeof(SPacketHeader)];
		MemcpyBuf(szPacket, sizeof(SPacketHeader), true);
		int nPacketLen = m_oParser.ParsePacket(szPacket, sizeof(SPacketHeader));
		if (-1 == nPacketLen)
		{
			DiscardLen(sizeof(SPacketHeader));
			pConnection->PushEvent(NETEVT_ERROR, NET_PACKET_ERROR);
			pConnection->Close();
		}
		else
		{
			if (nPacketLen <= wRealLen)
			{
				//协议包完整可以推出
				DiscardMessPushLen(nPacketLen);
				pConnection->PushEvent(NETEVT_RECV, nPacketLen);
				return;
			}
		}
	}
}

UINT16 CRingBuffer::AddBufLen(UINT16 wLen)
{
	m_oBuffLock.Lock();
	m_wMessEndCursor += wLen;
	if (m_wMessEndCursor >= MAX_MESSAGE_LEN)
	{
		m_wMessEndCursor -= MAX_MESSAGE_LEN;
	}
	m_oBuffLock.Unlock();

	return MAX_MESSAGE_LEN - GetRealLen();
}
UINT16 CRingBuffer::GetRealLen(bool IsPushBegin)
{
	if (IsPushBegin)
	{
		m_oBuffLock.Lock();
		if (m_wMessPushBeginCursor > m_wMessEndCursor)
		{
			UINT16 wRet = (MAX_MESSAGE_LEN - m_wMessPushBeginCursor) + m_wMessEndCursor;
			m_oBuffLock.Unlock();
			return wRet;
		}
		else
		{
			UINT16 wRet = m_wMessEndCursor - m_wMessPushBeginCursor;
			m_oBuffLock.Unlock();
			return wRet;
		}
	}
	else
	{
		m_oBuffLock.Lock();
		if (m_wMessBeginCursor > m_wMessEndCursor)
		{
			UINT16 wRet = (MAX_MESSAGE_LEN - m_wMessBeginCursor) + m_wMessEndCursor;
			m_oBuffLock.Unlock();
			return wRet;
		}
		else
		{
			UINT16 wRet = m_wMessEndCursor - m_wMessBeginCursor;
			m_oBuffLock.Unlock();
			return wRet;
		}
	}
}

bool CRingBuffer::MemcpyBuf(char* dst, UINT16 wLen, bool IsPushBegin)
{
	if (wLen > GetRealLen(IsPushBegin))
	{
		return false;
	}

	if (IsPushBegin)
	{
		m_oBuffLock.Lock();
		if (m_wMessPushBeginCursor > m_wMessEndCursor)
		{
			if (wLen <= (MAX_MESSAGE_LEN - m_wMessPushBeginCursor))
			{
				memcpy(dst, m_szMessBuf + m_wMessPushBeginCursor, wLen);
			}
			else
			{
				UINT16 wFirst = MAX_MESSAGE_LEN - m_wMessPushBeginCursor;
				UINT16 wSecond = wLen - wFirst;
				memcpy(dst, m_szMessBuf + m_wMessPushBeginCursor, wFirst);
				memcpy(dst, m_szMessBuf, wSecond);
			}
		}
		else
		{
			memcpy(dst, m_szMessBuf + m_wMessPushBeginCursor, wLen);
		}
		m_oBuffLock.Unlock();
	}
	else
	{
		m_oBuffLock.Lock();
		if (m_wMessBeginCursor > m_wMessEndCursor)
		{
			if (wLen <= (MAX_MESSAGE_LEN - m_wMessBeginCursor))
			{
				memcpy(dst, m_szMessBuf + m_wMessBeginCursor, wLen);
			}
			else
			{
				UINT16 wFirst = MAX_MESSAGE_LEN - m_wMessBeginCursor;
				UINT16 wSecond = wLen - wFirst;
				memcpy(dst, m_szMessBuf + m_wMessBeginCursor, wFirst);
				memcpy(dst, m_szMessBuf, wSecond);
			}
		}
		else
		{
			memcpy(dst, m_szMessBuf + m_wMessBeginCursor, wLen);
		}
		m_oBuffLock.Unlock();
	}
	return true;
}

void CRingBuffer::DiscardLen(UINT16 wLen)
{
	m_oBuffLock.Lock();
	if (m_wMessBeginCursor > m_wMessEndCursor)
	{
		m_wMessBeginCursor += wLen;
		if (m_wMessBeginCursor > MAX_MESSAGE_LEN)
		{
			m_wMessBeginCursor -= MAX_MESSAGE_LEN;
			if (m_wMessBeginCursor > m_wMessEndCursor)
			{
				m_wMessBeginCursor = m_wMessEndCursor;
			}
		}
	}
	else
	{
		m_wMessBeginCursor += wLen;
		if (m_wMessBeginCursor > m_wMessEndCursor)
		{
			m_wMessBeginCursor = m_wMessEndCursor;
		}
	}
	m_oBuffLock.Unlock();
}

CRingBuffer::CRingBuffer()
{
	m_wMessEndCursor = 0;
	m_wMessBeginCursor = 0;
	m_wMessPushBeginCursor = 0;
}

void CRingBuffer::DiscardMessPushLen(UINT16 wLen)
{
	m_oBuffLock.Lock();
	if (m_wMessPushBeginCursor > m_wMessEndCursor)
	{
		m_wMessPushBeginCursor += wLen;
		if (m_wMessPushBeginCursor > MAX_MESSAGE_LEN)
		{
			m_wMessPushBeginCursor -= MAX_MESSAGE_LEN;
			if (m_wMessPushBeginCursor > m_wMessEndCursor)
			{
				m_wMessPushBeginCursor = m_wMessEndCursor;
			}
		}
	}
	else
	{
		m_wMessPushBeginCursor += wLen;
		if (m_wMessPushBeginCursor > m_wMessEndCursor)
		{
			m_wMessPushBeginCursor = m_wMessEndCursor;
		}
	}
	m_oBuffLock.Unlock();
}
