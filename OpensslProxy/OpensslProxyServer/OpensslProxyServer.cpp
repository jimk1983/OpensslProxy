// OpensslProxyServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock2.h>
#include <Windows.h>
#include <Ws2tcpip.h>
#include <openssl\ssl.h>
#include <openssl/err.h>  

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

#define		LISTNNUM			10
#define		MAXBUFSIZE		2048
#define		IPADDRLEN			32


int main(int argc, char *argv[])
{
	WSADATA     wsaData;
	WORD	         wVersionRequested = 0;
	SSL_CTX		*pstSSlCtx	= NULL;
	SSL				*pstSSL		= NULL;
	struct  sockaddr_in  serveraddr = {0};
	struct  sockaddr_in  clientaddr  = {0};
	socklen_t					socklen=0;
	struct  in_addr		    ipaddr = { 0 };
	char			acBuf[MAXBUFSIZE] = { 0 };
	char			acSevAddr[IPADDRLEN] = { 0 };
	char			acCliAddr[IPADDRLEN] = { 0 };
	int			iError = 0;
	int			iError2 = 0;
	int			argPort = 0;
	int			iLen = 0;
	int			iSendLen = 0;
	int			iRecvLen = 0;
	int			iCount = 0;
	int			sockfd = 0;
	int			new_fd = 0;

	/*初始化winsock32环境*/
	wVersionRequested = MAKEWORD(2, 2);
	iError = WSAStartup(wVersionRequested, &wsaData);
	if (iError != 0) {
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		return -1;
	}

	if (argc < 2)
	{
		printf("****OpenSSL 1.1.1 Server UseAge: *.exe Localaddr  port*****\n");
		return -1;
	}
	else
	{
		strcpy_s(acSevAddr, IPADDRLEN, argv[1]);
		argPort = atoi(argv[2]);
		printf("OpenSSL1.1.1 info: %s:%d\n", acSevAddr, argPort);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("[SSL-Server]: Error-->socket error=%08x!\n", GetLastError());
		goto ErrorExit;
	}

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(argPort);
	inet_pton(AF_INET, acSevAddr, &ipaddr);
	serveraddr.sin_addr = ipaddr;
	socklen = sizeof(serveraddr);

	if ( bind(sockfd, (struct sockaddr *)&serveraddr, socklen) < 0 )
	{
		printf("[SSL-Server]: Error-->socket bind error=%08x!\n", GetLastError());
		goto ErrorExit;
	}

	if ( listen(sockfd, LISTNNUM)  < 0 ) 
	{
		printf("[SSL-Server]: Error-->socket listen error=%08x!\n", GetLastError());
		goto ErrorExit;
	}

	/* SSL 库初始化*/
	SSL_library_init();
	OPENSSL_malloc_init();
	SSLeay_add_ssl_algorithms();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	ERR_load_BIO_strings();

	printf("OpenSSL1.1.1 info:\n");
	printf("%s\n", SSLeay_version(OPENSSL_VERSION));
	printf("%s\n", SSLeay_version(OPENSSL_CFLAGS));
	printf("%s\n", SSLeay_version(OPENSSL_BUILT_ON));
	printf("%s\n", SSLeay_version(OPENSSL_PLATFORM));
	printf("%s\n", SSLeay_version(OPENSSL_DIR));
	printf("%s\n", SSLeay_version(OPENSSL_ENGINES_DIR));

	pstSSlCtx = SSL_CTX_new(SSLv23_server_method());
	if (NULL == pstSSlCtx)
	{
		printf("[SSL-Server]: Error-->SSL_CTX_new error!\n");
		goto ErrorExit;
	}

	if (SSL_CTX_set_cipher_list(pstSSlCtx, "TLS_AES_256_GCM_SHA384") <= 0)
	{
		ERR_print_errors_fp(stdout);
		goto ErrorExit;
	}

	if (SSL_CTX_use_certificate_file(pstSSlCtx, argv[4], SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stdout);
		goto ErrorExit;
	}
	if (SSL_CTX_use_PrivateKey_file(pstSSlCtx, argv[5], SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stdout);
		goto ErrorExit;
	}
	if (!SSL_CTX_check_private_key(pstSSlCtx)) {
		ERR_print_errors_fp(stdout);
		goto ErrorExit;
	}

	while (1) 
	{
		iLen = sizeof(struct sockaddr);

		if ((new_fd = accept(sockfd, (struct sockaddr *)&clientaddr, &iLen)) == -1) 
		{
			printf("[SSL-Server]: Error-->socket listen error=%08x!\n", GetLastError());
			break;
		}
		else
		{
			memset(acCliAddr,0, IPADDRLEN); 
			inet_ntop(AF_INET, &clientaddr.sin_addr, acCliAddr, IPADDRLEN);
			printf("server: got connection from %s, port %d, socket %d \n", acCliAddr, ntohs(clientaddr.sin_port), new_fd);
		}
		
		pstSSL = SSL_new(pstSSlCtx);
		if (NULL == pstSSL)
		{
			ERR_print_errors_fp(stdout);
			break;
		}

		SSL_set_fd(pstSSL, new_fd);
		iError = SSL_accept(pstSSL);
		if ( iError == -1)
		{
			ERR_print_errors_fp(stdout);
			break;
		}

		/*单次仅处理一个客户端的报文*/
		while (1)
		{
			memset(acBuf, 0, MAXBUFSIZE);
			iRecvLen = SSL_read(pstSSL, acBuf, MAXBUFSIZE);
			if (iRecvLen <= 0)
			{
				iError2 = SSL_get_error(pstSSL, iRecvLen);
				printf("[SSL-Server]: Error-->SSL_read  error=%d\n", iError2);
				break;
			}
			else
			{
				printf("[SSL-Server]: SSL_read Content=%s\n", acBuf);
			}

			memset(acBuf, 0, MAXBUFSIZE);
			iCount++;
			iLen = 0;
			iLen += sprintf_s(acBuf, MAXBUFSIZE - 1, "Server BackMessage, ServerHello, count=[%d]!", iCount);
			iSendLen = SSL_write(pstSSL, acBuf, iLen);
			if (iSendLen < 0)
			{
				iError2 = SSL_get_error(pstSSL, iSendLen);
				printf("[SSL-Server]: Error-->SSL write  error=%d\n", iError2);
				break;
			}
		}
		/*先释放上一次的SSL*/
		if (NULL != pstSSL)
		{
			SSL_free(pstSSL);
			pstSSL = NULL;
		}
	}

ErrorExit:
	if (sockfd > 0)
	{
		closesocket(sockfd);
	}

	if ( new_fd > 0 )
	{
		closesocket(new_fd);
	}
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

	WSACleanup();
    return -1;
}

