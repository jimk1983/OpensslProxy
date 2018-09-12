

/*定义三种类型的规则List*/
typedef enum
{
	OPENSSLPROXY_LISTTYPE_IPPORT = 0,
	OPENSSLPROXY_LISTTYPE_PORT,
	OPENSSLPROXY_LISTTYPE_IPADDR,

	OPENSSLPROXY_LISTTYPE_NUMS,
}RULE_LISTTYPE_E;


/*规则端口白名单*/
typedef struct tagRuleWritePortRange
{
	UINT32			uiLocalPortStart;
	UINT32			uiLocalPortEnd;
}RULE_PORTRANGE_S, *PRULE_PORTRANGE_S;

typedef struct tagRuleInfoEntry
{
	LIST_ENTRY		listEntry;
	UINT32			uiRuleType;
	UINT32			uiRuleIP;
	UINT32			uiRulePort;
}RULE_INFO_ENTRY, *PRULE_INFO_ENTRY;

/*规则管理上下文*/
typedef struct tagRuleMgrContext
{
	RESOURCE_LOCK_S					stResLock;				/*资源锁*/
	LIST_ENTRY								stRulePortList;			/*0.0.0.0:8080, 任意地址的端口匹配*/
	LIST_ENTRY								stRuleIPaddrList;		/*10.10.10.1:0, 任意端口的地址匹配*/
	LIST_ENTRY								stRuleIPPortList;		/*完全匹配*/
	UINT32									uiRuleNums;			/*所有规则的数量*/
	RULE_PORTRANGE_S				stSrcPortRange;		/*源端口白名单*/
}RULE_MGR_CTX_S, *PRULE_MGR_CTX_S;


NTSTATUS	OpenSSLProxy_RuleInit();

VOID			OpenSSLProxy_RuleUnInit();

NTSTATUS	OpenSSLProxy_RuleEntryAdd(IN UINT32 uiRuleIP, IN USHORT usRulePort);

VOID			OpenSSLProxy_RuleEntryRemove(UINT32 uiRuleIP, IN USHORT usRulePort);

VOID			OpenSSLProxy_RuleTypeClear(UINT32 uiRuleType);

VOID			OpenSSLProxy_RuleAllClear();

NTSTATUS	OpenSSLProxy_SetSrcPortRange(UINT32 uiPortStart, UINT32 uiPortEnd);

BOOLEAN	OpenSSLProxy_IsPortInRange(USHORT usSrcPort);

BOOLEAN	OpenSSLProxy_RuleIsMatch(IN UINT32 uiIPAddr, IN USHORT usPort);



