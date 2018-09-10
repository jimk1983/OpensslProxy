#pragma once

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

extern BOOLEAN						gDriverUnloading;
extern PDEVICE_OBJECT			gDeviceObject;
extern HANDLE						gEngineHandle;
extern BOOLEAN						gConnectionRedirectEnable;
extern HANDLE						g_hRedirectHandle;

NTSTATUS OpenSSLProxy_RegisterCallouts(
	IN void* deviceObject);

VOID OpenSSLProxy_UnRegisterCallouts();

NTSTATUS DriverCreate(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP p_IRP);

NTSTATUS DriverControl(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP p_IRP);

NTSTATUS DriverCompeleteRequest(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP p_IRP);

