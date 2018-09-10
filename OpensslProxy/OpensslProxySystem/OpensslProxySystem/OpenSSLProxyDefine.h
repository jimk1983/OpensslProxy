#pragma once

#define htonl(l)                  \
   ((((l) & 0xFF000000) >> 24) | \
   (((l) & 0x00FF0000) >> 8)  |  \
   (((l) & 0x0000FF00) << 8)  |  \
   (((l) & 0x000000FF) << 24))

#define htons(s) \
   ((((s) >> 8) & 0x00FF) | \
   (((s) << 8) & 0xFF00))

#define ntohl(l)                   \
   ((((l) >> 24) & 0x000000FFL) | \
   (((l) >>  8) & 0x0000FF00L) |  \
   (((l) <<  8) & 0x00FF0000L) |  \
   (((l) << 24) & 0xFF000000L))

#define ntohs(s)                     \
   ((USHORT)((((s) & 0x00ff) << 8) | \
   (((s) & 0xff00) >> 8)))


#define			OPENSSLPROXYDRV						L"\\Device\\OpenSSLProxyDriver"
#define			OPENSSLPROXYDOSDRV				L"\\??\\OpenSSLProxyDriver"


typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT  pDevice;
	UNICODE_STRING ustrDeviceName;
	UNICODE_STRING ustrSymLinkName;
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;













