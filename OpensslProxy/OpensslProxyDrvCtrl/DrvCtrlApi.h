#ifndef  _DRVCTRLAPI_H_
#define _DRVCTRLAPI_H_

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOLEAN		OpenSSLProxy_DrvCtrl_EnvLibInit();

VOID				OpenSSLProxy_DrvCtrl_EnvLibUnInit();

BOOLEAN		OpenSSLProxy_DrvCtrl_RuleMatchEnable();

BOOLEAN		OpenSSLProxy_DrvCtrl_RuleMatchDisable();

#ifdef __cplusplus
}
#endif

#endif
