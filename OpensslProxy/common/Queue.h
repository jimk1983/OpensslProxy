#ifndef _QUEUE_H_
#define _QUEUE_H_

/*经验值直接设定队列最大个数为2000个左右，每个8K，运行时也就消耗16M内存左右大小，*/
#define		SYS_QUE_NUMS				2000

/*单次接收到的最大的数据内容大小，为16K, 因为不能本地接收很快，外网转发很慢，
如果出现上传文件，则可能会出现队列耗光的情况*/
#define		SYS_QUE_BUF_LEN			16384

#define		SYS_QUE_TYPE_DATA		1		/*队列中为数据流*/
#define		SYS_QUE_TYPE_CTRL		2		/*队列中为控制流*/

typedef struct tagQueNodeHead	NODE_HEAD_S,*PNODE_HEAD_S;

struct tagQueNodeHead
{
	struct tagQueNodeHead *next;
	struct tagQueNodeHead *prev;
};

/*自定义修改结构*/
typedef struct tagUserDataInfo
{
	UINT32		uiType;					/*包内容类型，控制还是数据流*/
	UINT32		uiLength;				/*获取的包数据长度*/
	CHAR			acBuf[SYS_QUE_BUF_LEN];	/*数据buf长度，队列初始化中已经固定申请*/
}USER_DATA_INFO_S, *PUSER_DATA_INFO_S;

typedef struct tagQueueEntryNode
{
	NODE_HEAD_S				stHead;			/*链表头*/
	USER_DATA_INFO_S		stDataInfo;		/*数据结构*/
}QUE_ENTRY_S, *PQUE_ENTRY_S;

/*简单的队列池管理*/
typedef struct tagSysQueue
{
	NODE_HEAD_S					stFreeList;					/*空闲队列，用于获取空闲的节点*/
	UINT32								uiFreeNums;				/*空闲队列中的个数，用于检查异常*/
	NODE_HEAD_S					stUsedList;					/*已经使用的节点，回收用，否则会内存泄漏，使用完还要还回去*/
	UINT32								uiUsedNums;				/*队列计数*/
	CRITICAL_SECTION				stCriticalSection;			/*统一锁*/

	SYS_SEM_T							hSemNotify;					/*信号量通知*/
}SYS_QUE_S;


#define SYS_QUE_PROTECT(pQueue)			EnterCriticalSection(&pQueue->stCriticalSection)
#define SYS_QUE_UNPROTECT(pQueue)		LeaveCriticalSection(&pQueue->stCriticalSection)

QUE_ENTRY_S* SYS_QueNode_Create();

VOID SYS_QueNode_Free(QUE_ENTRY_S* pstQueNode);

QUE_ENTRY_S* SYS_Queue_FreePop(SYS_QUE_S* pstQueue);

INT32 SYS_Queue_FreePush(SYS_QUE_S* pstQueue, QUE_ENTRY_S*pstNode);

QUE_ENTRY_S* SYS_Queue_UsedPop(SYS_QUE_S* pstQueue);

INT32 SYS_Queue_UsedPush(SYS_QUE_S* pstQueue, QUE_ENTRY_S*pstNode);

SYS_QUE_S* SYS_Queue_Create(ULONG ulQueNums);

VOID  SYS_Queue_Free(SYS_QUE_S*pstQueue);

INT32 SYS_Queue_UsedListWaitfor(SYS_QUE_S *pstQueue);

#endif