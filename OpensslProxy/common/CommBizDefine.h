
/*SOCKET业务类型*/
typedef enum
{
	SOCKTYPE_MSG = 0,		/*线程间通信*/
	SOCKTYPE_LOCAL,			/*本地Socket*/
	SOCKTYPE_PROXY,			/*代理Socket*/

	SOCKTYPE_NUMS
}SOCKTYPE_E;


typedef struct tagSockInfo SOCKINFO_S, *PSOCKINFO_S;

typedef INT32(*PFSOCKCTRLCB)(SOCKINFO_S *pstSockInfo);

/*通过链表的方式进行管理*/
/*在Worker和Handler的WSAEvent中，都需要使用*/
struct tagSockInfo
{
	LIST_ENTRY						stNode;					/*节点*/
	SOCKTYPE_E					eSockType;				/*Socket业务类型*/
	SOCKET							sSockfd;					/*本Socket事件FD*/
	HANDLE							hEvtHandle;				/*事件句柄*/
	ULONG							ulEvtsIndex;				/*本身所在的Evts的数组索引*/
	ULONG							ulPeerEvtsIndex;		/*因为当前代理设计上规定是成对出现的，所以必然有一个对端的索引*/
	LIST_ENTRY						stIoBufList;				/*包顺序链表: 包含了数据包和控制包，按序存放，保持连接本身的传输一致性*/
	PFSOCKCTRLCB				pfSockCtrlCb;			/*本Socket的控制接口，错误关闭，或者是被动的调用*/
};

/*网络触发事件*/
typedef struct tagSockNetworkEvent
{
	WSAEVENT		arrWSAEvts[WSAEVT_NUMS];				/*当前的socket事件数组*/
	SOCKET			arrSocketEvts[WSAEVT_NUMS];			/*事件对应的socket索引*/
}SOCK_NEVET_S, *PSOCK_NEVET_S;

/*WSAEvent线程的socket管理器，Worker和Handler都会使用*/
/*每个线程都有该管理器*/
typedef struct tagSocketMgr
{
	UINT32				uiMgrCtxIndex;								/*所在管理器数组索引, 找原来的线程时候需要*/
	SOCKET				sMsgUdpPort;									/*简单通信的FD, 本地消息端口矩阵,  直接用UDP通信, 仅用于少量消息的线程间通信(大量时会不可靠)*/
	SOCKINFO_S		stArrySockInfo[WSAEVT_NUMS];	/*管理的Socket信息， 通过索引管理，这样就可以和网络事件复用相同索引，提高查找效率*/
	SOCK_NEVET_S	stNetEvent;										/*网络触发事件*/
	ULONG				ulSockNums;									/*当前有没有超过64-4个就可以了, 一个用作通信了, 然后保持成对*/
}SOCK_MGR_S, *PSOCK_MGR_S;


