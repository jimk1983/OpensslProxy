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
#include "../common/Sem.h"
#include "../common/Queue.h"
#include "OpensslProxyWorker.h"
#include "OpensslProxyPacketDispatch.h"
#include "OpenSSLProxyMgr.h"


PMGR_CTX_S g_pstMgrCtx = NULL;


VOID OpenSSLProxy_SockInit()
{
	WSADATA				 wsaData;

	// 请求2.2版本的WinSock库
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		CLOG_writelog_level("LPXY", CLOG_LEVEL_ERROR, "WSAStartup failed with error code: %d\n", GetLastError());
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


INT32 OpenSSLProxy_MgrInit()
{
	OpenSSLProxy_SockInit();

	/*管理器上下文*/
	g_pstMgrCtx = (PMGR_CTX_S)malloc(sizeof(MGR_CTX_S));
	if (NULL == g_pstMgrCtx)
	{
		CLOG_writelog_level("LPXY", CLOG_LEVEL_ERROR, "Malloc context error=%08x!\n", GetLastError());
		return SYS_ERR;
	}

	ZeroMemory(g_pstMgrCtx, sizeof(MGR_CTX_S));

	g_pstMgrCtx->ulProcessID = GetCurrentProcessId();
	g_pstMgrCtx->usListenPort = MGR_LISTENPORT;
	
	if (SYS_ERR == OpenSSLProxy_DrvCtrl_SetLocalProxyInfo((UINT32)g_pstMgrCtx->ulProcessID, (UINT32)g_pstMgrCtx->usListenPort))
	{
		CLOG_writelog_level("LPXY", CLOG_LEVEL_ERROR, "Driver Set Local Proxy info Error=%08x!\n", GetLastError());
		return SYS_ERR;
	}

	return SYS_OK;
}


VOID OpenSSLProxy_MgrUnInit()
{
	if (NULL != g_pstMgrCtx)
	{

		free(g_pstMgrCtx);
		g_pstMgrCtx = NULL;
	}
}





















