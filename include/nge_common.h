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

#define NGE_MAJOR_VERSION	2
#define NGE_MINOR_VERSION	0
#define NGE_PATCHLEVEL		0
#define NGE_VERSION "2.0.0"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL  ((void *)0)
#endif

#ifndef uint32
typedef unsigned int uint32;
#endif

#ifndef sint32
typedef int sint32;
#endif

#ifndef uint16
typedef unsigned short uint16;
#endif

#ifndef sint16
typedef short sint16;
#endif

#ifndef uint8
typedef unsigned char uint8;
#endif

#ifndef sint8
typedef  char sint8;
#endif

#ifndef BOOL
typedef uint8 BOOL;
#endif

#define SAFE_FREE(ptr) free(ptr)

#if defined(WIN32) && !defined(__cplusplus)
#define inline __inline
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* _NGE_COMMON_H */
