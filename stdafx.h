// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//


#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0500	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include <tchar.h>
#include <process.h>
#include "common/fxmeta.h"
#include <MSWSock.h>

class CGSocket;
struct SPer_IO_Data
{
	OVERLAPPED overlapped;
	WSABUF	databuf;
	char buf[128];
	int nOperType;
	CGSocket* poSocket;
};

#else

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>

#define Sleep sleep

#endif



;


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�