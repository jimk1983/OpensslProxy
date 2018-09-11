

/*规则管理上下文*/
typedef struct tagRuleMgrContext
{
	RESOURCE_LOCK_S					stResLock;				/*资源锁*/
	LIST_ENTRY								stRulePortList;			/*0.0.0.0:8080, 任意地址的端口匹配*/
	LIST_ENTRY								stRuleIPaddrList;		/*10.10.10.1:0, 任意端口的地址匹配*/
	LIST_ENTRY								stRuleIPPortList;		/*完全匹配*/
}RULE_MGR_CTX_S, *PRULE_MGR_CTX_S;

typedef struct tagRuleInfoEntry
{
	LIST_ENTRY		listEntry;
	UINT32			uiRuleIP;
	UINT32			uiRulePort;
}RULE_INFO_ENTRY, *PRULE_INFO_ENTRY;

NTSTATUS	OpenSSLProxy_RuleInit();

VOID			OpenSSLProxy_RuleUnInit();

PRULE_INFO_ENTRY OpenSSLProxy_RuleEntryAllocate(IN UINT32 uiRuleIP, IN USHORT usRulePort);

VOID			OpenSSLProxy_RuleEntryFree(IN PRULE_INFO_ENTRY pRuleEntry);

BOOLEAN	OpenSSLProxy_RuleIsMatch(IN UINT32 uiIPAddr, IN USHORT usPort);



