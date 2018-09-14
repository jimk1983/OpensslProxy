// OpensslProxyLocal.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Winsock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <mswsock.h>
#include <stdio.h>
#include <process.h>
#include "../common/CLog.h"
#include "../OpensslProxyDrvCtrl/DrvCtrlApi.h"
#include "../common/CommDef.h"
#include "../common/CommBizDefine.h"
#include "OpensslProxyHandler.h"
#include "OpensslProxyWorker.h"
#include "OpensslProxyPacketDispatch.h"
#include "OpenSSLProxyMgr.h"


#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "OpensslProxyDrvCtrl.lib")

int main()
{
	if ( FALSE == OpenSSLProxy_DrvCtrl_EnvLibInit())
	{
		CLOG_writelog_level("LPXY", CLOG_LEVEL_ERROR, "Driver enviroment error!\n");
		goto Exit;
	}
	else
	{
		CLOG_writelog_level("LPXY", CLOG_LEVEL_EVENT, "***INIT***: OpenSSLProxy Driver enviroment Init OK!");
	}

	if ( SYS_ERR == OpenSSLProxy_MgrInit() )
	{
		CLOG_writelog_level("LPXY", CLOG_LEVEL_ERROR, "Driver enviroment error!\n");
		goto Exit;
	}
	else
	{
		CLOG_writelog_level("LPXY", CLOG_LEVEL_EVENT, "***INIT***: Local Proxy Manager Init  OK!");
	}



	system("pause");
Exit:
	OpenSSLProxy_DrvCtrl_EnvLibUnInit();
	CLOG_writelog_level("LPXY", CLOG_LEVEL_EVENT, "***STOP***: The End!");
    return 0;
}

