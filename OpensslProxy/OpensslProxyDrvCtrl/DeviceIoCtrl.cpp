#include <string.h>
#include <stdio.h>
#include "../common/CLog.h"
#include "../common/CrashDump.h"
#include "DrvCtrlApi.h"
#include "DeviceIoCtrl.h"
#include "DevControlDefine.h"

DeviceIoCtrl::DeviceIoCtrl()
{
	m_hDev = INVALID_HANDLE_VALUE;
}

DeviceIoCtrl::~DeviceIoCtrl()
{

}

BOOLEAN DeviceIoCtrl::OpenDev()
{
	if ( m_hDev == INVALID_HANDLE_VALUE )
	{
		m_hDev = ::CreateFile(
				L"\\\\.\\OpenSSLProxyDriver", 
				GENERIC_READ | GENERIC_WRITE, 
				0, 
				NULL, 
				OPEN_EXISTING, 
				NULL, 
				NULL );
		if ( m_hDev == INVALID_HANDLE_VALUE )
		{
			CLOG_writelog_level("DEVCTL", CLOG_LEVEL_ERROR, "Create OpenSSLProxy Driver error=%08x!", GetLastError());
			return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}

void DeviceIoCtrl::CloseDev()
{
	if (m_hDev != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hDev);
		m_hDev = INVALID_HANDLE_VALUE;
	}
}

BOOLEAN DeviceIoCtrl::RuleMatchEnable()
{
	DWORD			dwRet = 0;

	if (m_hDev == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	if ( 0 == DeviceIoControl(m_hDev, DEVICE_IOCTL_MATCHENABLE, NULL, 0, NULL, NULL, &dwRet, NULL))
	{
		CLOG_writelog_level("DEVCTL", CLOG_LEVEL_ERROR, "Device  rule-match  enable error=%08x!", GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOLEAN DeviceIoCtrl::RuleMatchDisable()
{
	DWORD			dwRet = 0;

	if (m_hDev == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	if ( 0 == DeviceIoControl(m_hDev, DEVICE_IOCTL_MATCHDISABLE, NULL, 0, NULL, NULL, &dwRet, NULL) )
	{
		CLOG_writelog_level("DEVCTL", CLOG_LEVEL_ERROR, "Device  rule-match disable error=%08x!", GetLastError());
		return FALSE;
	}

	return TRUE;
}