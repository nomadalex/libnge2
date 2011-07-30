/***************************************************************************
 *            nge_color.h
 *
 *  2011/03/25 05:01:45
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

#ifndef _NGE_COLOR_H
#define _NGE_COLOR_H

#include "nge_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* color define */
#if defined _PSP
#include <pspgu.h>
#define DISPLAY_PIXEL_FORMAT_565  GU_COLOR_5650
#define DISPLAY_PIXEL_FORMAT_5551 GU_COLOR_5551
#define DISPLAY_PIXEL_FORMAT_4444 GU_COLOR_4444
#define DISPLAY_PIXEL_FORMAT_8888 GU_COLOR_8888
//blend methord
#define BLEND_ZERO					0x1000
#define BLEND_ONE					0x1002
#define BLEND_SRC_COLOR				GU_SRC_COLOR
#define BLEND_ONE_MINUS_SRC_COLOR	GU_ONE_MINUS_SRC_COLOR
#define BLEND_SRC_ALPHA				GU_SRC_ALPHA
#define BLEND_ONE_MINUS_SRC_ALPHA	GU_ONE_MINUS_SRC_ALPHA
#define BLEND_DST_ALPHA				GU_DST_ALPHA
#define BLEND_ONE_MINUS_DST_ALPHA	GU_ONE_MINUS_DST_ALPHA
#define BLEND_DST_COLOR				GU_DST_COLOR
#define BLEND_ONE_MINUS_DST_COLOR	GU_ONE_MINUS_DST_COLOR
#define BLEND_SRC_ALPHA_SATURATE	BLEND_ONE

#define PSM_5551 GU_PSM_5551
#define PSM_565  GU_PSM_5650
#define PSM_4444 GU_PSM_4444
#define PSM_8888 GU_PSM_8888
#else
//blend method hack to fuck with <windows.h>,this is copy from <gl/gl.h>
#define GL_UNSIGNED_SHORT_5_6_5   0x8363
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_1_5_5_5 0x8034
#define HACK_GL_UNSIGNED_BYTE     0x1401
#define DISPLAY_PIXEL_FORMAT_565  GL_UNSIGNED_SHORT_5_6_5
#define DISPLAY_PIXEL_FORMAT_5551 GL_UNSIGNED_SHORT_1_5_5_5
#define DISPLAY_PIXEL_FORMAT_4444 GL_UNSIGNED_SHORT_4_4_4_4
#define DISPLAY_PIXEL_FORMAT_8888 HACK_GL_UNSIGNED_BYTE

#define BLEND_ZERO					0
#define BLEND_ONE					1
#define BLEND_SRC_COLOR				0x0300
#define BLEND_ONE_MINUS_SRC_COLOR	0x0301
#define BLEND_SRC_ALPHA				0x0302
#define BLEND_ONE_MINUS_SRC_ALPHA	0x0303
#define BLEND_DST_ALPHA				0x0304
#define BLEND_ONE_MINUS_DST_ALPHA	0x0305
#define BLEND_DST_COLOR				0x0306
#define BLEND_ONE_MINUS_DST_COLOR	0x0307
#define BLEND_SRC_ALPHA_SATURATE	0x0308
//define for psp compatiable,no use on other
#define PSM_5551 0
#define PSM_565  1
#define PSM_4444 2
#define PSM_8888 3
#endif

#define MAKE_RGB(r,g,b) (r | (g << 8) | (b << 16)|((0xff)<<24))
#define MAKE_RGBA_8888(r,g,b,a)  (r | (g << 8) | (b << 16) | (a << 24))
#define GET_RGBA_8888(col,r,g,b,a)	{ \
	a=(col>>24)&0xFF;\
	b=(col>>16)&0xFF;\
	g=(col>>8)&0xFF;\
	r=(col&0xFF);\
}
#define MAKE_RGBA(r,g,b,a)  (r | (g << 8) | (b << 16) | (a << 24))
#define GET_RGBA_R(col)	(col&0xFF)
#define GET_RGBA_G(col)	((col>>8)&0xFF)
#define GET_RGBA_B(col)	((col>>16)&0xFF)
#define GET_RGBA_A(col)	((col>>24)&0xFF)

//注意PSP是ABGR(高->低)其他是RGBA(高->低)正好反序
#if defined _PSP
#define MAKE_RGBA_5551(r,g,b,a)  ((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | ((a >> 7) << 15))
#define MAKE_RGBA_4444(r,g,b,a)  ((r >> 4) | ((g >> 4) << 4) | ((b >> 4) << 8) | ((a >> 4) << 12))
#define MAKE_RGBA_565(r,g,b,a)   ((r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11))
#define GET_RGBA_565(col,r,g,b,a)	{ \
	a=0xFF;\
	b=(((col>>11)&0x1F)*255/31);\
	g=(((col>>5)&0x3F)*255/63);\
	r=((col&0x1F)*255/31);\
}
#define GET_RGBA_5551(col,r,g,b,a)  { \
	a=((col>>15)==0?0:0xFF);\
	b=(((col>>10)&0x1F)*255/31);\
	g=(((col>>5)&0x1F)*255/31);\
	r=((col&0x1F)*255/31);\
}
#define GET_RGBA_4444(col,r,g,b,a)	{ \
	a=(((col>>12)&0xF)*255/15);\
	b=(((col>>8)&0xF)*255/15);\
	g=(((col>>4)&0xF)*255/15);\
	r=((col&0xF)*255/15);\
}
#else
#define MAKE_RGBA_5551(r,g,b,a)  (((r >> 3)<<11) | ((g >> 3) << 6) | ((b >> 3) << 1) | (a >> 7))
#define MAKE_RGBA_4444(r,g,b,a)  (((r >> 4)<<12) | ((g >> 4) <<8) | ((b >> 4) << 4) | (a >> 4))
#define MAKE_RGBA_565(r,g,b,a)   (((r >> 3)<<11) | ((g >> 2) << 5) | (b >> 3))

#define GET_RGBA_565(col,r,g,b,a)	{			\
		a=0xFF;									\
		b=((col&0x1F)*255/31);					\
		g=(((col>>5)&0x3F)*255/63);				\
		r=(((col>>11)&0x1F)*255/31);			\
	}
#define GET_RGBA_5551(col,r,g,b,a)  {			\
		a=((col&0x1)==0?0:0xFF);				\
		b=(((col>>1)&0x1F)*255/31);				\
		g=(((col>>6)&0x1F)*255/31);				\
		r=(((col>>11)&0x1F)*255/31);			\
	}
#define GET_RGBA_4444(col,r,g,b,a)	{			\
		a=((col&0xF)*255/15);					\
		b=(((col>>4)&0xF)*255/15);				\
		g=(((col>>8)&0xF)*255/15);				\
		r=(((col>>12)&0xF)*255/15);				\
	}
#endif

/* some color define */
#define BLACK		MAKE_RGB( 0  , 0  , 0   )
#define BLUE		MAKE_RGB( 0  , 0  , 128 )
#define GREEN		MAKE_RGB( 0  , 128, 0   )
#define CYAN		MAKE_RGB( 0  , 128, 128 )
#define RED			MAKE_RGB( 128, 0  , 0   )
#define MAGENTA		MAKE_RGB( 128, 0  , 128 )
#define BROWN		MAKE_RGB( 128, 64 , 0   )
#define LTGRAY		MAKE_RGB( 192, 192, 192 )
#define GRAY		MAKE_RGB( 128, 128, 128 )
#define LTBLUE		MAKE_RGB( 0  , 0  , 255 )
#define LTGREEN		MAKE_RGB( 0  , 255, 0   )
#define LTCYAN		MAKE_RGB( 0  , 255, 255 )
#define LTRED		MAKE_RGB( 255, 0  , 0   )
#define LTMAGENTA	MAKE_RGB( 255, 0  , 255 )
#define YELLOW		MAKE_RGB( 255, 255, 0   )
#define WHITE		MAKE_RGB( 255, 255, 255 )
#define DKGRAY		MAKE_RGB( 32,  32,  32)

#define BLACK_5551		MAKE_RGBA_5551( 0  , 0  , 0   ,255)
#define BLUE_5551		MAKE_RGBA_5551( 0  , 0  , 128 ,255)
#define GREEN_5551		MAKE_RGBA_5551( 0  , 128, 0   ,255)
#define CYAN_5551		MAKE_RGBA_5551( 0  , 128, 128 ,255)
#define RED_5551		MAKE_RGBA_5551( 128, 0  , 0   ,255)
#define MAGENTA_5551	MAKE_RGBA_5551( 128, 0  , 128 ,255)
#define BROWN_5551		MAKE_RGBA_5551( 128, 64 , 0   ,255)
#define LTGRAY_5551		MAKE_RGBA_5551( 192, 192, 192 ,255)
#define GRAY_5551		MAKE_RGBA_5551( 128, 128, 128 ,255)
#define LTBLUE_5551		MAKE_RGBA_5551( 0  , 0  , 255 ,255)
#define LTGREEN_5551	MAKE_RGBA_5551( 0  , 255, 0   ,255)
#define LTCYAN_5551		MAKE_RGBA_5551( 0  , 255, 255 ,255)
#define LTRED_5551		MAKE_RGBA_5551( 255, 0  , 0   ,255)
#define LTMAGENTA_5551	MAKE_RGBA_5551( 255, 0  , 255 ,255)
#define YELLOW_5551		MAKE_RGBA_5551( 255, 255, 0   ,255)
#define WHITE_5551		MAKE_RGBA_5551( 255, 255, 255 ,255)
#define DKGRAY_5551		MAKE_RGBA_5551( 32,  32,  32  ,255)

#define FONT_BG_5551 MAKE_RGBA_5551(0xff,0xff,0xff,0x00)
#define FONT_FG_5551 MAKE_RGBA_5551(0x00,0x00,0x00,0xff)
#define FONT_SH_5551 MAKE_RGBA_5551(0x7f,0x7f,0x7f,0xff)
#define FONT_BG_4444 MAKE_RGBA_4444(0xff,0xff,0xff,0x00)
#define FONT_FG_4444 MAKE_RGBA_4444(0x00,0x00,0x00,0xff)
#define FONT_SH_4444 MAKE_RGBA_4444(0x7f,0x7f,0x7f,0xff)
#define FONT_BG_565  MAKE_RGBA_565(0xff,0xff,0xff,0x00)
#define FONT_FG_565  MAKE_RGBA_565(0x00,0x00,0x00,0xff)
#define FONT_SH_565  MAKE_RGBA_565(0x7f,0x7f,0x7f,0xff)
#define FONT_BG_8888 MAKE_RGBA_8888(0xff,0xff,0xff,0x00)
#define FONT_FG_8888 MAKE_RGBA_8888(0x00,0x00,0x00,0xff)
#define FONT_SH_8888 MAKE_RGBA_8888(0x7f,0x7f,0x7f,0xff)

typedef struct{
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;
}color4ub;

/**
 *根据颜色分量创建一个dtype对应的色彩
 *@param uint8 r,r颜色分量
 *@param uint8 g,g颜色分量
 *@param uint8 b,b颜色分量
 *@param uint8 a,a颜色分量
 *@param int dtype,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return int ,颜色值
 */
int CreateColor(uint8 r,uint8 g,uint8 b,uint8 a,int dtype); // it is in nge_image.c

#ifdef __cplusplus
}
#endif

#endif /* _NGE_COLOR_H */
