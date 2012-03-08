/*
 * Copyright (C) 1999-2001, 2005 Free Software Foundation, Inc.
 * Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 * This file is under GPL or LGPL License.
 */

/*
 * GBK
 */

inline static int
ces_gbk_mbtowc (ucs4_t *pwc, const unsigned char *s, int n)
{
  unsigned char c = *s;

  /* Code set 0 (ASCII or GB 1988-89) */
  if (c < 0x80)
    return ascii_mbtowc(pwc,s,n);
  /* Code set 1 (GBK) */
  if (c >= 0x81 && c < 0xff) {
    if (n < 2)
      return RET_TOOFEW(0);
    return gbk_mbtowc(pwc,s,2);
  }
  return RET_ILSEQ;
}

inline static int
ces_gbk_wctomb (unsigned char *r, ucs4_t wc, int n)
{
  unsigned char buf[2];
  int ret;

  /* Code set 0 (ASCII or GB 1988-89) */
  ret = ascii_wctomb(r,wc,n);
  if (ret != RET_ILUNI)
    return ret;

  /* Code set 1 (GBK) */
  ret = gbk_wctomb(buf,wc,2);
  if (ret != RET_ILUNI) {
    if (ret != 2) abort();
    if (n < 2)
      return RET_TOOSMALL;
    r[0] = buf[0];
    r[1] = buf[1];
    return 2;
  }

  return RET_ILUNI;
}
