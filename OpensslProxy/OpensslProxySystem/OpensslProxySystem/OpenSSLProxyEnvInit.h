#pragma once


NTSTATUS	OpenSSLProxy_EnvInit();

VOID			OpenSSLProxy_EnvUnInit();

IN_ADDR		*OpenSSLProxy_GetLocalSockaddr();