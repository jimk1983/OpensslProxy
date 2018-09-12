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
		KdPrint(("[OPENSSLDRV]: #OpenSSLProxy_RuleEntryAllocate#-->LooksideList allcate error!\n"));
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

NTSTATUS OpenSSLProxy_RuleEntryAdd(IN UINT32 uiRuleIP, IN USHORT usRulePort)
{
	KIRQL							OldIrql;
	NTSTATUS					Status = STATUS_SUCCESS;
	PRULE_INFO_ENTRY	pRuleEntry = NULL;
	UINT32						uiRuleType = OPENSSLPROXY_LISTTYPE_IPPORT;

	/*规则不能同时为0*/
	if (0 == uiRuleIP && 0 == usRulePort)
	{
		KdPrint(("[OPENSSLDRV]: #OpenSSLProxy_RuleEntryAdd#-->Add rule ip and port error!, info=[%08x:%d]\n", uiRuleIP, usRulePort));
		return STATUS_INVALID_PARAMETER;
	}

	/*先根据地址进行判断*/
	if (0==uiRuleIP)
	{
		uiRuleType = OPENSSLPROXY_LISTTYPE_IPADDR;
	}
	else if (0 == usRulePort)
	{
		uiRuleType = OPENSSLPROXY_LISTTYPE_PORT;
	}
	else
	{
		uiRuleType = OPENSSLPROXY_LISTTYPE_IPPORT;
	}

	pRuleEntry = OpenSSLProxy_RuleEntryAllocate(uiRuleIP, usRulePort);
	if (NULL == pRuleEntry)
	{
		KdPrint(("[OPENSSLDRV]: #OpenSSLProxy_RuleEntryAdd#-->Allocate Rule Entry error\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pRuleEntry->uiRuleType = uiRuleType;
	
	OpenSSLProxy_ResourceLock(&g_pstRuleMgrCtx->stResLock, &OldIrql);
	switch (uiRuleType)
	{
	case OPENSSLPROXY_LISTTYPE_PORT:
		InsertTailList(&g_pstRuleMgrCtx->stRulePortList, &pRuleEntry->listEntry);
		g_pstRuleMgrCtx->uiRuleNums++;
		break;
	case OPENSSLPROXY_LISTTYPE_IPADDR:
		InsertTailList(&g_pstRuleMgrCtx->stRuleIPaddrList, &pRuleEntry->listEntry);
		g_pstRuleMgrCtx->uiRuleNums++;
		break;
	case OPENSSLPROXY_LISTTYPE_IPPORT:
		InsertTailList(&g_pstRuleMgrCtx->stRuleIPPortList, &pRuleEntry->listEntry);
		g_pstRuleMgrCtx->uiRuleNums++;
		break;
	default:
		break;
	}
	OpenSSLProxy_ResourceUnLock(&g_pstRuleMgrCtx->stResLock, OldIrql);

	return Status;
}


VOID OpenSSLProxy_RuleEntryRemove(UINT32 uiRuleIP, IN USHORT usRulePort)
{
	KIRQL							OldIrql;
	PRULE_INFO_ENTRY	pRuleEntry = NULL;
	PLIST_ENTRY				pRulelist = NULL;
	PLIST_ENTRY				plistEntry = NULL;
	UINT32						uiRuleType = OPENSSLPROXY_LISTTYPE_IPPORT;

	/*先根据地址进行判断*/
	if (0 == uiRuleIP)
	{
		uiRuleType = OPENSSLPROXY_LISTTYPE_IPADDR;
		pRulelist = &g_pstRuleMgrCtx->stRuleIPaddrList;
	}
	else if (0 == usRulePort)
	{
		uiRuleType = OPENSSLPROXY_LISTTYPE_PORT;
		pRulelist = &g_pstRuleMgrCtx->stRulePortList;
	}
	else
	{
		uiRuleType = OPENSSLPROXY_LISTTYPE_IPPORT;
		pRulelist = &g_pstRuleMgrCtx->stRuleIPPortList;
	}

	/*链表*/
	while ( !IsListEmpty(pRulelist) )
	{
		OpenSSLProxy_ResourceLock(&g_pstRuleMgrCtx->stResLock, &OldIrql);

		if (!IsListEmpty(pRulelist))
		{
			plistEntry = RemoveHeadList(pRulelist);
		}

		if (plistEntry != NULL)
		{
			pRuleEntry = CONTAINING_RECORD(
											plistEntry,
											RULE_INFO_ENTRY,
											listEntry);
			if ( (uiRuleIP == pRuleEntry->uiRuleIP)
				&& (usRulePort == pRuleEntry->uiRulePort) )
			{
				g_pstRuleMgrCtx->uiRuleNums--;
				OpenSSLProxy_ResourceUnLock(&g_pstRuleMgrCtx->stResLock, OldIrql);
				break;
			}
			else
			{
				InsertTailList(pRulelist, &pRuleEntry->listEntry);
			}
		}
		OpenSSLProxy_ResourceUnLock(&g_pstRuleMgrCtx->stResLock, OldIrql);
	}

	OpenSSLProxy_RuleEntryFree(pRuleEntry);
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

	g_pstRuleMgrCtx->stSrcPortRange.uiLocalPortStart = 0;
	g_pstRuleMgrCtx->stSrcPortRange.uiLocalPortEnd	= 0;

	ExInitializeNPagedLookasideList(&g_RuleInfoEntryLookasideList,
		NULL,
		NULL,
		0,
		sizeof(RULE_INFO_ENTRY),
		OPENSSLPROXY_RULE_TAG,
		0);

	g_pstRuleMgrCtx->uiRuleNums = 0;

	return Status;
}


NTSTATUS OpenSSLProxy_GetSrcPortRange(UINT32 *puiPortStart, UINT32 *puiPortEnd)
{
	NTSTATUS				Status = STATUS_SUCCESS;
	KIRQL						OldIrql;

	if (puiPortStart == NULL
		|| puiPortStart == NULL)
	{
		return STATUS_INVALID_PARAMETER;
	}

	OpenSSLProxy_ResourceLock(&g_pstRuleMgrCtx->stResLock, &OldIrql);
	*puiPortStart = g_pstRuleMgrCtx->stSrcPortRange.uiLocalPortStart;
	*puiPortEnd  = g_pstRuleMgrCtx->stSrcPortRange.uiLocalPortEnd;
	OpenSSLProxy_ResourceUnLock(&g_pstRuleMgrCtx->stResLock, OldIrql);

	return Status;
}

NTSTATUS OpenSSLProxy_SetSrcPortRange(UINT32 uiPortStart, UINT32 uiPortEnd)
{
	NTSTATUS				Status = STATUS_SUCCESS;
	KIRQL						OldIrql;

	if (uiPortEnd >= OPENSSLPROXY_MAXPORT
		|| uiPortStart >= OPENSSLPROXY_MAXPORT )
	{
		return STATUS_INVALID_PARAMETER;
	}

	OpenSSLProxy_ResourceLock(&g_pstRuleMgrCtx->stResLock, &OldIrql);
	g_pstRuleMgrCtx->stSrcPortRange.uiLocalPortStart = uiPortStart;
	g_pstRuleMgrCtx->stSrcPortRange.uiLocalPortEnd = uiPortEnd;
	OpenSSLProxy_ResourceUnLock(&g_pstRuleMgrCtx->stResLock, OldIrql);

	return Status;
}

BOOLEAN	OpenSSLProxy_IsPortInRange(USHORT usSrcPort)
{
	KIRQL						OldIrql;

	OpenSSLProxy_ResourceLock(&g_pstRuleMgrCtx->stResLock, &OldIrql);

	if ( usSrcPort > g_pstRuleMgrCtx->stSrcPortRange.uiLocalPortStart 
		&&  usSrcPort < g_pstRuleMgrCtx->stSrcPortRange.uiLocalPortEnd )
	{
		OpenSSLProxy_ResourceUnLock(&g_pstRuleMgrCtx->stResLock, OldIrql);
		return TRUE;
	}

	OpenSSLProxy_ResourceUnLock(&g_pstRuleMgrCtx->stResLock, OldIrql);

	return FALSE;
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


VOID OpenSSLProxy_RuleAllClear()
{
	KIRQL						OldIrql;

	OpenSSLProxy_ResourceLock(&g_pstRuleMgrCtx->stResLock, &OldIrql);
	OpenSSLPRoxy_RuleListRelease(&g_pstRuleMgrCtx->stRulePortList);
	OpenSSLPRoxy_RuleListRelease(&g_pstRuleMgrCtx->stRuleIPaddrList);
	OpenSSLPRoxy_RuleListRelease(&g_pstRuleMgrCtx->stRuleIPPortList);
	OpenSSLProxy_ResourceUnLock(&g_pstRuleMgrCtx->stResLock, OldIrql);
}

VOID OpenSSLProxy_RuleTypeClear(UINT32 uiRuleType)
{
	KIRQL						OldIrql;

	OpenSSLProxy_ResourceLock(&g_pstRuleMgrCtx->stResLock, &OldIrql);
	switch (uiRuleType)
	{
		case OPENSSLPROXY_LISTTYPE_PORT:
			OpenSSLPRoxy_RuleListRelease(&g_pstRuleMgrCtx->stRulePortList);
			break;
		case OPENSSLPROXY_LISTTYPE_IPADDR:
			OpenSSLPRoxy_RuleListRelease(&g_pstRuleMgrCtx->stRuleIPaddrList);
			break;
		case OPENSSLPROXY_LISTTYPE_IPPORT:
			OpenSSLPRoxy_RuleListRelease(&g_pstRuleMgrCtx->stRuleIPPortList);
			break;
		default:
			break;
	}
	OpenSSLProxy_ResourceUnLock(&g_pstRuleMgrCtx->stResLock, OldIrql);
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




