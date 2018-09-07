#pragma once

#define			OPENSSLPROXYDRV					L"\\Device\\OpenSSLProxyDriver"
#define			OPENSSLPROXYDOSDRV				 L"\\??\\OpenSSLProxyDriver"


typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT  pDevice;
	UNICODE_STRING ustrDeviceName;
	UNICODE_STRING ustrSymLinkName;
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;













