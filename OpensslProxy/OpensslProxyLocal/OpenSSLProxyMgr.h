#pragma once

/*默认的本地监听端口*/
#define		MGR_LISTENPORT		9890



/*上下文管理器*/
typedef struct tagMgrContext
{
	PWORKER_CTX_S				pstWorkerCtx;				/*工作线程管理上下文*/
	PDISPATCHPACK_CTX_S		pstClientDispatchCtx;	/*客户端新建派发线程*/
	ULONG								ulProcessID;					/*当前进程ID*/
	USHORT								usListenPort;				/*当前端口*/
}MGR_CTX_S, *PMGR_CTX_S;


INT32	OpenSSLProxy_MgrInit();

VOID	OpenSSLProxy_MgrUnInit();

VOID	OpensslProxy_utils_GenUniqueID(CHAR *pcGuID);