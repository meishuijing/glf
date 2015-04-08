#ifndef __RINGBUFFER_MEISHUIJING_2015_04_01__
#define __RINGBUFFER_MEISHUIJING_2015_04_01__

#include "common/fxmeta.h"
#include "common/ifnet.h"
#include "common/packetparser.h"
#include "common/lock.h"

class CConnection;
class CRingBuffer
{
public:
	CRingBuffer();
	//协议包缓存操作
	void	GetBufPushPtrAndLen(char*& pPush, UINT16& wPushLen);
	void	GetBufPopPtrAndLen(char*& pPop, UINT16& wPopLen);
	void	DealBuffer2Packet(CConnection* pConnection);
	UINT16	AddBufLen(UINT16 wLen);

	UINT16	GetRealLen(bool IsPushBegin = false);
	bool	MemcpyBuf(char* dst, UINT16 wLen, bool IsPushBegin = false);
	void	DiscardLen(UINT16 wLen);
	void	DiscardMessPushLen(UINT16 wLen);

private:
	char	m_szMessBuf[MAX_MESSAGE_LEN];
	UINT16	m_wMessEndCursor;
	UINT16	m_wMessBeginCursor;
	FxLock	m_oBuffLock;
	FxPacketParser m_oParser;

	UINT16	m_wMessPushBeginCursor;//用作在接受返回判定有整包可以push出时，标志着已推出的包并还没有确认丢弃的游标，其值必定在beign和end游标之间
};

#endif// __RINGBUFFER_MEISHUIJING_2015_04_01__