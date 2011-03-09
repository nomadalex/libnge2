/* @(#)utils.h
 */

#ifndef _UTILS_H
#define _UTILS_H 1
#ifdef __cplusplus
extern "C" {
#endif

//生成一个min---max的浮点数
float rand_float(float min, float max);
//生成一个(min,max)的整数
int rand_int(int min, int max);

#ifdef __cplusplus
	}
#endif
#endif /* _UTILS_H */
