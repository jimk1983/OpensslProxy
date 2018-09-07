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


/*×¢²áCallout*/
NTSTATUS OpenSSLProxy_RegisterCallouts( IN void* deviceObject)
{
	NTSTATUS status = STATUS_SUCCESS;
	FWPM_SUBLAYER   OpenSSLProxySubLayer;
	BOOLEAN engineOpened = FALSE;
	BOOLEAN inTransaction = FALSE;
	FWPM_SESSION session = { 0 };

	session.flags = FWPM_SESSION_FLAG_DYNAMIC;
	status = FwpmEngineOpen(
								NULL,
								RPC_C_AUTHN_WINNT,
								NULL,
								&session,
								&gEngineHandle);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}
	engineOpened = TRUE;

	status = FwpmTransactionBegin(gEngineHandle, 0);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	RtlZeroMemory(&OpenSSLProxySubLayer, sizeof(FWPM_SUBLAYER));

	OpenSSLProxySubLayer.subLayerKey = OPENSSLPROXY_SUBLAYER;
	OpenSSLProxySubLayer.displayData.name = L"OpenSSL Proxy Sub-Layer";
	OpenSSLProxySubLayer.displayData.description = L"Sub-Layer for use by OpenSSL Proxy callouts";
	OpenSSLProxySubLayer.flags = 0;
	OpenSSLProxySubLayer.weight = FWP_EMPTY; // auto-weight.;

	status = FwpmSubLayerAdd(gEngineHandle, &OpenSSLProxySubLayer, NULL);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

#if (NTDDI_VERSION >= NTDDI_WIN8)
	status = FwpsRedirectHandleCreate(
		&CLOUDSCREEN_SUBLAYER,
		0,
		&g_hredirecthandle);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("FwpsRedirectHandleCreate error!");
		goto Exit;
	}
#endif

	inTransaction = TRUE;



	status = FwpmTransactionCommit(gEngineHandle);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}
	inTransaction = FALSE;

Exit:
	if (!NT_SUCCESS(status))
	{

#if (NTDDI_VERSION >= NTDDI_WIN8)
		if (g_hredirecthandle)
		{
			FwpsRedirectHandleDestroy(g_hredirecthandle);
			g_hredirecthandle = NULL;
		}
#endif
		if (inTransaction)
		{
			FwpmTransactionAbort(gEngineHandle);
		}

		if (engineOpened)
		{
			FwpmEngineClose(gEngineHandle);
			gEngineHandle = NULL;
		}
	}
	UNREFERENCED_PARAMETER(deviceObject);
	return status;
}



VOID DriverUnRegisterCallouts()
{
	if (NULL != gEngineHandle)
	{
		FwpmEngineClose(gEngineHandle);
		gEngineHandle = NULL;
	}
	FwpsCalloutUnregisterById(gConnectRedirectCalloutIdV4);
}

