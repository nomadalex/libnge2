#ifndef NGE_MISC_H_
#define NGE_MISC_H_

#include "nge_common.h"

//休眠毫秒数
#if defined WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define nge_sleep(tick) Sleep(tick)

#elif defined _PSP
#include <pspthreadman.h>
#define nge_sleep(tick) sceKernelDelayThread((tick)*1000)

#elif defined __linux__
#include <unistd.h>
#define nge_sleep(tick) usleep(tick)
#endif

#ifdef __cplusplus
extern "C"{
#endif

/**
 *生成一个min---max的浮点数
 *@param[in] min,最小值
 *@param[in] max,最大值
 *@return float,生成的浮点数
 */
	NGE_API float rand_float(float min, float max);
/**
 *生成一个(min,max)的整数
 *@param[in] min,最小值
 *@param[in] max,最大值
 *@return int,生成的整数
 */
	NGE_API int rand_int(int min, int max);

#ifdef __cplusplus
}
#endif

#endif
