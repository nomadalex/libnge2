#include "nge_sys_compatible.h"

#if defined WIN32 || defined IPHONEOS
//win32
#include <time.h>
int sys_localtime(sys_time_p stm)
{
	time_t t = time(NULL);
	struct tm *pt = localtime(&t);
	if(stm){
		stm->year  = pt->tm_year+1900;
		stm->month = pt->tm_mon+1;
		stm->day   = pt->tm_mday;
		stm->hour  = pt->tm_hour;
		stm->minutes = pt->tm_min;
		stm->seconds = pt->tm_sec;
		stm->microseconds = 0;
		return 1;
	}
	return 0;
}

#else
//psp
int sys_localtime(sys_time_p stm)
{
	if(stm)
		return sceRtcGetCurrentClockLocalTime(stm);
	else
		return 0;
}
#endif


