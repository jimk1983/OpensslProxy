#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include "CommDef.h"
#include "sem.h"
#include "queue.h"

QUE_ENTRY_S* SYS_QueNode_Create()
{
	QUE_ENTRY_S*	pstQueNode = NULL;

	pstQueNode = (PQUE_ENTRY_S)malloc(sizeof(QUE_ENTRY_S));
	if (NULL == pstQueNode)
	{
		return NULL;
	}
	memset(pstQueNode, 0, sizeof(QUE_ENTRY_S));

	pstQueNode->stHead.next = &pstQueNode->stHead;
	pstQueNode->stHead.prev = &pstQueNode->stHead;

	return pstQueNode;
}

VOID SYS_QueNode_Free(QUE_ENTRY_S* pstQueNode)
{
	if (NULL != pstQueNode)
	{
		free(pstQueNode);
	}
}

INT32 SYS_Queue_UsedPush(SYS_QUE_S* pstQueue, QUE_ENTRY_S*pstNode)
{
	if (NULL == pstQueue
		|| NULL == pstNode)
	{
		return SYS_ERR;
	}

	/*加入尾部*/
	SYS_QUE_PROTECT(pstQueue);
	pstNode->stHead.next = &pstQueue->stUsedList;
	pstNode->stHead.prev = pstQueue->stUsedList.prev;
	pstQueue->stUsedList.prev->next = &pstNode->stHead;
	pstQueue->stUsedList.prev = &pstNode->stHead;
	pstQueue->uiUsedNums++;
	SYS_QUE_UNPROTECT(pstQueue);
	
	/*Used队列有数据推送时，直接就通知过去*/
	SYS_Sem_Signal(pstQueue->hSemNotify);

	return SYS_OK;
}

QUE_ENTRY_S* SYS_Queue_UsedPop(SYS_QUE_S* pstQueue)
{
	PQUE_ENTRY_S		pstQueNode  = NULL;
	PNODE_HEAD_S		pstNode			= NULL;

	if (NULL == pstQueue)
	{
		return NULL;
	}

	/*从头部取出*/
	SYS_QUE_PROTECT(pstQueue);
	if (0 >= pstQueue->uiUsedNums)
	{
		SYS_QUE_UNPROTECT(pstQueue);
		return NULL;
	}

	pstNode =  pstQueue->stUsedList.next;
	if (NULL != pstNode)
	{
		pstNode->next->prev = pstNode->prev;
		pstNode->prev->next = pstNode->next;
		pstNode->next = pstNode;
		pstNode->prev = pstNode;
	}
	pstQueue->uiUsedNums--;
	SYS_QUE_UNPROTECT(pstQueue);

	pstQueNode = (QUE_ENTRY_S*)pstNode;

	return pstQueNode;
}

INT32 SYS_Queue_FreePush(SYS_QUE_S* pstQueue, QUE_ENTRY_S*pstNode)
{
	if (NULL == pstQueue
		|| NULL == pstNode)
	{
		return SYS_ERR;
	}

	SYS_QUE_PROTECT(pstQueue);
	pstNode->stHead.next = &pstQueue->stFreeList;
	pstNode->stHead.prev = pstQueue->stFreeList.prev;
	pstQueue->stFreeList.prev->next = &pstNode->stHead;
	pstQueue->stFreeList.prev = &pstNode->stHead;
	pstQueue->uiFreeNums++;
	SYS_QUE_UNPROTECT(pstQueue);

	return SYS_OK;
}

QUE_ENTRY_S* SYS_Queue_FreePop(SYS_QUE_S* pstQueue)
{
	QUE_ENTRY_S			*pstQueNode = NULL;
	NODE_HEAD_S		*pstNode = NULL;

	if (NULL == pstQueue)
	{
		return NULL;
	}

	SYS_QUE_PROTECT(pstQueue);
	if (0 >= pstQueue->uiFreeNums)
	{
		SYS_QUE_UNPROTECT(pstQueue);
		return NULL;
	}

	pstNode = pstQueue->stFreeList.next;
	if (NULL != pstNode)
	{
		pstNode->next->prev = pstNode->prev;
		pstNode->prev->next = pstNode->next;
		pstNode->next = pstNode;
		pstNode->prev = pstNode;
	}
	pstQueue->uiFreeNums--;
	SYS_QUE_UNPROTECT(pstQueue);

	pstQueNode = (QUE_ENTRY_S*)pstNode;

	memset(&pstQueNode->stDataInfo, 0 ,sizeof(USER_DATA_INFO_S));

	return pstQueNode;
}

INT32 SYS_Queue_UsedListWaitfor(SYS_QUE_S*	pstQueue)
{
	INT32 iRet = SYS_ERR;

	if (NULL == pstQueue)
	{
		return SYS_ERR;
	}

	iRet = SYS_Sem_Wait(pstQueue->hSemNotify, 0);
	if (SYS_ERR == iRet)
	{
		return SYS_ERR;
	}

	return SYS_OK;
}


SYS_QUE_S* SYS_Queue_Create(ULONG ulQueNums)
{
	SYS_QUE_S*			pstQueue = NULL;
	QUE_ENTRY_S*	pstNode = NULL;
	UINT32				uiCount = 0;

	pstQueue = (SYS_QUE_S*)malloc(sizeof(SYS_QUE_S));
	if (NULL == pstQueue)
	{
		return NULL;
	}
	memset(pstQueue, 0, sizeof(SYS_QUE_S));

	pstQueue->hSemNotify = SYS_Sem_Create(1);
	if (INVALID_HANDLE_VALUE == pstQueue->hSemNotify)
	{
		free(pstQueue);
		return NULL;
	}

	pstQueue->stFreeList.next = &pstQueue->stFreeList;
	pstQueue->stFreeList.prev = &pstQueue->stFreeList;
	pstQueue->uiFreeNums = 0;

	/*要保证收发包顺序，设计为双向链表*/
	pstQueue->stUsedList.next = &pstQueue->stUsedList;
	pstQueue->stUsedList.prev = &pstQueue->stUsedList;
	pstQueue->uiUsedNums = 0;

	InitializeCriticalSection(&pstQueue->stCriticalSection);

	for (uiCount = 0; uiCount < ulQueNums; uiCount++)
	{
		pstNode = SYS_QueNode_Create();
		if (NULL == pstNode)
		{
			SYS_Queue_Free(pstQueue);
			break;
		}

		if (SYS_ERR == SYS_Queue_FreePush(pstQueue, pstNode))
		{
			free(pstNode);
			SYS_Queue_Free(pstQueue);
			pstQueue = NULL;
			break;
		}
	}

	return pstQueue;
}

VOID SYS_Queue_Free(SYS_QUE_S* pstQueue)
{
	QUE_ENTRY_S*	pstQueNode	= NULL;
	UINT32				uiNums	= 0;
	UINT32				uiCount	= 0;

	if (NULL != pstQueue)
	{
		SYS_Sem_Free(pstQueue->hSemNotify);

		SYS_QUE_PROTECT(pstQueue);
		uiNums = pstQueue->uiUsedNums;
		for (uiCount = 0; uiCount < uiNums; uiCount++)
		{
			pstQueNode = SYS_Queue_UsedPop(pstQueue);
			if (NULL == pstQueNode)
			{
				break;
			}
			SYS_QueNode_Free(pstQueNode);
			pstQueNode = NULL;
		}

		uiNums = pstQueue->uiFreeNums;
		for (uiCount = 0; uiCount < uiNums; uiCount++)
		{
			pstQueNode = SYS_Queue_FreePop(pstQueue);
			if (NULL == pstQueNode)
			{
				break;
			}
			SYS_QueNode_Free(pstQueNode);
			pstQueNode = NULL;
		}
		SYS_QUE_UNPROTECT(pstQueue);

		free(pstQueue);
	}
}

