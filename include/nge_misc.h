#ifndef NGE_MISC_H_
#define NGE_MISC_H_

//休眠毫秒数
#if defined WIN32
#include "SDL.h"
#define nge_sleep(tick) SDL_Delay(tick)

#elif defined _PSP
#include <pspthreadman.h>
#define nge_sleep(tick) sceKernelDelayThread((tick)*1000)

#elif defined IPHONEOS || defined(__linux__)
#include <stdlib.h>
#define nge_sleep(tick) usleep(tick)
#endif

#ifdef __cplusplus
extern "C"{
#endif

//生成一个min---max的浮点数
float rand_float(float min, float max);
//生成一个(min,max)的整数
int rand_int(int min, int max);

/**
 *将GBK编码的字符转化为unicode字符
 *注意outbuffer需要预先分配足够大小.
 *@param unsigned short* out,转换后unicode字符保存在这里,注意需要预先分配足够大小
 *@param const char* in,输入字符缓存
 *@param int n,GBK字符个数以字节记,strlen计算的个数(例如一个汉字为2个字符)
 *@return int n,转换的unicode字符个数
 */
int gbk_to_unicode(unsigned short* out,const char* in,int n);

#ifdef __cplusplus
}
#endif

#endif
