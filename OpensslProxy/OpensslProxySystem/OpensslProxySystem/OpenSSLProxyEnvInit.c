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
#include "OpenSSLProxyEnvInit.h"

ENV_CTX_S	 *g_pstEnvCtx = NULL;

NTSTATUS OpenSSLProxy_EnvInit()
{
	NTSTATUS Status = STATUS_SUCCESS;

	g_pstEnvCtx  = (PENV_CTX_S)ExAllocatePoolWithTag(NonPagedPool, sizeof(ENV_CTX_S), OPENSSLPROXY_MEM_TAG);
	if (NULL == g_pstEnvCtx)
	{
		KdPrint(("[OPENSSLDRV]: #OpenSSLProxy_EnvInit#-->Malloc error\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlZeroMemory(g_pstEnvCtx, sizeof(ENV_CTX_S));
	LocalHostIPStringToSockAddr(&g_pstEnvCtx->stLocalSockaddr);

	Status = OpenSSLProxy_RuleInit();
	if (STATUS_SUCCESS != Status )
	{
		KdPrint(("[OPENSSLDRV]: #OpenSSLProxy_EnvInit#-->Rule init error\n"));
		ExFreePool(g_pstEnvCtx);
		g_pstEnvCtx = NULL;
		return Status;
	}

	return Status;
}

VOID	OpenSSLProxy_EnvUnInit()
{
	OpenSSLProxy_RuleUnInit();

	if (NULL != g_pstEnvCtx)
	{
		ExFreePool(g_pstEnvCtx);
		g_pstEnvCtx = NULL;
	}
}

IN_ADDR *OpenSSLProxy_GetLocalSockaddr()
{
	return &g_pstEnvCtx->stLocalSockaddr;
}

ULONG OpenSSLProxy_GetLocalProxyPID()
{
	ULONG ulLocalPid = 0;

	if ( NULL == g_pstEnvCtx)
	{
		return 0;
	}

	ulLocalPid = g_pstEnvCtx->ulLocalProxyPID;

	return ulLocalPid;
}


USHORT OpenSSLProxy_GetLocalProxyPort()
{
	if (NULL == g_pstEnvCtx)
	{
		return 0;
	}

	return g_pstEnvCtx->usLocalPort;
}


NTSTATUS	OpenSSLProxy_SetLocalProxyInfo(UINT32 uiLocalPid, USHORT usPort)
{
	if ( NULL == g_pstEnvCtx)
	{
		return STATUS_INVALID_PARAMETER;
	}

	g_pstEnvCtx->ulLocalProxyPID = uiLocalPid;
	g_pstEnvCtx->usLocalPort = usPort;

	return STATUS_SUCCESS;
}

















