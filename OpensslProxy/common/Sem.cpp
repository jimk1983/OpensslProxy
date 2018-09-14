#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include "CommDef.h"
#include "sem.h"


SYS_SEM_T SYS_Sem_Create(UINT8 uiCount)
{
	return CreateSemaphore(NULL, uiCount, MAXLONG, NULL);
}

VOID SYS_Sem_Free(SYS_SEM_T hSem)
{
	CloseHandle(hSem);
}

/*信号量增加1，会触发后面的WaitFor动作*/
VOID SYS_Sem_Signal(SYS_SEM_T hSem)
{
	if (!ReleaseSemaphore(hSem, 1, NULL))
	{
		//TODO: add log?
		return;
	}
}

UINT32 SYS_Sem_Wait(SYS_SEM_T hSem, UINT32 uiTimeout)
{
	DWORD dwWaitResult = WaitForSingleObject(hSem, (uiTimeout != 0) ? uiTimeout : INFINITE);
	switch (dwWaitResult)
	{
		case WAIT_OBJECT_0:
			//printf("sem wait ok!\n");
			return SYS_OK;
		case WAIT_ABANDONED:
			//printf("sem wait abandoned!\n");
		case WAIT_TIMEOUT:
			//printf("sem wait timeout!\n");
		default:
			//printf("sem wait error!\n");
			return SYS_ERR;
	}
}














