#ifndef __NGE_CHARSET__H
#define __NGE_CHARSET__H

#include "nge_common.h"

#define NGE_RET_BUFFER_SMALL -1

#ifdef __cplusplus
extern "C"{
#endif

/*以下函数为内部使用,通常无需调用*/

/**
 *将GBK编码的字符转化为unicode(ucs-2)字符
 *@remark 注意out buffer需要预先分配足够大小.
 *@param[in] in 输入字符串
 *@param[out] out 转换后unicode字符保存在这里,注意需要预先分配足够大小
 *@param[in] len in字符串个数，以字节记，即strlen计算的个数
 *@param[in] n out buffer的大小，以字节记
 *@return int,转换后的字符个数，以uint16_t个数计，0与负数即为error
 */
	NGE_API int nge_charset_gbk_to_ucs2(const uint8_t* in, uint16_t* out, int len, int n);

/**
 *将utf8编码的字符转化为unicode(ucs-2)字符
 *@remark 注意out buffer需要预先分配足够大小.
 *@param[in] in 输入字符串
 *@param[out] out 转换后unicode字符保存在这里,注意需要预先分配足够大小
 *@param[in] len in字符串个数，以字节记，即strlen计算的个数
 *@param[in] n out buffer的大小，以字节记
 *@return int,转换后的字符个数，以uint16_t个数计，0与负数即为error
 */
	NGE_API int nge_charset_utf8_to_ucs2(const uint8_t* in, uint16_t* out, int len, int n);

/**
 *将utf8编码的字符转化为gbk字符
 *@remark 注意out buffer需要预先分配足够大小.
 *@param[in] in 输入字符串
 *@param[out] out 转换后gbk字符保存在这里,注意需要预先分配足够大小
 *@param[in] len in字符串个数，以字节记，即strlen计算的个数
 *@param[in] n out buffer的大小，以字节记
 *@return int,转换后的字符个数，以字节计，0与负数即为error
 */
	NGE_API int nge_charsets_utf8_to_gbk(const uint8_t* in, uint8_t* out, int len, int n);

/**
 *将gbk编码的字符转化为utf8字符
 *@remark 注意out buffer需要预先分配足够大小.
 *@param[in] in 输入字符串
 *@param[out] out 转换后gbk字符保存在这里,注意需要预先分配足够大小
 *@param[in] len in字符串个数，以字节记，即strlen计算的个数
 *@param[in] n out buffer的大小，以字节记
 *@return int,转换后的字符个数，以字节计，0与负数即为error
 */
	NGE_API int nge_charsets_gbk_to_utf8(const uint8_t* in, uint8_t* out, int len, int n);

#ifdef __cplusplus
}
#endif

#endif
