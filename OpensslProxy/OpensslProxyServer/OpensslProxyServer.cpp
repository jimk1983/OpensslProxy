// OpensslProxyServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <openssl\ssl.h>

#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")



int main()
{
	WSADATA    wsaData;
	WORD	         wVersionRequested = 0;
	int                 iError			= 0;
	SSL_CTX		*pstSSlCtx	= NULL;
	SSL				*pstSSL		= NULL;




	/*初始化winsock32环境*/
	wVersionRequested = MAKEWORD(2, 2);
	iError = WSAStartup(wVersionRequested, &wsaData);
	if (iError != 0) {
		return NULL;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		return NULL;
	}

	/* SSL 库初始化*/
	SSL_library_init();
	SSLeay_add_ssl_algorithms();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	ERR_load_BIO_strings();

	pstSSlCtx = SSL_CTX_new(TLSv1_2_server_method());
	if (NULL == pstSSlCtx)
	{
		printf("[SSL-Client]: Error-->SSL_CTX_new error!\n");
		goto ErrorExit;
	}


	return 0;

ErrorExit:
	WSACleanup();

	if (NULL != pstSSlCtx)
	{
		SSL_CTX_free(pstSSlCtx);
		pstSSlCtx = NULL;
	}

	if (NULL != pstSSL)
	{
		SSL_free(pstSSL);
		pstSSL = NULL;
	}

    return -1;
}

