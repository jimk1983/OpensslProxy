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
#include "OpenSSLProxyDevControl.h"
#include "OpenSSLProxyUtils.h"
#include "OpenSSLProxyRule.h"
#include "OpenSSLProxyEnvInit.h"

NTSTATUS DeviceIoControl(
	__in_bcount_opt(InputBufferLength) PVOID InputBuffer,
	__in ULONG InputBufferLength,
	__out_bcount_opt(OutputBufferLength) PVOID OutputBuffer,
	__in ULONG OutputBufferLength,
	__in ULONG IoControlCode,
	__inout PIO_STATUS_BLOCK IoStatus
)
{
	NTSTATUS		Status = STATUS_SUCCESS;

	ASSERT(IoStatus != NULL);
	IoStatus->Status = Status;
	IoStatus->Information = 0;

	switch (IoControlCode)
	{
		case DEVICE_IOCTL_MATCHENABLE:
				gConnectionRedirectEnable = TRUE;
				KdPrint(("[OPENSSLDRV]: #DeviceIoControl#-->Device ioctl match Enable!\n"));
			break;
		case DEVICE_IOCTL_MATCHDISABLE:
				gConnectionRedirectEnable = FALSE;
				KdPrint(("[OPENSSLDRV]: #DeviceIoControl#-->Device ioctl match Disable!\n"));
			break;
		case DEVICE_IOCTL_SETRULECLEAR:
				OpenSSLProxy_RuleAllClear();
				KdPrint(("[OPENSSLDRV]: #DeviceIoControl#-->Rule All Clear!\n"));
			break;
		case DEVICE_IOCTL_SETRULETYPECLEAR:
			{
				UINT32 uiRuleType = OPENSSLPROXY_LISTTYPE_IPPORT;

				OpenSSLProxy_RuleTypeClear(uiRuleType);
				KdPrint(("[OPENSSLDRV]: #DeviceIoControl#-->Rule All Clear!\n"));
			}
			break;
		case DEVICE_IOCTL_SETRULEIPPORT:
			{
				PDEVICE_IOCTL_RULEINFO  pRuleInfo = (PDEVICE_IOCTL_RULEINFO)InputBuffer;

				if (InputBuffer == NULL
					|| InputBufferLength != sizeof(DEVICE_IOCTL_RULEINFO))
				{
					IoStatus->Status = STATUS_INVALID_PARAMETER;
				}
				else
				{
					Status = OpenSSLProxy_RuleEntryAdd(pRuleInfo->uiRuleIPAddr, (USHORT)pRuleInfo->uiRulePort);
					if (STATUS_SUCCESS != Status)
					{
						KdPrint(("[OPENSSLDRV]: #DeviceIoControl#-->Device add rule info error! [%d - %d]!\n",
							pRuleInfo->uiRuleIPAddr, pRuleInfo->uiRulePort));
					}
					else
					{
						KdPrint(("[OPENSSLDRV]: #DeviceIoControl#-->Device add rule info successful! [%d - %d]!\n",
							pRuleInfo->uiRuleIPAddr, pRuleInfo->uiRulePort));
					}
				}
			}
			break;
		case DEVICE_IOCTL_DELRULEIPPORT:
			{
				PDEVICE_IOCTL_RULEINFO  pRuleInfo = (PDEVICE_IOCTL_RULEINFO)InputBuffer;

				if (InputBuffer == NULL
					|| InputBufferLength != sizeof(DEVICE_IOCTL_RULEINFO))
				{
					IoStatus->Status = STATUS_INVALID_PARAMETER;
				}
				else
				{
					OpenSSLProxy_RuleEntryRemove(pRuleInfo->uiRuleIPAddr, (USHORT)pRuleInfo->uiRulePort);
					
					KdPrint(("[OPENSSLDRV]: #DeviceIoControl#-->Device delete rule info successful! [%d - %d]!\n",
						pRuleInfo->uiRuleIPAddr, pRuleInfo->uiRulePort));
				}
			}
			break;
		case DEVICE_IOCTL_SETPORTRANGE:
			{
				PDEVICE_IOCTL_PORTRANGE_S pPortRange = (PDEVICE_IOCTL_PORTRANGE_S)InputBuffer;
				if ( InputBuffer == NULL
					 || InputBufferLength != sizeof(DEVICE_IOCTL_PORTRANGE_S))
				{
					IoStatus->Status = STATUS_INVALID_PARAMETER;
				}
				else
				{
					Status = OpenSSLProxy_SetSrcPortRange(pPortRange->uiLocalPortStart, pPortRange->uiLocalPortEnd);
					if (STATUS_SUCCESS != Status)
					{
						KdPrint(("[OPENSSLDRV]: #DeviceIoControl#-->Device set port range error :[%d - %d]!\n",
							pPortRange->uiLocalPortStart, pPortRange->uiLocalPortEnd));
					}
					else
					{
						KdPrint(("[OPENSSLDRV]: #DeviceIoControl#-->Device set port range successful!  range: [%d - %d]!\n",
							pPortRange->uiLocalPortStart, pPortRange->uiLocalPortEnd));
					}
				}
			}
			break;
		case DEVICE_IOCTL_GETPORTRANGE:
		{
			PDEVICE_IOCTL_PORTRANGE_S pPortRange = (PDEVICE_IOCTL_PORTRANGE_S)InputBuffer;
			if (OutputBuffer == NULL || OutputBufferLength < sizeof(DEVICE_IOCTL_PORTRANGE_S))
			{
				IoStatus->Status = STATUS_INVALID_PARAMETER;
				break;
			}
			else
			{
				Status = OpenSSLProxy_GetSrcPortRange(&pPortRange->uiLocalPortStart, &pPortRange->uiLocalPortEnd);
				if (STATUS_SUCCESS != Status)
				{
					KdPrint(("[OPENSSLDRV]: #DeviceIoControl#-->Device get port range error\n"));
				}
				else
				{
					KdPrint(("[OPENSSLDRV]: #DeviceIoControl#-->Device get port range successful!  range: [%d - %d]!\n",
						pPortRange->uiLocalPortStart, pPortRange->uiLocalPortEnd));
				}
			}
		}
		break;
		case DEVICE_IOCTL_SETLOCALPROXY:
			{
				PDEVICE_IOCTL_SETPROXYINFO pLocalProxyInfo = (PDEVICE_IOCTL_SETPROXYINFO)InputBuffer;

				if ( InputBuffer == NULL
					|| InputBufferLength != sizeof(DEVICE_IOCTL_SETPROXYINFO))
				{
					IoStatus->Status = STATUS_INVALID_PARAMETER;
				}
				else
				{
					Status = OpenSSLProxy_SetLocalProxyInfo(pLocalProxyInfo->uiPID, (USHORT)pLocalProxyInfo->uiTcpPort);
					if (STATUS_SUCCESS != Status)
					{
						(("[OPENSSLDRV]: #DeviceIoControl#-->Device set proxy Error , Pid=%d, Port=%d!\n", 
							pLocalProxyInfo->uiPID, pLocalProxyInfo->uiTcpPort));
					}
					else
					{
						KdPrint(("[OPENSSLDRV]: #DeviceIoControl#-->Device set proxy info successful! Pid=%d, Port=%d!\n",
							pLocalProxyInfo->uiPID, pLocalProxyInfo->uiTcpPort));
					}
				}
			}
			break;
		default:
				IoStatus->Status = STATUS_INVALID_PARAMETER;
			break;
	}

	UNREFERENCED_PARAMETER(OutputBuffer);
	UNREFERENCED_PARAMETER(OutputBufferLength);

	return IoStatus->Status;
}

NTSTATUS DriverControl(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP p_IRP)
{
	PIO_STACK_LOCATION l_IrpSp;
	NTSTATUS					 Status = STATUS_SUCCESS;

	l_IrpSp		= IoGetCurrentIrpStackLocation(p_IRP);
	
	if ( DeviceObject == gDeviceObject )
	{
		p_IRP->IoStatus.Status = STATUS_SUCCESS;
		p_IRP->IoStatus.Information = 0;

		//也可以做为Dispatch函数进行处理
		switch (l_IrpSp->MajorFunction)
		{
			case IRP_MJ_DEVICE_CONTROL:

				Status = DeviceIoControl(p_IRP->AssociatedIrp.SystemBuffer,
					l_IrpSp->Parameters.DeviceIoControl.InputBufferLength,
					p_IRP->AssociatedIrp.SystemBuffer,
					l_IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
					l_IrpSp->Parameters.DeviceIoControl.IoControlCode,
					&p_IRP->IoStatus);
				break;
			default:
				break;
		}
	}
	else
	{
		KdPrint(("[OPENSSLDRV]: #DriverControl#-->Not our device!\n"));
	}

	p_IRP->IoStatus.Status = Status;
	IoCompleteRequest(p_IRP, IO_NO_INCREMENT);
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
