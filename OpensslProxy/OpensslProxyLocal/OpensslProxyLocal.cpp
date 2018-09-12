// OpensslProxyLocal.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../OpensslProxyDrvCtrl/DrvCtrlApi.h"

#pragma comment(lib, "OpensslProxyDrvCtrl.lib")

int main()
{
	if ( FALSE == OpenSSLProxy_EnvLibInit())
	{
		return -1;
	}

	OpenSSLProxy_EnvLibUnInit();

    return 0;
}

