#include "nge_common.h"
#include "nge_debug_log.h"
#include "nge_sys_compatible.h"
#include <stdio.h>
#include <string.h>

static int cpu_type = CPU_TYPE_222MHZ;

#ifdef NGE_PSP
#include <psppower.h>
//psp
int sys_localtime(sys_time_p stm)
{
	if(stm)
		return sceRtcGetCurrentClockLocalTime(stm);
	else
		return 0;
}

char* sys_get_idstorage()
{
	static char ret[32] = {0};
	unsigned char buffer[0x1e]; /* Buffer */
	/*sceIdStorageLookup(0x050, 0x0, buffer, 0x1e);
	sprintf(ret,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
		buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5],
		buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11],
		buffer[12], buffer[13], buffer[14], buffer[15], buffer[16], buffer[17],
		buffer[18], buffer[19], buffer[20], buffer[21], buffer[22], buffer[23],
		buffer[24], buffer[25], buffer[26], buffer[27], buffer[28], buffer[29]);*/
	return ret;
}

char* sys_get_mac()
{
	static char ret[24]={0};
	unsigned char sVal[8]= {0};
	int retVal;
	//retVal = sceWlanGetEtherAddr(sVal);
	if (retVal == 0)
		sprintf(ret,"%02X:%02X:%02X:%02X:%02X:%02X", sVal[0], sVal[1], sVal[2], sVal[3], sVal[4], sVal[5]);
	return ret;
}

char* sys_get_nickname()
{
	static char ret[64]={0};
	/*if(sceUtilityGetSystemParamString(PSP_SYSTEMPARAM_ID_STRING_NICKNAME, ret, 64) == PSP_SYSTEMPARAM_RETVAL_FAIL)
		strncpy(ret,"NGE2",4);*/
	return ret;
}

int sys_set_frequency_type(int type)
{
	int ret = -1;
	switch(type)
	{
	case CPU_TYPE_333MHZ:
		ret = scePowerSetClockFrequency(333, 333, 166);
		break;
	case CPU_TYPE_222MHZ:
		ret = scePowerSetClockFrequency(222, 222, 111);
		break;
	case CPU_TYPE_111MHZ:
		ret = scePowerSetClockFrequency(111, 111, 66);
		break;
	default:
		break;
	}
	if(ret == 0){
		cpu_type = type;
		return 1;
	}
	return 0;
}

int sys_get_frequency_type()
{
	return cpu_type;
}
#else
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

//set 'F'
char* sys_get_idstorage()
{
	static char ret[32] = {0};
	memset(ret,'F',30);
	return ret;
}

char* sys_get_mac()
{
	static char ret[24]={0};
	unsigned char sVal[8]= {0};
	memset(sVal,0xff,8);
  	sprintf(ret,"%02X:%02X:%02X:%02X:%02X:%02X", sVal[0], sVal[1], sVal[2], sVal[3], sVal[4], sVal[5]);
	return ret;
}

char* sys_get_nickname()
{
	static char ret[64]={"NGE2"};
	return ret;
}

int sys_set_frequency_type(int type)
{
	printf("switch cpu to %d mhz\n",type);
	cpu_type = type;
	return 1;
}

int sys_get_frequency_type()
{
	return cpu_type;
}
#endif
