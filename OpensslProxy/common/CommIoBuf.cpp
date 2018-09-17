#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include "CommDef.h"
#include "CommIoBuf.h"
#include "sem.h"
#include "queue.h"


COM_IOBUF* COMM_IOBUF_Create()
{
	COM_IOBUF*	pstIoBuf = NULL;

	pstIoBuf = (COM_IOBUF *)malloc(sizeof(COM_IOBUF));
	if (NULL == pstIoBuf)
	{
		return NULL;
	}
	memset(pstIoBuf, 0, sizeof(COM_IOBUF));

	InitializeListHead(&pstIoBuf->stNode);
	pstIoBuf->uiBufSize		= IOBUF_MAXSIZE;
	pstIoBuf->uiDatalen		= 0;

	return pstIoBuf;
}

VOID COMM_IOBUF_Free(COM_IOBUF* pstIoBuf)
{
	if (NULL != pstIoBuf)
	{
		free(pstIoBuf);
	}
}
