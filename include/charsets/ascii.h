/*
 * Copyright (C) 1999-2001 Free Software Foundation, Inc.
 * Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 * This file is under GPL or LGPL License.
 */

/*
 * ASCII
 */

inline static int
ascii_mbtowc (ucs4_t *pwc, const unsigned char *s, int n)
{
  unsigned char c = *s;
  if (c < 0x80) {
    *pwc = (ucs4_t) c;
    return 1;
  }
  return RET_ILSEQ;
}

inline static int
ascii_wctomb (unsigned char *r, ucs4_t wc, int n)
{
  if (wc < 0x0080) {
    *r = wc;
    return 1;
  }
  return RET_ILUNI;
}
