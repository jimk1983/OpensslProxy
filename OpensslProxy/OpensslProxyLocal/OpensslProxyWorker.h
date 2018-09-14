#pragma once

typedef struct tagWorkerContext
{
	PSOCK_MGR_S				pstArrySockHandler[MGR_ARRYNUMS];		/*工作线程最大数量*/
	UINT32							uiNums;															/*工作的线程个数*/
}WORKER_CTX_S, *PWORKER_CTX_S;




