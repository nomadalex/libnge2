/***************************************************************************
 *            nge_gif.h
 *
 *  2011/03/26 02:03:11
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

#ifndef _NGE_GIF_H
#define _NGE_GIF_H

#include "nge_common.h"
#include "nge_image.h"
#include "nge_timer.h"

typedef struct _image_chain{
	image_p pimage;
	uint32  frame_delay;
	struct _image_chain* next;
}image_chains_t,*image_chains_p;

//struct tag_gif_desc_t;
#define GIF_SHOW_INIT 0
#define GIF_SHOW_PLAY 1
#define GIF_SHOW_STOP 2
//typedef image_p (*gif_nextframe)(struct tag_gif_desc_t* pgif);

typedef struct  tag_gif_desc_t{
	image_chains_p gif_image_chains;
	uint32	       gif_delay;
	sint32		   gif_transparent;
	uint32         gif_dispose;
	uint32		   gif_loopcount;
	sint32         gif_rgbtransparent;
	sint32         gif_rgbbackgnd;
	uint32         gif_w;
	uint32         gif_h;
	uint32         gif_texw;
	uint32         gif_texh;
	uint32         gif_framecount;
	/*for display*/
	nge_timer*     gif_inner_timer;
	uint32         gif_last_ticks;
	uint32         gif_current_ticks;
	uint32         gif_show_status;
	image_p        gif_current_frame;
	image_chains_p gif_current_chains;
}gif_desc_t,*gif_desc_p;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * gif动画加载程序,将gif动画帧全部加载到内存,
 * 本函数是预先加载函数,请注意内存的使用量
 *@param const char* filename,gif文件名
 *@param int displaymode,显示模式
 *@param int swizzle,优化标志,通常填1
 *@return gif_desc_p,gif绘图文件句柄,供显示函数调用
 */
	NGE_API gif_desc_p gif_animation_load(const char* filename,int displaymode,int swizzle);

/**
 * 释放一个gif动画资源
 *@param gif_desc_p pgif,由gif_animation_load加载的描述符
 *@return
 */
	NGE_API void gif_animation_free(gif_desc_p pgif);

	NGE_API void GifAnimationToScreen(gif_desc_p pgif,float x,float y);
	NGE_API void RenderGifAnimation(gif_desc_p pgif,float sx ,float sy ,float sw ,float sh ,float dx ,float dy ,float xscale  ,float yscale ,float angle ,int mask);
	NGE_API void DrawGifAnimationMask(gif_desc_p pgif,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,int mask);
	NGE_API void DrawGifAnimation(gif_desc_p pgif,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh);

#ifdef __cplusplus
}
#endif

#endif /* _NGE_GIF_H */
