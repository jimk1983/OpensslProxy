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
#include "OpenSSLProxyEnvInit.h"

BOOLEAN					gDriverUnloading = FALSE;
BOOLEAN					gConnectionRedirectEnable = FALSE;
PDEVICE_OBJECT			gDeviceObject;
HANDLE						gEngineHandle;


VOID DriverUnload(
	IN  PDRIVER_OBJECT driverObject
)
{
	PDEVICE_EXTENSION		pDevExt;

	OpenSSLProxy_EnvUnInit();

	OpenSSLProxy_UnRegisterCallouts();

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
	

	if (STATUS_SUCCESS != OpenSSLProxy_EnvInit())
	{
		KdPrint(("[OPENSSLDRV]: #DriverEntry#-->OpenSSLProxy EnvInit error=%08x\n", status));
		IoDeleteSymbolicLink(&DeviceSymLinkName);
		IoDeleteDevice(gDeviceObject);
		gDeviceObject = NULL;
		return status;
	}

	status = OpenSSLProxy_RegisterCallouts(gDeviceObject);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[OPENSSLDRV]: #DriverEntry#-->OpenSSLProxy RegisterCallouts error=%08x\n", status));
		OpenSSLProxy_EnvUnInit();
		IoDeleteSymbolicLink(&DeviceSymLinkName);
		IoDeleteDevice(gDeviceObject);
		gDeviceObject = NULL;
		return status;
	}

	for ( int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		driverObject->MajorFunction[i] = DriverCompeleteRequest;
	}

	driverObject->MajorFunction[IRP_MJ_CREATE]						= DriverCreate;
	driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]	= DriverControl;

	driverObject->DriverUnload = DriverUnload;
	KdPrint(("[OPENSSLDRV]: #DriverEntry#-->DriverLoad successful!\n", status));
	UNREFERENCED_PARAMETER(registryPath);
	return status;
}
