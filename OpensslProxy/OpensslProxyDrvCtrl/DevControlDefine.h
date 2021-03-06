#pragma once

#define DEVICE_CONTROL_CODE(request,method) \
		CTL_CODE (FILE_DEVICE_UNKNOWN, request, method, FILE_ANY_ACCESS)

/*使能开关*/
#define DEVICE_IOCTL_MATCHENABLE					DEVICE_CONTROL_CODE (0x01, METHOD_BUFFERED)
/*去使能开关*/
#define DEVICE_IOCTL_MATCHDISABLE					DEVICE_CONTROL_CODE (0x02, METHOD_BUFFERED)
/*设置地址端口*/
#define DEVICE_IOCTL_SETRULEIPPORT   				DEVICE_CONTROL_CODE (0x03, METHOD_BUFFERED)
/*删除地址端口*/
#define DEVICE_IOCTL_DELRULEIPPORT   				DEVICE_CONTROL_CODE (0x04, METHOD_BUFFERED)
/*设置本地信息*/
#define DEVICE_IOCTL_SETLOCALPROXY				DEVICE_CONTROL_CODE (0x05, METHOD_BUFFERED)
/*清除所有规则*/
#define DEVICE_IOCTL_SETRULECLEAR					DEVICE_CONTROL_CODE (0x06, METHOD_BUFFERED)
/*按照类型清除*/
#define DEVICE_IOCTL_SETRULETYPECLEAR			DEVICE_CONTROL_CODE (0x07, METHOD_BUFFERED)
/*设置端口范围*/
#define DEVICE_IOCTL_SETPORTRANGE				DEVICE_CONTROL_CODE (0x08, METHOD_BUFFERED)
/*获取端口范围*/
#define DEVICE_IOCTL_GETPORTRANGE				DEVICE_CONTROL_CODE (0x09, METHOD_BUFFERED)

typedef struct _DEVICE_IOCTL_RULEINFO
{
	UINT32			uiRuleIPAddr;
	UINT32			uiRulePort;
}DEVICE_IOCTL_RULEINFO, *PDEVICE_IOCTL_RULEINFO;

typedef struct _DEVICE_IOCTL_PORTRANGEINFO
{
	UINT32			uiLocalPortStart;
	UINT32			uiLocalPortEnd;
}DEVICE_IOCTL_PORTRANGE_S, *PDEVICE_IOCTL_PORTRANGE_S;

typedef struct _DEVICE_IOCTL_SETPROXYINFO
{
	UINT32			uiPID;
	UINT32 			uiTcpPort;
}DEVICE_IOCTL_SETPROXYINFO, *PDEVICE_IOCTL_SETPROXYINFO;



