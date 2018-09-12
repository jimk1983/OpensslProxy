#include <windows.h>
#include <winsvc.h>
#include <fcntl.h>
#include <WinBase.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ShlObj.h>
#include "clog.h"


#define CLOG_WRITELOG_BUFLEN	16384
#define CLOG_WRITEBUF_LEN		8192

#define CLOG_WRITELOG_OFF	0	/*日志关闭宏*/

typedef HANDLE CLOG_MUTEX_HANDLE;

typedef struct tagClogMutex
{
	const char				*pcMutexName;	/*文件锁名称*/
}CLOG_MUTEX_S;


/*结构放在内部，简化外部接口*/
typedef struct tagCLogFileInfo
{
	CLOG_TYPE_E					uiLogType;		/*上面的类型*/
	CLOG_MUTEX_S				stMutex;		/*文件锁结构*/
	const char					*pcFileName;	/*文件名称*/
}CLOG_FILE_INFO_S;


const CLOG_FILE_INFO_S stLogInfo[]=
{
	{
		CLOG_TYPE_DEVCTRL,
		{
			"#%log_device_ctrl_mutex"
		},
		"log_device_ctrl.txt"
	},

};

static char g_acCurLogFile[MAX_PATH] = {0};

CLOG_MUTEX_HANDLE g_pMutexhandle = NULL;

LONG		g_lLogLevel = CLOG_LEVEL_EVENT;

VOID clog_snprintf_s(char *pcbuf, UINT32 uilen, const CHAR *pcformat, ...)
{
	va_list args_ptr = { 0 };

	va_start(args_ptr, pcformat);
	(VOID)vsprintf_s(pcbuf, uilen - 1, pcformat, args_ptr);
	va_end(args_ptr);

	return;
}

int CLOG_atoi(char *pcint)
{
	return atoi(pcint);
}

void CLOG_itoa(int iNum, char *pcNum)
{
	_itoa_s(iNum, pcNum, MAX_PATH, 10);

	return;
}

int CLOG_AsciitoUnicode(char *pcStr, wchar_t *plwstr)
{
	wchar_t pwStr[MAX_PATH * sizeof(wchar_t)] = { 0 };;
	int wcharlen = 0;

	wcharlen = MultiByteToWideChar(CP_ACP, 0, pcStr, -1, NULL, 0);

	MultiByteToWideChar(CP_ACP, 0, pcStr, -1, (LPWSTR)plwstr, wcharlen);

	//memcpy_s(plwstr, wcharlen, pwStr, MAX_PATH * sizeof(wchar_t));

	return 0;
}

int CLOG_UnicodeToAscii(wchar_t *plwstr, char *pcStr)
{
	char acStr[MAX_PATH] = { 0 };
	int ilen = 0;

	ilen = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)plwstr, -1, NULL, 0, NULL, NULL);

	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)plwstr, -1, acStr, ilen, NULL, NULL);

	memcpy_s(pcStr, ilen, acStr, MAX_PATH);

	return 0;
}

VOID CLOG_GetTempPath(char *temppath)
{
	char acDir[MAX_PATH] = { 0 };
	int  ilen = 0;

	if (NULL == temppath)
	{
		return;
	}

	GetTempPathA(MAX_PATH, acDir);
	ilen = (int)strlen(acDir);
	memcpy_s(temppath, MAX_PATH - 1, acDir, MAX_PATH - 1);
	temppath[ilen] = '\0';
	return;
}

void CLOG_GetCurrentAppdata(char *appPath)
{
	char acAppDir[MAX_PATH] = { 0 };
	int  ilen = 0;

	if (NULL == appPath)
	{
		return;
	}

	/*系统服务，%appdata%不是当前用户路径,所以直接放置在C:\ProgramData的目录下了
	这样所有的用户都可以获取到*/
	SHGetSpecialFolderPathA(NULL, acAppDir, CSIDL_COMMON_APPDATA, 0);
	ilen = (int)strlen(acAppDir);
	strcpy_s(appPath, MAX_PATH - 1, acAppDir);
	appPath[ilen] = '\0';
	return;
}

void CLOG_GetCurrentProcessDirectory(char *pcPath)
{
	char *pcStr = NULL;
	if (NULL == pcPath)
	{
		return;
	}

	GetModuleFileNameA(NULL, pcPath, MAX_PATH);
	pcStr = strrchr(pcPath, ('\\'));
	if (NULL != pcStr)
	{
		pcStr[1] = 0;
	}

	return;
}

BOOL CLOG_DirIsExist(char *pcDir)
{
	WIN32_FIND_DATAA wfd;
	HANDLE hFile = FindFirstFileA(pcDir, &wfd);

	if ((hFile != INVALID_HANDLE_VALUE)
		&& (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		FindClose(hFile);
		return TRUE;
	}
	FindClose(hFile);

	return FALSE;
}

int CLOG_DirCreate(char *pcDir)
{
	if (NULL == pcDir)
	{
		return  -1;
	}

	SHCreateDirectoryExA(NULL, pcDir, NULL);

	return 0;
}

BOOL CLOG_FileIsExist(char *pcFile)
{
	WIN32_FIND_DATAA wfd;
	HANDLE hFile = FindFirstFileA(pcFile, &wfd);

	if ((hFile != INVALID_HANDLE_VALUE)
		&& !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		FindClose(hFile);
		return TRUE;
	}

	FindClose(hFile);
	return FALSE;
}

int CLOG_FileIsCreate(char *pcFilePath)
{
	HANDLE hFile = NULL;
	DWORD dwFileSize = 0;

	hFile = CreateFileA(pcFilePath,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		0,
		0);
	if (NULL == hFile)
	{
		return SYS_ERR;
	}

	if (0 != GetFileSize(hFile, &dwFileSize))
	{
		CloseHandle(hFile);
		return SYS_ERR;
	}

	CloseHandle(hFile);

	return SYS_OK;
}


void CLOG_WriteLogNoMutex(const char *pcformat, ...)
{
	va_list args_ptr = { 0 };
	CHAR acbuf[CLOG_BUFLEN] = { 0 };
	FILE *pfile = NULL;

	va_start(args_ptr, pcformat);
	(VOID)vsprintf_s(acbuf, CLOG_BUFLEN - 1, pcformat, args_ptr);
	va_end(args_ptr);

	fopen_s(&pfile, g_acCurLogFile, "a+");
	fprintf(pfile, "%s\n", acbuf);
	fclose(pfile);

	return;
}
int CLOG_CheckAndCreateLogPath(CLOG_TYPE_E eClogType)
{
	char acProcessPath[MAX_PATH] = { 0 };
	char acProcessFile[MAX_PATH] = { 0 };

#if 1
	CLOG_GetCurrentProcessDirectory(acProcessPath);

	//strcat_s(acProcessPath, MAX_PATH, "\\");

	/*后面使用，越界保护*/
	if (eClogType >= CLOG_TYPE_NUMS)
	{
		return SYS_ERR;
	}
#else
	CLOG_GetCurrentAppdata(acProcessPath);
#endif
	strcat_s(acProcessPath, MAX_PATH, "\\");
	strcat_s(acProcessPath, MAX_PATH, CLOG_DIRNAME);
	
	if (FALSE == CLOG_DirIsExist(acProcessPath))
	{
		if (-1 == CLOG_DirCreate(acProcessPath))
		{
			return SYS_ERR;
		}
	}

	strcpy_s(acProcessFile, acProcessPath);
	strcat_s(acProcessFile, MAX_PATH, "\\");
	strcat_s(acProcessFile, MAX_PATH, stLogInfo[eClogType].pcFileName);

	if (FALSE == CLOG_FileIsExist(acProcessFile))
	{
		if (-1 == CLOG_FileIsCreate(acProcessFile))
		{
			return SYS_ERR;
		}
	}
	strcpy_s(g_acCurLogFile, acProcessFile);

	if (NULL == g_pMutexhandle)
	{
		g_pMutexhandle = CreateMutexA(NULL, FALSE, stLogInfo[eClogType].stMutex.pcMutexName);
		if (NULL == g_pMutexhandle)
		{
			CLOG_WriteLogNoMutex("create mutex error!");
			return -1;
		}
	}

	return 0;
}


void CLOG_FileLock()
{
	WaitForSingleObject(g_pMutexhandle, INFINITE);
}

void CLOG_FileUnLock()
{
	ReleaseMutex(g_pMutexhandle);
}

int CLOG_evn_init(CLOG_TYPE_E eType)
{

#if CLOG_WRITELOG_OFF
	return 0;
#endif 

	if (-1 == CLOG_CheckAndCreateLogPath(eType))
	{
		return -1;
	}

	return 0;
}

void CLOG_evn_uninit()
{
	if (NULL != g_pMutexhandle)
	{
		CloseHandle(g_pMutexhandle);
		g_pMutexhandle = NULL;
	}
	return;
}

void CLOG_writelog(char *pcModuleName, const char *fmt, ...)
{
	struct tm cur_time;
	char buffer[CLOG_WRITELOG_BUFLEN] = { 0 };
	time_t time_seconds;
	va_list argptr;                                               //你的类型链表 
	int i = 0;
	FILE* pfile;
	INT32 iyear = 0;
	INT32 imoth = 0;
	INT32 iday = 0;
	INT32 ihour = 0;
	INT32 imute = 0;
	INT32 isecs = 0;

#if CLOG_WRITELOG_OFF
	return;
#endif 


	if (g_acCurLogFile[0] == '\0')
	{
		return;
	}

	/*检查文件是否存在了*/
	if (FALSE == CLOG_FileIsExist(g_acCurLogFile))
	{
		/*被删除了，就再重新创建一个*/
		if (-1 == CLOG_FileIsCreate(g_acCurLogFile))
		{
			return;
		}
	}

	/*获取系统时间*/
	time(&time_seconds);
	localtime_s(&cur_time, &time_seconds);                        

	memset(buffer, 0, CLOG_WRITELOG_BUFLEN);
	iyear = cur_time.tm_year + 1900;
	imoth = cur_time.tm_mon + 1;
	iday = cur_time.tm_mday;
	ihour = cur_time.tm_hour;
	imute = cur_time.tm_min;
	isecs = cur_time.tm_sec;

	sprintf_s(buffer, CLOG_WRITEBUF_LEN, "[%04d-%02d-%02d %02d:%02d:%02d] ",
		cur_time.tm_year + 1900,
		cur_time.tm_mon + 1,
		cur_time.tm_mday,
		cur_time.tm_hour,
		cur_time.tm_min,
		cur_time.tm_sec);
	i += (int)strlen(buffer);

	/*拼接模块名称不能超过8字节*/
	i += sprintf_s(buffer + i, CLOG_WRITELOG_BUFLEN - i, "[M:%8s]", pcModuleName);

	va_start(argptr, fmt);                                                        //初始化你的fmt链表
	vsprintf_s(buffer + i, CLOG_WRITELOG_BUFLEN-i-1, fmt, argptr);              //可选参数
	//vsnprintf_s(buffer + i, CLOG_WRITELOG_BUFLEN - i - 1, _TRUNCATE,  fmt, argptr);
	va_end(argptr);


	CLOG_FileLock();
	fopen_s(&pfile, g_acCurLogFile, "a+");
	if (NULL == pfile)
	{
		CLOG_FileUnLock();
		return;
	}

	fprintf(pfile, "%s", buffer);
	fprintf(pfile, "\n");
	fclose(pfile);

	CLOG_FileUnLock();
	return;
}



void CLOG_writelog_level(char *pcModuleName,int level, const char *fmt, ...)
{
	struct tm cur_time;
	char buffer[16384] = { 0 };
	time_t time_seconds;
	va_list argptr;                                               //你的类型链表 
	int i = 0;
	FILE* pfile;

#if CLOG_WRITELOG_OFF
	return;
#endif 

	if ( level > g_lLogLevel )
	{
		return;
	}

	if (g_acCurLogFile[0] == '\0')
	{
		return;
	}

	/*检查文件是否存在了*/
	if (FALSE == CLOG_FileIsExist(g_acCurLogFile))
	{
		/*被删除了，就再重新创建一个*/
		if (-1 == CLOG_FileIsCreate(g_acCurLogFile))
		{
			return;
		}
	}

	/*获取系统时间*/
	time(&time_seconds);
	localtime_s(&cur_time, &time_seconds);

	memset(buffer, 0, CLOG_WRITEBUF_LEN);
	sprintf_s(buffer, CLOG_WRITEBUF_LEN, "\r\n[%04d-%02d-%02d %02d:%02d:%02d] ",
		cur_time.tm_year + 1900,
		cur_time.tm_mon + 1,
		cur_time.tm_mday,
		cur_time.tm_hour,
		cur_time.tm_min,
		cur_time.tm_sec);
	i += (int)strlen(buffer);

	/*拼接模块名称不能超过8字节*/
	switch (level)
	{
	case CLOG_LEVEL_ERROR:
		i += sprintf_s(buffer + i, CLOG_WRITELOG_BUFLEN - i - CLOG_LEVEL_STR_LEN, "[M:%8s]<%8s-->", pcModuleName, CLOG_LEVEL_STR_ERROR);
		break;
	case CLOG_LEVEL_WARNING:
		i += sprintf_s(buffer + i, CLOG_WRITELOG_BUFLEN - i - CLOG_LEVEL_STR_LEN, "[M:%8s]<%8s-->", pcModuleName, CLOG_LEVEL_STR_WARING);
		break;
	default:
		i += sprintf_s(buffer + i, CLOG_WRITELOG_BUFLEN - i - CLOG_LEVEL_STR_LEN, "[M:%8s]<        -->", pcModuleName);
		break;
	}
	
    bool bNeedWriteLog = false;
	va_start(argptr, fmt);                                                        //初始化你的fmt链表
    int nlen = _vscprintf_l(fmt, 0, argptr) + 1; // 计算需要的长度, _vscprintf doesn't count, terminating '\0' 
    if (nlen < (CLOG_WRITEBUF_LEN - 128))        // 这里前面写入日期时间等已经处理相关数据，需要把长度需要减一下
    {
        vsprintf_s(buffer + i, CLOG_WRITEBUF_LEN, fmt, argptr);                                              //可选参数
        bNeedWriteLog = true;
    }
    else
    {
        // 文本过程，目前日志模块无法打印
    }
	va_end(argptr);

    if (bNeedWriteLog)
    {
        // Write file
        CLOG_FileLock();
        fopen_s(&pfile, g_acCurLogFile, "a+");
        if (NULL == pfile)
        {
            CLOG_FileUnLock();
            return;
        }

        fprintf(pfile, "%s", buffer);
        //fprintf(pfile, "\n");
        fclose(pfile);

        CLOG_FileUnLock();
    }
	return;
}