#include <string.h>
#include <stdio.h>
#include "../common/CLog.h"
#include "../common/CrashDump.h"
#include "DrvCtrlApi.h"
#include "DeviceIoCtrl.h"

DeviceIoCtrl *g_pstDevIoCtrl = NULL;

BOOLEAN	OpenSSLProxy_EnvLibInit()
{
	(VOID)CLOG_evn_init(CLOG_TYPE_DEVCTRL);

	CrashDumpInitialize();

	if ( NULL == g_pstDevIoCtrl )
	{
		g_pstDevIoCtrl = new DeviceIoCtrl;
		if (NULL == g_pstDevIoCtrl )
		{
			CLOG_writelog_level("DEVCTL", CLOG_LEVEL_ERROR, "Create DeviceIoCtrl Class Object error!");
			return FALSE;
		}
		
		if( FALSE == g_pstDevIoCtrl->OpenDev() )
		{
			CLOG_writelog_level("DEVCTL", CLOG_LEVEL_ERROR, "Open Device Handle error!");
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}

VOID OpenSSLProxy_EnvLibUnInit()
{
	if (NULL != g_pstDevIoCtrl)
	{
		g_pstDevIoCtrl->CloseDev();

		delete g_pstDevIoCtrl;
		g_pstDevIoCtrl = NULL;
	}
}



BOOLEAN OpenSSLProxy_DrvCtrl_RuleMatchEnable()
{
	if (NULL != g_pstDevIoCtrl)
	{
		return g_pstDevIoCtrl->RuleMatchEnable();
	}

	return FALSE;
}

BOOLEAN OpenSSLProxy_DrvCtrl_RuleMatchDisable()
{
	if (NULL != g_pstDevIoCtrl)
	{
		return g_pstDevIoCtrl->RuleMatchDisable();
	}

	return FALSE;
}
























