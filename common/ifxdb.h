/*
 * FileName:    FxDB.h
 * Author:      Nick  Version: FxLib V1.00  Date: 2010-4-10
 * Description: DBģ��ͷ�ļ�   
 * Version:     V0.01
 * History:        
 * 	<author>	<time>		<version>	<desc>
 * 	<Nick>		<2010-4-10>	<V0.01>		����
 */

#ifndef __FXDB_H_2009_0824__
#define __FXDB_H_2009_0824__

#include "fxmeta.h"

#define MAX_DBNAME_SIZE		            64
#define MAX_DBCHARACTSET_SIZE		    32
#define MAX_DBQUERY_SIZE				100

enum EFxDBCode
{
    FXDB_ERR_UNKNOWN    = -4,       // δ֪����
    FXDB_ERR_READER     = -3,       // ��������ݽ������
    FXDB_ERR_CONN       = -2,       // ���Ӵ���
    FXDB_DISCONNECT     = -1,       // �Ͽ�����
    FXDB_SUCCESS        = 0,        // �ɹ�
    FXDB_NO_RESULT      = 1,        // �ɹ����޽��
    FXDB_HAS_RESULT     = 2         // �ɹ����в�ѯ���
};

struct SDBAccount{
    char                m_szHostName[MAX_DBNAME_SIZE];        
    char                m_szDBName[MAX_DBNAME_SIZE];          
    char                m_szLoginName[MAX_DBNAME_SIZE];       
    char                m_szLoginPwd[MAX_DBNAME_SIZE];        
	char                m_szCharactSet[MAX_DBCHARACTSET_SIZE];  
    UINT16              m_wConnPort;					          
};

struct SDBConnInfo
{    
    UINT32              m_dwDBId;           
	SDBAccount          m_stAccount; 
};

class IDataReader
{
public:
    virtual   ~IDataReader(){}
	
	virtual UINT32      GetRecordCount(void) = 0;
    
    virtual UINT32      GetFieldCount(void) = 0;
    
    virtual bool        GetFirstRecord(void) = 0;

    virtual bool        GetNextRecord(void) = 0;
	
    virtual const char* GetFieldValue(UINT32 dwIndex) = 0;
	
    virtual INT32       GetFieldLength(UINT32 dwIndex) = 0;
	
    virtual void        Release(void)=0;
};

class IDBConnection
{
public:	
    virtual   ~IDBConnection(){}

    virtual UINT32      EscapeString(const char* pszSrc, INT32 nSrcLen, char* pszDest, INT32 nDestLen) = 0;
    
	virtual INT32       Query(const char* pszSQL) = 0;
	
    virtual INT32       Query(const char* pszSQL, IDataReader** ppReader) = 0;
	
	virtual int      GetLastError(void) = 0;
    
    virtual const char* GetLastErrorString(void) = 0;
};

class IQuery 
{
public:
    virtual   ~IQuery(){}

    virtual INT32       GetDBId(void) = 0;
	
	virtual void        OnQuery(IDBConnection *poDBConnection) = 0;
	
    virtual void        OnResult(void) = 0;
	
    virtual void        Release(void) = 0;
};

class IDBModule
{
public:
    virtual   ~IDBModule(){}

	virtual	bool		Init() = 0;

	virtual bool        Open(SDBConnInfo& DBInfo) = 0;
	
	virtual void        Close(UINT32 dwDBId) = 0;
    	
	virtual bool        AddQuery(IQuery *poQuery) = 0;
	
	virtual bool        Run(UINT32 dwCount = -1) = 0;
};

IDBModule *             FxDBGetModule();

typedef IDBModule *     (*PFN_FxDBGetModule)();

#endif  // __FXDB_H_2009_0824__
