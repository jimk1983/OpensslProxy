
/*通过链表的方式进行管理*/
/*在Worker和Handler的WSAEvent中，都需要使用*/
typedef struct tagSockInfo
{
	LIST_ENTRY		stNode;					/*节点*/
	SOCKET			sSockfd;					/*本Socket事件FD*/
	HANDLE			hEvtHandle;				/*事件句柄*/
	ULONG			ulEvtsIndex;				/*所在的Evts的数组索引*/
	LIST_ENTRY		stIoBufList;				/*包顺序链表: 包含了数据包和控制包，按序存放，保持连接本身的传输一致性*/
}SOCKINFO_S, *PSOCKINFO_S;

/*WSAEvent线程的socket管理器，Worker和Handler都会使用*/
/*每个线程都有该管理器*/
typedef struct tagSocketMgr
{
	UINT32			uiMgrIndex;											/*所在管理器数组索引, 找原来的线程时候需要*/
	SOCKET			sMsgFd;												/*通信的FD, 直接用UDP通信, 仅用于少量消息的线程间通信*/
	LIST_ENTRY		stSockList;											/*当前的socket信息链表*/
	ULONG			ulSockNums;										/*当前有没有超过64-4个就可以了, 一个用作通信了, 然后保持成对*/
	WSAEVENT		arrWSAEvts[WSAEVT_NUMS];				/*当前的socket事件数组*/
	SOCKET			arrSocketEvts[WSAEVT_NUMS];			/*事件对应的socket索引*/
}SOCK_MGR_S, *PSOCK_MGR_S;


