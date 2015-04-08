#ifndef __FXBASE_H_2009_0817__
#define __FXBASE_H_2009_0817__

typedef signed char         INT8;
typedef unsigned char       UINT8;
typedef signed short        INT16;
typedef unsigned short      UINT16;
typedef signed int          INT32;
typedef unsigned int        UINT32;
typedef signed long long    INT64;
typedef unsigned long long  UINT64;
#ifndef NULL
#define NULL 0
#endif
class IFxLock
{
public:
    virtual  ~IFxLock(){}

    virtual void            Lock() = 0;

    virtual void            UnLock() = 0;

    virtual void            Release() = 0;
};

#endif
