/***************************************************************************
 *            nge_common.h
 *
 *  2011/03/25 06:23:30
 *  Copyright  2011  Kun Wang <ifreedom.cn@gmail.com>
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _NGE_COMMON_H
#define _NGE_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NDEBUG
#ifdef MMGR
//for debug -- mmgr can use in ANSI C ...
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "mmgr.h"
#endif
#endif

#ifndef uint32
#define uint32 unsigned int
#endif

#ifndef sint32
#define sint32 int
#endif

#ifndef uint16
#define uint16 unsigned short
#endif

#ifndef sint16
#define sint16 short
#endif

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef sint8
#define sint8  char
#endif

#ifndef BOOL
#define BOOL uint8
#endif

#define SAFE_FREE(ptr) free(ptr)

#ifdef __cplusplus
}
#endif

#endif /* _NGE_COMMON_H */
