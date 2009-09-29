#ifndef __NGE_CHARSET__H
#define __NGE_CHARSET__H

#include "nge_define.h"
#ifdef __cplusplus
extern "C"{
#endif
/*以下函数为内部使用,通常无需调用*/
uint32 charsets_ucs_conv(const uint8 *uni, uint8 *cjk);
void charsets_big5_conv(const uint8 *big5, uint8 *cjk);
void charsets_sjis_conv(const uint8 *jis, uint8 **cjk, uint32 * newsize);
uint32 charsets_utf8_conv(const uint8 *ucs, uint8 *cjk);
uint32 charsets_utf16_conv(const uint8 *ucs, uint8 *cjk);
uint32 charsets_utf16be_conv(const uint8 *ucs, uint8 *cjk);
uint16 charsets_gbk_to_ucs(const uint8 * cjk);

#ifdef __cplusplus
}
#endif

#endif
