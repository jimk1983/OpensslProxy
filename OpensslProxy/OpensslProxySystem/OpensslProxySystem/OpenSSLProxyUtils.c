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

#define	LOCALADDR		 L"127.0.0.1"


VOID LocalHostIPStringToSockAddr(IN_ADDR *pstInAddr)
{
	UNICODE_STRING	realipuncodestring;
	PWSTR						terminator;
	const WCHAR			*localaddr = LOCALADDR;

	if (NULL == pstInAddr)
	{
		return;
	}

	RtlInitUnicodeString(&realipuncodestring, localaddr);

	(VOID) RtlIpv4StringToAddressW( (PCWSTR)localaddr, TRUE, &terminator, pstInAddr);

	return;
}


VOID OpenSSLProxy_ResourceLock(PRESOURCE_LOCK_S pLock, PKIRQL OldIrql)
{
	if (KeGetCurrentIrql() <= APC_LEVEL)
	{
		ExAcquireResourceExclusiveLite(&pLock->stLockResource, TRUE);
	}
	else
	{
		KeAcquireSpinLock(&pLock->stLockKSpin, OldIrql);
	}
}

VOID OpenSSLProxy_ResourceUnLock(PRESOURCE_LOCK_S pLock, KIRQL OldIrql)
{
	if (KeGetCurrentIrql() <= APC_LEVEL)
	{
		ExReleaseResourceLite(&pLock->stLockResource);
	}
	else
	{
		KeReleaseSpinLock(&pLock->stLockKSpin, OldIrql);
	}
}
