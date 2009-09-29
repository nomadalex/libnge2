#ifndef NGE_SYS_COMPATIBLE_H_
#define NGE_SYS_COMPATIBLE_H_
#include "nge_define.h"
#if defined WIN32 || defined IPHONEOS
typedef struct st_tm{
	uint16  year;
	uint16  month; 
	uint16  day; 
	uint16  hour; 
	uint16  minutes; 
	uint16  seconds; 
	uint32  microseconds; 
}sys_time_t,*sys_time_p;
#else
//psp
#include <psprtc.h>
typedef pspTime  sys_time_t;
typedef pspTime*  sys_time_p;
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

#ifdef __cplusplus
}
#endif


#endif