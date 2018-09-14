#include <Winsock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <mswsock.h>
#include <stdio.h>
#include <process.h>
#include "../common/CLog.h"
#include "../common/CommDef.h"
#include "../common/CommBizDefine.h"
#include "OpensslProxyHandler.h"
#include "OpensslProxyWorker.h"
#include "OpensslProxyPacketDispatch.h"
#include "OpenSSLProxyMgr.h"





VOID OpenSSLProxy_SockInit()
{
	WSADATA				 wsaData;

	// 请求2.2版本的WinSock库
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		CLOG_writelog_level("LPXY", CLOG_LEVEL_ERROR,"WSAStartup failed with error code: %d\n", GetLastError());
		return;
	}

	// 检查套接字库是否申请成功
	if (2 != HIBYTE(wsaData.wVersion) || 2 != LOBYTE(wsaData.wVersion))
	{
		CLOG_writelog_level("LPXY", CLOG_LEVEL_ERROR, "WSAStartup Socket version not supported.\n");
		WSACleanup();
		return;
	}
}


INT32 OpenSSLProxy_ServerInit()
{
	OpenSSLProxy_SockInit();




	return SYS_OK;
}


INT32 OpenSSLProxy_ServerUnInit()
{

	return SYS_OK;
}


