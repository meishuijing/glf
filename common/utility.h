#ifndef __UTILITY_MEISHUIJING_2015_03_23__
#define __UTILITY_MEISHUIJING_2015_03_23__

#ifdef WIN32
#include <time.h>
#else
#include <stdlib.h>
#include <sys/time.h>
#endif
#include <vector>
#include <string>
#include "fxmeta.h"
#include <io.h>
#include <direct.h>

using namespace std;

inline void SplitString(string str, char c, vector<string>& vec)
{
	if (str.empty())
		return;

	size_t pos = str.find_first_of(c, 0);
	if (string::npos == pos)
	{
		vec.push_back(str);
		return;
	}

	vec.push_back(str.substr(0, pos));
	if (pos < str.length() - 1)
		return SplitString(str.substr(pos + 1, str.length()), c, vec);
}

inline int GetRandom(int nMin, int nMax)
{
	int nDiffer = nMax - nMin;
	if (nDiffer <= 0)
		return 0;

	int nRand = rand() % nDiffer;
	return nRand + nMin;
}

/************************************************************************
* 二进制字节数组转换十六进制字符串函数
* 输入：
*       data 二进制字节数组
*       size 二进制字节数组长度
* 输出：
*       十六进制字符串，需要free函数释放空间，失败返回NULL
*
************************************************************************/
inline char *bin2hex(unsigned char *data, int size)
{
	int  i = 0;
	int  v = 0;
	char *p = NULL;
	char *buf = NULL;
	char base_char = 'A';

	buf = p = (char *)malloc(size * 2 + 1);
	for (i = 0; i < size; i++) {
		v = data[i] >> 4;
		*p++ = v < 10 ? v + '0' : v - 10 + base_char;

		v = data[i] & 0x0f;
		*p++ = v < 10 ? v + '0' : v - 10 + base_char;
	}

	*p = '\0';
	return buf;
}

/************************************************************************
* 十六进制字符串转换二进制字节数组
* 输入：
*       data 十六进制字符串
*       size 十六进制字符串长度，2的倍数
*       outlen 转换后的二进制字符数组长度
* 输出：
*       二进制字符数组，需要free函数释放空间，失败返回NULL
*
************************************************************************/
inline unsigned char *hex2bin(const char *data, int size, int *outlen)
{
	int i = 0;
	int len = 0;
	char char1 = '\0';
	char char2 = '\0';
	unsigned char value = 0;
	unsigned char *out = NULL;

	if (size % 2 != 0) {
		return NULL;
	}

	len = size / 2;
	out = (unsigned char *)malloc(len * sizeof(char) + 1);
	if (out == NULL) {
		return NULL;
	}

	while (i < len) {
		char1 = *data;
		if (char1 >= '0' && char1 <= '9') {
			value = (char1 - '0') << 4;
		}
		else if (char1 >= 'a' && char1 <= 'f') {
			value = (char1 - 'a' + 10) << 4;
		}
		else if (char1 >= 'A' && char1 <= 'F') {
			value = (char1 - 'A' + 10) << 4;
		}
		else {
			free(out);
			return NULL;
		}
		data++;

		char2 = *data;
		if (char2 >= '0' && char2 <= '9') {
			value |= char2 - '0';
		}
		else if (char2 >= 'a' && char2 <= 'f') {
			value |= char2 - 'a' + 10;
		}
		else if (char2 >= 'A' && char2 <= 'F') {
			value |= char2 - 'A' + 10;
		}
		else {
			free(out);
			return NULL;
		}

		data++;
		*(out + i++) = value;
	}
	*(out + i) = '\0';

	if (outlen != NULL) {
		*outlen = i;
	}

	return out;
}


inline UINT8 AddState(UINT8 val1, UINT8 val2)
{
	return val1 | val2;
}

inline UINT16 AddState(UINT16 val1, UINT16 val2)
{
	return val1 | val2;
}

inline UINT32 AddState(UINT32 val1, UINT32 val2)
{
	return val1 | val2;
}

inline UINT8 DelState(UINT8 val1, UINT8 val2)
{
	return val1 & ~val2;
}

inline UINT16 DelState(UINT16 val1, UINT16 val2)
{
	return val1 & ~val2;
}

inline UINT32 DelState(UINT32 val1, UINT32 val2)
{
	return val1 & ~val2;
}

inline bool HasState(UINT8 val, UINT8 comp)
{
	UINT8 v = val & comp;
	if (v == comp)
	{
		return true;
	}
	return false;
}

inline bool HasState(UINT16 val, UINT16 comp)
{
	UINT16 v = val & comp;
	if (v == comp)
	{
		return true;
	}
	return false;
}

inline bool HasState(UINT32 val, UINT32 comp)
{
	UINT32 v = val & comp;
	if (v == comp)
	{
		return true;
	}
	return false;
}

inline UINT32 GetSystemSecond()
{
#ifdef WIN32
	return (UINT32)_time32(NULL);
#else
	return (UINT32)time(NULL);
#endif
}

template <size_t size>
inline void copystr(char(&dst)[size], const char* src)
{
	strcpy_s(dst, src);
}


typedef void(*DEALFILECONTENTFUNC)(const char* pFileName, const string& strContent);
inline bool SearchPathFile(const char* szPath, const char* pFilter, DEALFILECONTENTFUNC pFunc)
{
	char m_szInitDir[MAX_PATH];
	if (_fullpath(m_szInitDir, szPath, _MAX_PATH) == NULL)
		return false;

	if (_chdir(m_szInitDir) < 0)
		return false;

	long hFile;
	_finddata_t fileinfo;

	if ((hFile = _findfirst(pFilter, &fileinfo)) != -1)
	{
		do
		{
			if (!(fileinfo.attrib & _A_SUBDIR))
			{
				char szFileName[_MAX_PATH];
				strcpy(szFileName, m_szInitDir);
				strcat(szFileName, fileinfo.name);
				FILE* pFile = fopen(szFileName, "r");
				if (!pFile)
					continue;

				string strContent;
				fseek(pFile, 0, SEEK_SET);
				char temp[100];
				while (!feof(pFile))
				{
					int nCount = fread(temp, 1, 100, pFile);
					strContent.append(temp, nCount);
				}
				pFunc(fileinfo.name, strContent);
				fclose(pFile);
			}
			else
			{
				char szSubdir[_MAX_PATH];
				strcpy(szSubdir, m_szInitDir);
				strcat(szSubdir, fileinfo.name);
				strcat(szSubdir, "//");
				SearchPathFile(szSubdir, pFilter, pFunc);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
	}
	_findclose(hFile);
	return true;
}



#endif// __UTILITY_MEISHUIJING_2015_03_23__