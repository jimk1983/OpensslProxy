// OpensslProxyClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock2.h>
#include <Windows.h>
#include <Ws2tcpip.h>
#include <openssl\ssl.h>
#include <openssl\err.h>  

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

#define		MAXBUFSIZE		2048
#define		IPADDRLEN		32

void DisplayPeerSslInfo(SSL * ssl)
{
	X509	*cert;
	char		*line;

	cert = SSL_get_peer_certificate(ssl);
	if (cert != NULL) {
		printf("数字证书信息:\n");
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		printf("证书: %s\n", line);
		//free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		printf("颁发者: %s\n", line);
		//free(line);
		X509_free(cert);
	}
	else
		printf("无证书信息！\n");

	printf("OpenSSL1.1.1 info:\n");
	printf("SSL CipherName    =[%s]\n", SSL_get_cipher_name(ssl));
	printf("SSL VersionName   =[%s]\n", SSL_get_version(ssl));

}


int main(int argc, char *argv[])
{
	WSADATA		wsaData;
	WORD		wVersionRequested = 0;
	SSL_CTX	*pstSSlCtx = NULL;
	SSL			*pstSSL = NULL;
	struct sockaddr_in serveraddr = { 0 };
	struct in_addr		 ipaddr = { 0 };
	char			acBuf[MAXBUFSIZE] = { 0 };
	char			acSevAddr[IPADDRLEN] = { 0 };
	int			sockfd = 0;
	int			iError = 0;
	int			iError2 = 0;
	int			argPort = 0;
	int			iLen = 0;
	int			iSendLen = 0;
	int			iRecvLen = 0;
	int			iCount = 0;

	if (argc < 2)
	{
		printf("OpenSSL 1.1.1 Client UseAge: *.exe serveraddr serverport\n");
		return -1;
	}
	else
	{
		strcpy_s(acSevAddr, IPADDRLEN, argv[1]);
		argPort = atoi(argv[2]);
		printf("OpenSSL1.1.1 info: %s:%d\n", acSevAddr, argPort);
	}

	/*初始化winsock32环境*/
	wVersionRequested = MAKEWORD(2, 2);
	iError = WSAStartup(wVersionRequested, &wsaData);
	if (iError != 0) 
	{
		printf("WSAStartup error!\n");
		return NULL;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		return NULL;
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

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("[SSL-Client]: Error-->socket error=%08x!\n", GetLastError());
		goto ErrorExit;
	}

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(argPort);
	inet_pton(AF_INET, acSevAddr, &ipaddr);
	serveraddr.sin_addr = ipaddr;

	if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) != 0) {
		printf("[SSL-Client]: Error-->connect error!\n");
		goto ErrorExit;
	}

	pstSSlCtx = SSL_CTX_new(SSLv23_client_method());
	if (NULL == pstSSlCtx)
	{
		printf("[SSL-Client]: Error-->SSL_CTX_new error!\n");
		goto ErrorExit;
	}

	// 将连接付给SSL
	pstSSL = SSL_new(pstSSlCtx);
	if (pstSSL == NULL)
	{
		printf("[SSL-Client]: Error-->connect error!\n");
		goto ErrorExit;
	}

	SSL_set_fd(pstSSL, sockfd);

	iError = SSL_connect(pstSSL);
	if (iError == -1)
	{
		iError2 = SSL_get_error(pstSSL, iError);
		const char* p1 = SSL_state_string(pstSSL);
	}

	DisplayPeerSslInfo(pstSSL);

	//释放资源
	memset(acBuf, 0, MAXBUFSIZE);
	iLen += sprintf_s(acBuf, MAXBUFSIZE-1, "This is from Client Message, count=[%d]!", iCount);
	
	iSendLen = SSL_write(pstSSL, acBuf, iLen);
	if (iSendLen < 0 )
	{
		iError2 = SSL_get_error(pstSSL, iError);
		printf("[SSL-Client]: Error-->SSL write  ssl error=%d\n", iError2);
		goto ErrorExit;
	}

	while (1)
	{
		memset(acBuf, 0, MAXBUFSIZE);
		iRecvLen = SSL_read(pstSSL,  acBuf,  MAXBUFSIZE);
		if (iRecvLen <=0)
		{
			iError2 = SSL_get_error(pstSSL, iRecvLen);
			printf("[SSL-Client]: Error-->SSL_read  error=%d\n", iError2);
			break;
		}
		else
		{
			printf("[SSL-Client]: SSL_read Content=%s\n", acBuf);
		}

		memset(acBuf, 0, MAXBUFSIZE);
		iCount++;
		iLen = 0;
		iLen += sprintf_s(acBuf, MAXBUFSIZE - 1, "This is from Client Message, count=[%d]!", iCount);
		iSendLen = SSL_write(pstSSL, acBuf,  iLen);
		if (iSendLen < 0)
		{
			iError2 = SSL_get_error(pstSSL, iSendLen);
			printf("[SSL-Client]: Error-->SSL write  error=%d\n", iError2);
			break;
		}
		Sleep(1000);
	}
	
	system("pause");

ErrorExit:
	if (sockfd > 0 )
	{
		closesocket(sockfd);
	}

	if (NULL != pstSSL)
	{
		SSL_free(pstSSL);
		pstSSL = NULL;
	}

	if (NULL != pstSSlCtx )
	{
		SSL_CTX_free(pstSSlCtx);
		pstSSlCtx = NULL;
	}

	WSACleanup();
	return -1;
}

