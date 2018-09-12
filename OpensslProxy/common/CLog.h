
#ifdef __cplusplus
extern "C" {
#endif


#define CLOG_LEVEL_ERROR	1		/*错误级别日志: 主要添加Error: */
#define CLOG_LEVEL_WARNING	2		/*告警级别日志: 主要添加Warning:*/
#define CLOG_LEVEL_EVENT	3		/*正常打印日志*/
#define CLOG_LEVEL_DEBUG	4		/*调试日志*/

#define CLOG_LEVEL_STR_ERROR		"Error  :"	/*错误日志前面会有ERROR*/
#define CLOG_LEVEL_STR_WARING	"Warning:"
#define CLOG_LEVEL_STR_LEN		8			/*字符对齐，前面都空余8个长度*/


	/*日志目录名称*/
#define CLOG_DIRNAME	"CloudScreen\\log\\trasitservice"	

/*日志长度单条*/
#define CLOG_BUFLEN		1024

#ifndef SYS_ERR
#define SYS_ERR -1
#endif

#ifndef SYS_OK
#define SYS_OK 0
#endif

/*日志文件区分*/
typedef enum
{
	CLOG_TYPE_DEVCTRL = 0,	/**/

	CLOG_TYPE_NUMS
}CLOG_TYPE_E;

int		CLOG_evn_init(CLOG_TYPE_E eType);

void		CLOG_evn_uninit();

void		CLOG_writelog(char *pcModuleName, const char *fmt, ...);

void		CLOG_writelog_level(char *pcModuleName, int level, const char *fmt, ...);


#ifdef __cplusplus
}
#endif
