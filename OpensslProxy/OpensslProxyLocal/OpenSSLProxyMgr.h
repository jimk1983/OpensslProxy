#pragma once


/*上下文管理器*/
typedef struct tagMgrContext
{
	PWORKER_CTX_S				pstWorkerCtx;				/*工作线程管理上下文*/
	PDISPATCHPACK_CTX_S		pstDispatchPackCtx;	/*包分派线程*/
	PHANDLER_CTX_S				pstHandlerCtx;				/*代理线程管理上下文*/
}MGR_CTX_S, *PMGR_CTX_S;


INT32 OpenSSLProxy_MgrInit();

VOID OpenSSLProxy_MgrUnInit();

