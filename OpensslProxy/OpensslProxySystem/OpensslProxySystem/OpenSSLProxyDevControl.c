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



NTSTATUS DriverControl(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP p_IRP)
{
	PIO_STACK_LOCATION l_IrpSp;
	NTSTATUS					 Status = STATUS_SUCCESS;

	l_IrpSp		= IoGetCurrentIrpStackLocation(p_IRP);
	p_IRP->IoStatus.Status = STATUS_SUCCESS;
	p_IRP->IoStatus.Information = 0;



	UNREFERENCED_PARAMETER(DeviceObject);
	return Status;
}


NTSTATUS DriverCompeleteRequest(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP p_IRP)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PAGED_CODE();

	p_IRP->IoStatus.Status					= STATUS_SUCCESS;
	p_IRP->IoStatus.Information		= 0;

	IoCompleteRequest(p_IRP, IO_NO_INCREMENT);

	UNREFERENCED_PARAMETER(DeviceObject);
	return STATUS_SUCCESS;
}

NTSTATUS DriverCreate(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP p_IRP)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PAGED_CODE();

	p_IRP->IoStatus.Status = STATUS_SUCCESS;
	p_IRP->IoStatus.Information = FILE_OPENED;

	IoCompleteRequest(p_IRP, IO_NO_INCREMENT);

	UNREFERENCED_PARAMETER(DeviceObject);
	return STATUS_SUCCESS;
}
