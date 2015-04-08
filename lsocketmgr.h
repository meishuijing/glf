#ifndef __LSOCKETMGR_MEISHUIJING_2015_04_03__
#define __LSOCKETMGR_MEISHUIJING_2015_04_03__

#include "lsocket.h"
#include "common/dynamicpoolex.h"

class CLSocketMgr
{
public:
	static CLSocketMgr* Instance();

	bool		Init();
	void		Uninit();

	CLSocket*	FetchSocket();
	void		ReleaseSocket(CLSocket* pSocket);

private:
	UINT32	m_uMarkIndex;
	TDynamicPoolEx<CLSocket> m_poGSocket;
};

#endif// __LSOCKETMGR_MEISHUIJING_2015_04_03__