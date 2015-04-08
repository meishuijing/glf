// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//


#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include <tchar.h>
#include <process.h>

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


#endif



#define DECLARE_SINGLETON(cls)\
private:\
    static cls* m_poInstance;\
public:\
    static bool CreateInstance()\
			    {\
    if(NULL == m_poInstance)\
    m_poInstance = new cls;\
    return m_poInstance != NULL;\
			    }\
    static cls* Instance(){ return m_poInstance; }\
    static void DestroyInstance()\
			    {\
    if(m_poInstance != NULL)\
		        {\
        delete m_poInstance;\
        m_poInstance = NULL;\
		        }\
			    }

#define IMPLEMENT_SINGLETON(cls) \
    cls* cls::m_poInstance = NULL;


// TODO:  在此处引用程序需要的其他头文件
