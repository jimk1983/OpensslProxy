
#include <Winsock2.h>  
#include <Windows.h>
#include <WS2tcpip.h>
#include <mswsock.h>
#include <stdio.h>
#include <process.h> 

#include "../common/CLog.h"
#include "../common/CommDef.h"
#include "../common/CommBizDefine.h"

#pragma comment(lib,"Ole32.Lib")

VOID OpensslProxy_utils_GenUniqueID(CHAR *pcGuID)
{
	GUID guid;
	char buffer[MGR_GUIDLEN] = { 0 };

	if ( CoCreateGuid(&guid))
	{
		fprintf(stderr, "create guid error\n");
		return;
	}
	else
	{
		_snprintf_s(buffer, MGR_GUIDLEN,
			"%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2],
			guid.Data4[3], guid.Data4[4], guid.Data4[5],
			guid.Data4[6], guid.Data4[7]);

		strcpy_s(pcGuID, MGR_GUIDLEN, buffer);
	}

	return;
}
