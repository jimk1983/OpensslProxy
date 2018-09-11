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

/*本地的127.0.0.1的环回地址*/
IN_ADDR		g_LocalProxySockaddr = { 0 };


NTSTATUS OpenSSLProxy_EnvInit()
{
	NTSTATUS Status = STATUS_SUCCESS;

	Status = OpenSSLProxy_RuleInit();
	if (STATUS_SUCCESS != Status )
	{
		return Status;
	}

	LocalHostIPStringToSockAddr(&g_LocalProxySockaddr);

	return Status;
}

VOID	OpenSSLProxy_EnvUnInit()
{
	OpenSSLProxy_RuleUnInit();
}



IN_ADDR *OpenSSLProxy_GetLocalSockaddr()
{
	return &g_LocalProxySockaddr;
}













