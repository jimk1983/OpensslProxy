#pragma once

typedef struct tagHandlerContext
{
	PSOCK_MGR_S			pstArrySockHandler[MGR_ARRYNUMS];		/*代理线程最大数量*/
	UINT32						uiNums;															/*当前代理线程个数*/


}HANDLER_CTX_S, *PHANDLER_CTX_S;











