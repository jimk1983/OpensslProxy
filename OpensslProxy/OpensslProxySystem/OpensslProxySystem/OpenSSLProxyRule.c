#include <ntddk.h>
#pragma warning(push)
#pragma warning(disable:4201)       
#include <fwpsk.h>
#pragma warning(pop)
#include <fwpmk.h>

#include <ntstrsafe.h>
#include <wdmsec.h>
#include <ws2ipdef.h>
#include <in6addr.h>
#include <ip2string.h>

#define INITGUID
#include <guiddef.h>
#include <wchar.h>

#include "OpenSSLProxyDefine.h"
#include "OpenSSLProxyKernel.h"
#include "OpenSSLProxyCallout.h"
#include "OpenSSLProxyDriver.h"
#include "OpenSSLProxyUtils.h"
#include "OpenSSLProxyRule.h"

PRULE_MGR_CTX_S						g_pstRuleMgrCtx = NULL;
NPAGED_LOOKASIDE_LIST			g_RuleInfoEntryLookasideList;

VOID OpenSSLPRoxy_RuleListRelease(PLIST_ENTRY pstList);


PRULE_INFO_ENTRY OpenSSLProxy_RuleEntryAllocate(IN UINT32 uiRuleIP, IN USHORT usRulePort)
{
	PRULE_INFO_ENTRY  pRuleEntry = NULL;

	pRuleEntry = (PRULE_INFO_ENTRY)ExAllocateFromNPagedLookasideList(&g_RuleInfoEntryLookasideList);
	if (NULL == pRuleEntry)
	{
		KdPrint(("[MFSPY]::mspy utls allocate create error!\r\n"));
		return NULL;
	}

	RtlZeroMemory(pRuleEntry, sizeof(RULE_INFO_ENTRY));
	InitializeListHead(&pRuleEntry->listEntry);
	pRuleEntry->uiRuleIP		= uiRuleIP;
	pRuleEntry->uiRulePort	= usRulePort;
	
	return pRuleEntry;
}

VOID OpenSSLProxy_RuleEntryFree(IN PRULE_INFO_ENTRY pRuleEntry)
{
	if (NULL != pRuleEntry)
	{
		ExFreeToNPagedLookasideList(&g_RuleInfoEntryLookasideList, pRuleEntry);
	}
	return;
}

/*初始化规则信息*/
NTSTATUS OpenSSLProxy_RuleInit()
{
	NTSTATUS Status = STATUS_SUCCESS;

	g_pstRuleMgrCtx =  (PRULE_MGR_CTX_S)ExAllocatePoolWithTag(NonPagedPool, sizeof(RULE_MGR_CTX_S), OPENSSLPROXY_MEM_TAG);
	if (NULL == g_pstRuleMgrCtx )
	{
		KdPrint(("[OPENSSLDRV]: #OpenSSLProxy_RuleInit#-->Malloc error\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlZeroMemory(g_pstRuleMgrCtx, sizeof(RULE_MGR_CTX_S));
	ExInitializeResourceLite(&g_pstRuleMgrCtx->stResLock.stLockResource);
	KeInitializeSpinLock(&g_pstRuleMgrCtx->stResLock.stLockKSpin);
	InitializeListHead(&g_pstRuleMgrCtx->stRulePortList);
	InitializeListHead(&g_pstRuleMgrCtx->stRuleIPaddrList);
	InitializeListHead(&g_pstRuleMgrCtx->stRuleIPPortList);

	ExInitializeNPagedLookasideList(&g_RuleInfoEntryLookasideList,
		NULL,
		NULL,
		0,
		sizeof(RULE_INFO_ENTRY),
		OPENSSLPROXY_RULE_TAG,
		0);

	return Status;
}


/*释放规则资源*/
VOID OpenSSLProxy_RuleUnInit()
{
	KIRQL						OldIrql;

	OpenSSLProxy_ResourceLock(&g_pstRuleMgrCtx->stResLock,  &OldIrql);
	OpenSSLPRoxy_RuleListRelease(&g_pstRuleMgrCtx->stRulePortList);
	OpenSSLPRoxy_RuleListRelease(&g_pstRuleMgrCtx->stRuleIPaddrList);
	OpenSSLPRoxy_RuleListRelease(&g_pstRuleMgrCtx->stRuleIPPortList);
	OpenSSLProxy_ResourceUnLock(&g_pstRuleMgrCtx->stResLock, OldIrql);

	if (NULL != g_pstRuleMgrCtx )
	{
		ExDeleteResourceLite(&g_pstRuleMgrCtx->stResLock.stLockResource);

		ExFreePool(g_pstRuleMgrCtx);
		g_pstRuleMgrCtx = NULL;
	}
	ExDeleteNPagedLookasideList(&g_RuleInfoEntryLookasideList);
}


VOID OpenSSLPRoxy_RuleListRelease(PLIST_ENTRY pstList)
{
	PRULE_INFO_ENTRY  pRuleEntry = NULL;
	PLIST_ENTRY			listEntry = NULL;

	while (!IsListEmpty(pstList))
	{
		if (!IsListEmpty(pstList))
		{
			listEntry = RemoveHeadList(pstList);
		}

		if (listEntry != NULL)
		{
			pRuleEntry = CONTAINING_RECORD(
				listEntry,
				RULE_INFO_ENTRY,
				listEntry);

			OpenSSLProxy_RuleEntryFree(pRuleEntry);
			listEntry = NULL;
		}
	}
}
/*根据地址和端口进行匹配*/
BOOLEAN OpenSSLProxy_RuleIsMatch(IN UINT32 uiIPAddr, IN USHORT usPort)
{

	/*1. 0.0.0.0: 80 任意地址规则*/

	/*2. 10.10.10.1: 0 某地址的任意端口规则 */

	/*3. 10.10.10.1:8080 ，地址+端口完全匹配的规则*/

#if  DBG
	KdPrint(("[OPENSSLDRV]: #OpenSSLProxy_RuleIsMatch#-->Match Rule IPAddr=%08x:%d\n", uiIPAddr, usPort));
#endif
	return FALSE;
}




