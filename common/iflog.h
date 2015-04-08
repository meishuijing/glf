#ifndef __ILOGOPER_MEISHUIJING_2015_03_25__
#define __ILOGOPER_MEISHUIJING_2015_03_25__

#include "lock.h"


#define MAX_CONTENT_LENGTH 102

enum
{
	LOG_LEVEL_NONE = -1,
	LOG_LEVEL_DBG,
	LOG_LEVEL_USR,
	LOG_LEVEL_SYS,
	LOG_LEVEL_MAX
};

class ILogOper
{
public:
	virtual ~ILogOper(){};
	virtual bool Init(const char* pszName, int nLevel) = 0;
	virtual void Log(int nLevel, const char* pFunction, int nLine, const char* format, ...) = 0;;
};


#define DBG_LOG_OPER(x, ...) GetLogModule()->Log(LOG_LEVEL_DBG, __FUNCTION__, __LINE__, x, __VA_ARGS__)
#define USR_LOG_OPER(x, ...)  GetLogModule()->Log(LOG_LEVEL_USR,__FUNCTION__, __LINE__, x, __VA_ARGS__)
#define SYS_LOG_OPER(x, ...)  GetLogModule()->Log(LOG_LEVEL_SYS,__FUNCTION__, __LINE__, x, __VA_ARGS__)


ILogOper* GetLogModule();

#endif// __ILOGOPER_MEISHUIJING_2015_03_25__