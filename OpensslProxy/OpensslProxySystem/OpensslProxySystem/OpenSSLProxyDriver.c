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

// 
// Callout and sublayer GUIDs
//

//1e818ad3-e247-4cc5-af81-f215c301fb96
DEFINE_GUID(
	OPENSSLPROXY_SUBLAYER,
	0x1e818ad3,
	0xe247,
	0x4cc5,
	0xaf, 0x81, 0xf2, 0x15, 0xc3, 0x01, 0xfb, 0x96
);

//connection redirectID
//22b026f1-42ac-4f4b-9ccb-b4bc11cbf29f
DEFINE_GUID(
	OPENSSLPROXY_CONNECTION_REDIRECTED_CALLOUT_V4,
	0x22b026f1,
	0x42ac,
	0x4f4b,
	0x9c, 0xcb, 0xb4, 0xbc, 0x11, 0xcb, 0xf2, 0x9f
);

BOOLEAN					gDriverUnloading = FALSE;
PDEVICE_OBJECT			gDeviceObject;
HANDLE						gEngineHandle;


VOID DriverUnload(
	IN  PDRIVER_OBJECT driverObject
)
{
	PDEVICE_EXTENSION		pDevExt;

	if (NULL != gDeviceObject)
	{
		pDevExt = (PDEVICE_EXTENSION)gDeviceObject->DeviceExtension;
		IoDeleteSymbolicLink(&pDevExt->ustrSymLinkName);
		IoDeleteDevice(gDeviceObject);
		gDeviceObject = NULL;
	}

	gDriverUnloading = TRUE;

	KdPrint(("[OPENSSLDRV]: #DriverUnload#-->Driver unload successful!\n"));

	UNREFERENCED_PARAMETER(driverObject);
	return;
}


NTSTATUS DriverEntry(
	IN  PDRIVER_OBJECT  driverObject,
	IN  PUNICODE_STRING registryPath)
{
	NTSTATUS					status = STATUS_SUCCESS;
	UNICODE_STRING		DeviceName;
	UNICODE_STRING		DeviceSymLinkName; 
	UNICODE_STRING		ustrSecDDL;
	PDEVICE_EXTENSION  pDevExt;

	RtlInitUnicodeString(&ustrSecDDL, L"D:P(A;;GA;;;AU)");
	RtlInitUnicodeString(&DeviceName, OPENSSLPROXYDRV);
	status = IoCreateDeviceSecure(driverObject,
													sizeof(DEVICE_EXTENSION),
													&DeviceName,
													FILE_DEVICE_NETWORK,
													0,
													FALSE,
													&ustrSecDDL, 
													NULL, 
													&gDeviceObject);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[OPENSSLDRV]: #DriverEntry#-->IoCreateDeviceSecure error=%08x\n", status));
		return status;
	}

	RtlInitUnicodeString(&DeviceSymLinkName, OPENSSLPROXYDOSDRV);
	status = IoCreateSymbolicLink(&DeviceSymLinkName, &DeviceName);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[OPENSSLDRV]: #DriverEntry#-->IoCreateSymbolicLink error=%08x\n", status));
		IoDeleteDevice(gDeviceObject);
		gDeviceObject = NULL;
		return status;
	}

	gDeviceObject->Flags |= DO_BUFFERED_IO;
	pDevExt = (PDEVICE_EXTENSION)gDeviceObject->DeviceExtension;
	pDevExt->pDevice = gDeviceObject;
	pDevExt->ustrDeviceName = DeviceName;
	pDevExt->ustrSymLinkName = DeviceSymLinkName;




	driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverControl;
	driverObject->DriverUnload = DriverUnload;
	KdPrint(("[OPENSSLDRV]: #DriverEntry#-->DriverLoad successful!\n", status));
	UNREFERENCED_PARAMETER(registryPath);
	return status;
}
