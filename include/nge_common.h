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
#define NGE_MINOR_VERSION	1
#define NGE_PATCHLEVEL		0
#define NGE_VERSION "2.1.0"

#ifndef _MSC_VER
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL  (0)
#endif
#endif

typedef unsigned int uint32;
typedef int sint32;
typedef unsigned short uint16;
typedef short sint16;
typedef unsigned char uint8;
typedef char sint8;

#ifdef _MSC_VER
typedef int BOOL;
#else
typedef uint8 BOOL;
#endif

#define SAFE_FREE(ptr) free(ptr)

#if defined(_MSC_VER) && !defined(__cplusplus)
#define inline __inline
#endif

#endif /* _NGE_COMMON_H */
