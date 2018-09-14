
typedef HANDLE SYS_SEM_T;

SYS_SEM_T	SYS_Sem_Create(UINT8 uiCount);

VOID		SYS_Sem_Free(SYS_SEM_T hSem);

VOID		SYS_Sem_Signal(SYS_SEM_T hSem);

UINT32		SYS_Sem_Wait(SYS_SEM_T hSem, UINT32 uiTimeout);