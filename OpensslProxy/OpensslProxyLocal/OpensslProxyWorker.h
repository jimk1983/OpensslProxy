#pragma once

typedef struct tagWorkerContext
{
	PSOCK_MGR_S				pstArryWorker[MGR_ARRYNUMS];		/*工作线程最大数量*/
	UINT32							uiWorkerNums;									/*工作的线程个数*/
	CRITICAL_SECTION			stWorkerLock;										/*统一锁*/
}WORKER_CTX_S, *PWORKER_CTX_S;




