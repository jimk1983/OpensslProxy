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

#include "mstcpip.h"
#include "OpenSSLProxyDefine.h"
#include "OpenSSLProxyKernel.h"
#include "OpenSSLProxyCallout.h"
#include "OpenSSLProxyDriver.h"
#include "OpenSSLProxyUtils.h"
#include "OpenSSLProxyRule.h"
#include "OpenSSLProxyEnvInit.h"


UINT32					g_uiConnectRedirectCalloutIdV4 = 0;
HANDLE					g_hRedirectHandle			= NULL;


VOID OpenSSLProxy_ConnectionRedirectClassify(
	IN const FWPS_INCOMING_VALUES*						inFixedValues,
	IN const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	IN OUT OPTIONAL VOID*										layerData,
	IN OPTIONAL const VOID*										classifyContext,
	IN const FWPS_FILTER*											filter,
	IN UINT64																flowContext,
	IN OUT FWPS_CLASSIFY_OUT*								classifyOut
)
{
	NTSTATUS    Status = STATUS_SUCCESS;
	UINT64        ClassifyHandle = 0;
	VOID*         WriteableLayerData = NULL;
	FWPS_CONNECT_REQUEST* pConnectRequest = NULL;
#if(NTDDI_VERSION >= NTDDI_WIN8)
	SOCKADDR_STORAGE* pSockAddrStorage = NULL;
#endif
	//FWP_BYTE_BLOB *pstProcInfo = inMetaValues->processPath;
	ULONG		ProcessID = (ULONG)inMetaValues->processId;
	UINT8			Protocol = inFixedValues->incomingValue[FWPS_FIELD_ALE_CONNECT_REDIRECT_V4_IP_PROTOCOL].value.uint8;
	UINT32		remoteipaddr = 0;
	USHORT		remotePort = 0;
	UINT32		timesRedirected = 0;
	UINT32		localipaddr = 0;
	USHORT		localport = 0;

	if ( FALSE == gConnectionRedirectEnable )
	{
		goto Exit;
	}

	if ((classifyOut->rights & FWPS_RIGHT_ACTION_WRITE) == 0)
	{
		KdPrint(("[OPENSSLDRV]: #ConnectionRedirectClassify#-->Action not write!\n"));
		goto Exit;
	}

#if(NTDDI_VERSION >= NTDDI_WIN8)

	FWPS_CONNECTION_REDIRECT_STATE redirectionState = FWPS_CONNECTION_NOT_REDIRECTED;

	if (FWPS_IS_METADATA_FIELD_PRESENT(inMetaValues, FWPS_METADATA_FIELD_REDIRECT_RECORD_HANDLE))
	{
		VOID* pRedirectContext = NULL;

		redirectionState = FwpsQueryConnectionRedirectState(inMetaValues->redirectRecords,
			g_hRedirectHandle,
			&pRedirectContext);
	}

	switch (redirectionState)
	{
		/// Go ahead and continue with our redirection
	case FWPS_CONNECTION_NOT_REDIRECTED:
	case FWPS_CONNECTION_REDIRECTED_BY_OTHER:
	{
		//KdPrint(("[OPENSSLDRV]: #ConnectionRedirectClassify#-->Ale connection not redirected, goto redirect handler!"));
		break;
	}
	/// We've already seen this, so let it through
	case FWPS_CONNECTION_REDIRECTED_BY_SELF:
	{
		classifyOut->actionType = FWP_ACTION_PERMIT;
		KdPrint(("[OPENSSLDRV]: #ConnectionRedirectClassify#-->Ale connection not redirect by self, permit it !\n"));
		goto Exit;
	}
	/// Must not perform redirection. In this case we are letting the last redirection action win.
	case FWPS_CONNECTION_PREVIOUSLY_REDIRECTED_BY_SELF:
	{
		KdPrint(("[OPENSSLDRV]: #ConnectionRedirectClassify#-->Ale connection redirect by self previously, exit!\n"));
		goto Exit;
	}
	}
#endif /// (NTDDI_VERSION >= NTDDI_WIN8)
	Status = FwpsAcquireClassifyHandle((void*)classifyContext, 0, &ClassifyHandle);
	if (Status != STATUS_SUCCESS)
	{
		KdPrint(("[OPENSSLDRV]: #ConnectionRedirectClassify#-->FwpsAcquireClassifyHandle error!status=%#x\n", Status));
		goto Exit;
	}

	Status = FwpsAcquireWritableLayerDataPointer(ClassifyHandle,
		filter->filterId,
		0,
		&WriteableLayerData,
		classifyOut
	);
	if (Status != STATUS_SUCCESS)
	{
		KdPrint(("[OPENSSLDRV]: #ConnectionRedirectClassify#-->FwpsAcquireWritableLayerDataPointer error!status=%#x\n", Status));

		goto Exit;
	}

	if (inFixedValues->incomingValue[FWPS_FIELD_ALE_CONNECT_REDIRECT_V4_FLAGS].value.uint32 & FWP_CONDITION_FLAG_IS_REAUTHORIZE)
	{
		pConnectRequest = ((FWPS_CONNECT_REQUEST*)(WriteableLayerData))->previousVersion;

		if (pConnectRequest->modifierFilterId == filter->filterId)
		{
			classifyOut->actionType = FWP_ACTION_PERMIT;
			classifyOut->rights |= FWPS_RIGHT_ACTION_WRITE;
			goto Exit;
		}
	}

	for (pConnectRequest = ((FWPS_CONNECT_REQUEST*)(WriteableLayerData))->previousVersion;
		pConnectRequest;
		pConnectRequest = pConnectRequest->previousVersion)
	{
		if (pConnectRequest->modifierFilterId == filter->filterId)
			timesRedirected++;

		if (timesRedirected > 3)
		{
			Status = STATUS_TOO_MANY_COMMANDS;
			goto Exit;
		}
	}

	pConnectRequest = (FWPS_CONNECT_REQUEST*)WriteableLayerData;

	remoteipaddr = ((PSOCKADDR_IN)&(pConnectRequest->remoteAddressAndPort))->sin_addr.S_un.S_addr;
	remotePort = ((PSOCKADDR_IN)&(pConnectRequest->remoteAddressAndPort))->sin_port;
	localipaddr = ((PSOCKADDR_IN)&(pConnectRequest->localAddressAndPort))->sin_addr.S_un.S_addr;
	localport = ((PSOCKADDR_IN)&(pConnectRequest->localAddressAndPort))->sin_port;

	/*KdPrint(("[OPENSSLDRV]: #ConnectionRedirectClassify#-->Ale-Connection info:%8x:%d -->%08x:%d ,ProcessID=%d\n",
		localipaddr, ntohs(localport), remoteipaddr, ntohs(remotePort), ProcessID));

	if (NULL != pstProcInfo )
	{
		KdPrint(("[OPENSSLDRV]: #ConnectionRedirectClassify#-->Ale-Connection info:ProcessID=[%d], ProcessName=[len=%d, %s]\n",
			ProcessID, pstProcInfo->size, pstProcInfo->data));
	}*/
	
	if ( TRUE == OpenSSLProxy_RuleIsMatch(remoteipaddr, remotePort) )
	{
		KdPrint(("[OPENSSLDRV]: #ConnectionRedirectClassify#-->Ale Redirect connection Rule is Match! [pto=%d] %08x:%d --> %08x:%d,PID=%d\n",
			Protocol, ntohl(localipaddr), ntohs(localport), ntohl(remoteipaddr), ntohs(remotePort), ProcessID));

		if (INETADDR_ISANY((PSOCKADDR)&(pConnectRequest->localAddressAndPort)))
		{
			INETADDR_SETLOOPBACK((PSOCKADDR)&(pConnectRequest->remoteAddressAndPort));
		}
		else
		{
			INETADDR_SET_ADDRESS((PSOCKADDR)&(pConnectRequest->remoteAddressAndPort),
				INETADDR_ADDRESS((PSOCKADDR)&(pConnectRequest->localAddressAndPort)));
		}

		INETADDR_SET_ADDRESS((PSOCKADDR)&(pConnectRequest->remoteAddressAndPort), (const UCHAR *)OpenSSLProxy_GetLocalSockaddr());
		//INETADDR_SET_PORT((PSOCKADDR)&(pConnectRequest->remoteAddressAndPort), RegEditGetTcpLocalPort());
		//pConnectRequest->localRedirectTargetPID = RegEditGetLocalServerPID();
	}

Exit:
#if(NTDDI_VERSION >= NTDDI_WIN8)
	if (Status != STATUS_SUCCESS)
	{
		if (pSockAddrStorage != NULL)
		{
			ExFreePool(pSockAddrStorage);
		}
	}

#endif 
	if (Status != STATUS_SUCCESS)
	{
		classifyOut->actionType = FWP_ACTION_BLOCK;
	}
	else
	{
		classifyOut->actionType = FWP_ACTION_PERMIT;
	}

	classifyOut->rights ^= FWPS_RIGHT_ACTION_WRITE;

	if (WriteableLayerData)
	{
		FwpsApplyModifiedLayerData(ClassifyHandle,
			WriteableLayerData,
			FWPS_CLASSIFY_FLAG_REAUTHORIZE_IF_MODIFIED_BY_OTHERS);
	}

	if (ClassifyHandle)
	{
		FwpsReleaseClassifyHandle(ClassifyHandle);
	}

	UNREFERENCED_PARAMETER(layerData);
	UNREFERENCED_PARAMETER(flowContext);
	return;
}


NTSTATUS OpenSSLProxy_ConnectionRedirectNotify(
	IN FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	IN const GUID* filterKey,
	IN OUT const FWPS_FILTER* filter)
{
	UNREFERENCED_PARAMETER(notifyType);
	UNREFERENCED_PARAMETER(filterKey);
	UNREFERENCED_PARAMETER(filter);

	return STATUS_SUCCESS;
}


NTSTATUS OpenSSLProxy_ConnectRedirectAddFilter(
	IN const wchar_t* filterName,
	IN const wchar_t* filterDesc,
	IN UINT64 context,
	IN const GUID* layerKey,
	IN const GUID* calloutKey
)
{
	NTSTATUS status = STATUS_SUCCESS;
	FWPM_FILTER filter = { 0 };
	FWPM_FILTER_CONDITION filterConditions[3] = { 0 };
	UINT conditionIndex;
	UINT64  _64 = 0xFFFFFFFFFFFFFFFF;

	filter.layerKey = *layerKey;
	filter.displayData.name = (wchar_t*)filterName;
	filter.displayData.description = (wchar_t*)filterDesc;
	filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
	filter.action.calloutKey = *calloutKey;
	filter.filterCondition = filterConditions;
	filter.subLayerKey = OPENSSLPROXY_SUBLAYER;
	filter.rawContext = context;

	//filter.weight.type = FWP_UINT64;
	filter.weight.uint64 = &_64;

	conditionIndex = 0;

	filterConditions[conditionIndex].fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;
	filterConditions[conditionIndex].matchType = FWP_MATCH_NOT_EQUAL;
	filterConditions[conditionIndex].conditionValue.type = FWP_UINT32;
	filterConditions[conditionIndex].conditionValue.uint32 = htonl(0x7F000001);
	conditionIndex++;

	filter.numFilterConditions = conditionIndex;
	status = FwpmFilterAdd(
		gEngineHandle,
		&filter,
		NULL,
		NULL);

	return status;
}


/*×¢²áÖØ¶¨Ïòsublayer*/
NTSTATUS OpenSSLProxy_ConnectionRedirectRegisterCallouts(
	IN const GUID* layerKey,
	IN const GUID* calloutKey,
	IN void* deviceObject,
	OUT UINT32* calloutId )
{
	NTSTATUS status = STATUS_SUCCESS;
	FWPM_DISPLAY_DATA displayData = { 0 };
	FWPS_CALLOUT sCallout = { 0 };
	FWPM_CALLOUT mCallout = { 0 };
	BOOLEAN CalloutRegistered = FALSE;

	sCallout.calloutKey		= *calloutKey;
	sCallout.notifyFn			= OpenSSLProxy_ConnectionRedirectNotify;
	sCallout.classifyFn		= OpenSSLProxy_ConnectionRedirectClassify;

	status = FwpsCalloutRegister(
		deviceObject,
		&sCallout,
		calloutId);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[OPENSSLDRV]: #RegisterCallouts#-->FwpsCalloutRegister error=%08x\n", status));
		goto Exit;
	}
	CalloutRegistered = TRUE;

	displayData.name = L"OpenSSLProxy connection redirect Callout";
	displayData.description = L"OpenSSLProxy connection redirect traffic";

	mCallout.calloutKey = *calloutKey;
	mCallout.displayData = displayData;
	mCallout.applicableLayer = *layerKey;

	status = FwpmCalloutAdd(
		gEngineHandle,
		&mCallout,
		NULL,
		NULL
	);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("FwpmCalloutAdd0 error!");
		goto Exit;
	}

	status = OpenSSLProxy_ConnectRedirectAddFilter(
						L"OpenSSLProxy ALE connection redirect",
						L"OpenSSLProxy ALE connection redirect traffic",
						0,
						layerKey,
						calloutKey);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[OPENSSLDRV]: #OpenSSLProxyConnectionRedirectRegisterCallouts#-->Add ReConnection filter error=%08x\n", status));
		goto Exit;
	}

Exit:

	if (!NT_SUCCESS(status))
	{
		KdPrint(("[OPENSSLDRV]: #OpenSSLProxyConnectionRedirectRegisterCallouts#-->Add ReConnection filter error=%08x\n", status));
		if (CalloutRegistered)
		{
			FwpsCalloutUnregisterById(*calloutId);
			*calloutId = 0;
		}
	}

	return status;
}


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
		&OPENSSLPROXY_SUBLAYER,
		0,
		&g_hRedirectHandle);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[OPENSSLDRV]: #OpenSSLProxy_RegisterCallouts#-->Win8 FwpsRedirectHandleCreate error=%08x!", status));
		goto Exit;
	}
#endif

	inTransaction = TRUE;

	status = OpenSSLProxy_ConnectionRedirectRegisterCallouts(
		&FWPM_LAYER_ALE_CONNECT_REDIRECT_V4,
		&OPENSSLPROXY_CONNECTION_REDIRECTED_CALLOUT_V4,
		deviceObject,
		&g_uiConnectRedirectCalloutIdV4
	);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

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
		if ( NULL != g_hRedirectHandle)
		{
			FwpsRedirectHandleDestroy(g_hRedirectHandle);
			g_hRedirectHandle = NULL;
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


VOID OpenSSLProxy_UnRegisterCallouts()
{

#if (NTDDI_VERSION >= NTDDI_WIN8)
	if (NULL != g_hRedirectHandle)
	{
		FwpsRedirectHandleDestroy(g_hRedirectHandle);
		g_hRedirectHandle = NULL;
	}
#endif

	if (0 != g_uiConnectRedirectCalloutIdV4)
	{
		FwpsCalloutUnregisterById(g_uiConnectRedirectCalloutIdV4);
	}

	if (NULL != gEngineHandle)
	{
		FwpmEngineClose(gEngineHandle);
		gEngineHandle = NULL;
	}
}

