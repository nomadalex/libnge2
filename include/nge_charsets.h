#ifndef __NGE_CHARSET__H
#define __NGE_CHARSET__H

#define NGE_RET_BUFFER_SMALL -1

#ifdef __cplusplus
extern "C"{
#endif

/*以下函数为内部使用,通常无需调用*/

/**
 *将GBK编码的字符转化为unicode(ucs-2)字符
 *注意out buffer需要预先分配足够大小.
 *@param const uint8* in,输入字符串
 *@param uint8* out,转换后unicode字符保存在这里,注意需要预先分配足够大小
 *@param int len,in字符串个数，以字节记，即strlen计算的个数
 *@param int n,out buffer的大小，以字节记
 *@return int,转换后的字符个数，以uint16个数计，0与负数即为error
 */
	int nge_charset_gbk_to_ucs2(const uint8* in, uint16* out, int len, int n);

/**
 *将utf8编码的字符转化为unicode(ucs-2)字符
 *注意out buffer需要预先分配足够大小.
 *@param const uint8* in,输入字符串
 *@param uint8* out,转换后unicode字符保存在这里,注意需要预先分配足够大小
 *@param int len,in字符串个数，以字节记，即strlen计算的个数
 *@param int n,out buffer的大小，以字节记
 *@return int,转换后的字符个数，以uint16个数计，0与负数即为error
 */
	int nge_charset_utf8_to_ucs2(const uint8* in, uint16* out, int len, int n);

/**
 *将utf8编码的字符转化为gbk字符
 *注意out buffer需要预先分配足够大小.
 *@param const uint8* in,输入字符串
 *@param uint8* out,转换后unicode字符保存在这里,注意需要预先分配足够大小
 *@param int len,in字符串个数，以字节记，即strlen计算的个数
 *@param int n,out buffer的大小，以字节记
 *@return int,转换后的字符个数，以字节计，0与负数即为error
 */
	int nge_charsets_utf8_to_gbk(const uint8* in, uint8* out, int len, int n);

#ifdef __cplusplus
}
#endif

#endif
