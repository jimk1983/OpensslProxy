#pragma once


/*新来的本地的socket信息*/
typedef struct tagLocalSockInfo
{
	SOCKET			sLocalFD;					/*本地的Socket信息*/
	SOCKADDR		stLocalInfo;				/*本地的Socket信息*/
}CLIENT_INFO_S, *PCLIENT_INFO_S;

/*分派转发包处理: TODO: 可以做一些过滤的操作*/
typedef struct tagDispatchPackContext
{
	CLIENT_INFO_S	stClientInfo;			/*新的客户端连接信息*/
	ULONG				ulBlanceAlgm;		/*分发的均衡算法*/
}DISPATCHPACK_CTX_S, *PDISPATCHPACK_CTX_S;





