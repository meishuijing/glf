#ifndef __CRASH_HELPER_H__
#define __CRASH_HELPER_H__

#include <Windows.h>
class CrashHelper
{
public:

public:
	static CrashHelper* Instance() { static CrashHelper ins; return &ins;}
	
	bool Initalize();

	bool DumpProgram(_EXCEPTION_POINTERS * pExceptionInfo = 0);

private:
	bool ScreenShot(const char* pszFileName);

public:
	static LONG WINAPI UnhandledExceptionFilterInProcess(_EXCEPTION_POINTERS* pExceptionInfo);
	static LONG WINAPI UnhandledExceptionFilter(_EXCEPTION_POINTERS* pExceptionInfo);

private:
	static DWORD ms_dwCreashThreadId;
};

#endif //__CRASH_HELPER_H__