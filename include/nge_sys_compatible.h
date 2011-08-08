#ifndef NGE_SYS_COMPATIBLE_H_
#define NGE_SYS_COMPATIBLE_H_

#include "nge_common.h"

#define CPU_TYPE_333MHZ 333
#define CPU_TYPE_222MHZ 222
#define CPU_TYPE_111MHZ 111

#ifdef NGE_PSP
#include <time.h>
#include <psptypes.h>
#include <psprtc.h>
typedef pspTime sys_time_t,*sys_time_p;
#else
#include "nge_common.h"
typedef struct st_tm{
	uint16  year;
	uint16  month;
	uint16  day;
	uint16  hour;
	uint16  minutes;
	uint16  seconds;
	uint32  microseconds;
}sys_time_t,*sys_time_p;
#endif

#ifdef __cplusplus
extern "C"{
#endif

/**
 *获取系统的本地时间
 *@param sys_time_p tm,系统时间结构
 *@return int,成功返回1,失败返回0
 */
int sys_localtime(sys_time_p tm);

/**
 *获取系统的唯一的30位序列号.
 *@return char*,30位序列号的字符串.
 */
char* sys_get_idstorage();

/**
 *获取系统的MAC地址字符串.
 *@return char*,返回字符串,XX:XX:XX:XX:XX:XX.
 */
char* sys_get_mac();

/**
 *获取系统的用户名.
 *@return char*,返回字符串.
 */
char* sys_get_nickname();

/**
 *设置CPU的频率类型.
 *@param int,类型为CPU_TYPE_333MHZ,CPU_TYPE_222MHZ,CPU_TYPE_111MHZ
 *@return int,1设置成功,0设置失败.
 */
int sys_set_frequency_type(int type);

/**
 *取得CPU的频率类型.
 *@return int,类型为CPU_TYPE_333MHZ,CPU_TYPE_222MHZ,CPU_TYPE_111MHZ
 */
int sys_get_frequency_type();

#ifdef __cplusplus
}
#endif

#endif
