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
	//Э����������
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

	UINT16	m_wMessPushBeginCursor;//�����ڽ��ܷ����ж�����������push��ʱ����־�����Ƴ��İ�����û��ȷ�϶������α꣬��ֵ�ض���beign��end�α�֮��
};

#endif// __RINGBUFFER_MEISHUIJING_2015_04_01__