#pragma once

extern BOOLEAN						gDriverUnloading;
extern PDEVICE_OBJECT			gDeviceObject;
extern HANDLE						gEngineHandle;

NTSTATUS DriverControl(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP p_IRP
);

