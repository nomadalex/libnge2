// charset.cpp: implementation of the charset class.
//
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "nge_debug_log.h"
#include "nge_charsets.h"
#include <stdlib.h>
#include <string.h>

/* definitions */

typedef uint32 ucs4_t;
/* Return code if invalid. (xxx_mbtowc) */
#define RET_ILSEQ      -1
/* Return code if only a shift sequence of n bytes was read. (xxx_mbtowc) */
#define RET_TOOFEW(n)  (-2-(n))
/* Return code if invalid. (xxx_wctomb) */
#define RET_ILUNI      -1
/* Return code if output buffer is too small. (xxx_wctomb) */
#define RET_TOOSMALL   -2

typedef struct {
  uint16 indx; /* index into big table */
  uint16 used; /* bitmask of used entries */
} Summary16;

#include "ascii.h"
#include "utf8.h"
#include "gbk.h"
#include "ces_gbk.h"

int nge_charset_gbk_to_ucs2(const uint8* in, uint8* out, int len, int n) {
	ucs4_t wc = 0;
	int cur = 0, ret;
	uint16 *pOut = out, *pEnd = out+n/2;

	while (cur < len) {
		ret = ces_gbk_mbtowc(&wc, in+cur, len-cur);
		if (ret < 0 || wc > 0xffff)
			return 0;
		*pOut = (uint16)wc;
		cur += ret;
		pOut++;
		if (pOut > pEnd)
			return NGE_RET_BUFFER_SMALL;
	}
	*pOut = 0x0;
	return pOut - (uint16*)out;
}

int nge_charset_utf8_to_ucs2(const uint8* in, uint16* out, int len, int n) {
	ucs4_t wc = 0;
	int cur = 0, ret;
	uint16 *pOut = out, *pEnd = out+n/2;

	while (cur < len) {
		ret = utf8_mbtowc(&wc, in+cur, len-cur);
		if (ret < 0 || wc > 0xffff)
			return 0;
		*pOut = (uint16)wc;
		cur += ret;
		pOut++;
		if (pOut > pEnd)
			return NGE_RET_BUFFER_SMALL;
	}
	*pOut = 0x0;
	return pOut - (uint16*)out;
}

int nge_charsets_utf8_to_gbk(const uint8* in, uint8* out, int len, int n) {
	ucs4_t wc = 0;
	int cur = 0, ret;
	uint8 *pOut = out, *pEnd = out+n;

	while (cur < len) {
		ret = utf8_mbtowc(&wc, in+cur, len-cur);
		if (ret < 0)
			return 0;
		cur += ret;

		ret = ces_gbk_wctomb(pOut, wc, n);
		if (ret < 0)
			return 0;
		pOut += ret;
		n -= ret;

		if (pOut > pEnd)
			return NGE_RET_BUFFER_SMALL;
	}
	*pOut = 0x0;
	return pOut - out - 1;
}
