#pragma once

typedef struct tagResourceLock
{
	ERESOURCE					stLockResource;					//资源锁
	KSPIN_LOCK				stLockKSpin;						//irql太高时使用
}RESOURCE_LOCK_S, *PRESOURCE_LOCK_S;


VOID	LocalHostIPStringToSockAddr(IN_ADDR *pstInAddr);

VOID	OpenSSLProxy_ResourceLock(PRESOURCE_LOCK_S pLock, PKIRQL OldIrql);

VOID	OpenSSLProxy_ResourceUnLock(PRESOURCE_LOCK_S pLock, KIRQL OldIrql);

