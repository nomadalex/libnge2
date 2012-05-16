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
 *@param[in] filename gif文件名
 *@param[in] displaymode 显示模式
 *@param[in] swizzle 优化标志,通常填1
 *@return gif_desc_p gif绘图文件句柄,供显示函数调用
 */
	NGE_API gif_desc_p gif_animation_load(const char* filename,int displaymode,int swizzle);

/**
 * 释放一个gif动画资源
 *@param[in] pgif 由gif_animation_load加载的描述符
 *@return
 */
	NGE_API void gif_animation_free(gif_desc_p pgif);

/**
 * 在屏幕上显示一个gif动画1, 效率最高
 *@param[in] pgif 由gif_animation_load加载的描述符
 *@param[in] x 屏幕x坐标
 *@param[in] y 屏幕y坐标
 *@return
 */
	NGE_API void GifAnimationToScreen(gif_desc_p pgif,float x,float y);

/**
 * 在屏幕上显示一个gif动画2, 效率最低
 * sx,sy,sw,sh,构成待显示的动画范围，例如要显示一张200*200的动画的
 * 20，20到50宽50高的子图块，这里就填依次20,20,50,50。如果要显示原图,都填上0
 * 即可.都填0是显示0,0,pgif->gif_w,pgif->gif_w。xscale,yscale是放大缩小因子1是原始比例，
 * 如果放大一倍，都填2，缩小1倍填0.5。mask是颜色遮罩，用于对图片进行混色，
 * 例如半透明效果等，默认是显示原色，这里用pgif->gif_image_chains->pimage->mask即可
 * 例子1:将200*200的动画pgif显示在屏幕100，0处，并放大一倍显示
 * RenderGifAnimation(pgif,0,0,0,0,100,0,2,2,0,pgif->gif_image_chains->pimage->mask);
 * 例子2:将动画pgif的32，32开始的64宽,64高的部分显示在40，80处并旋转90度
 * RenderGifAnimation(pgif,32,32,64,64,40,80,1,1,90,pgif->gif_image_chains->pimage->mask);
 * 例子3:将动画pgif(pgif显示模式是8888)的80，80开始的64宽,64高的部分显示在100，100处并半透明显示
 * RenderGifAnimation(pgif,80,80,64,64,100,100,1,1,0,MAKE_RGBA_8888(255,255,255,128));
 *@param[in] pgif 由gif_animation_load加载的描述符
 *@param[in] sx 动画x坐标
 *@param[in] sy 动画y坐标
 *@param[in] sw 动画宽
 *@param[in] sh 动画高,
 *@param[in] dx 屏幕x坐标
 *@param[in] dy 屏幕y坐标
 *@param[in] xscale 动画x方向放大缩小因子
 *@param[in] yscale 动画y方向放大缩小因子
 *@param[in] angle 旋转角度
 *@param[in] mask 颜色遮罩
 *@return
 */
	NGE_API void RenderGifAnimation(gif_desc_p pgif,float sx ,float sy ,float sw ,float sh ,float dx ,float dy ,float xscale  ,float yscale ,float angle ,int mask);

/**
 * 在屏幕上显示一个gif动画3
 * 同DrawGifAnimation，只是多了个MASK
 *@param[in] pgif 由gif_animation_load加载的描述符
 *@param[in] sx 动画x坐标
 *@param[in] sy 动画y坐标
 *@param[in] sw 动画宽
 *@param[in] sh 动画高
 *@param[in] dx 屏幕x坐标
 *@param[in] dy 屏幕y坐标
 *@param[in] dw 屏幕宽
 *@param[in] dh 屏幕高
 *@param[in] mask 颜色遮罩
 *@return 无
 */
	NGE_API void DrawGifAnimationMask(gif_desc_p pgif,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,int mask);

/**
 * 在屏幕上显示一个gif动画3,sw,sh为0是画原图，dw，dh为0是显示sw和dh大小
 * 例子1:将200*200的gif动画pgif显示在屏幕100，0处
 * DrawGifAnimation(pgif,0,0,0,0,100,0,0,0);
 * 或者用严格方式：DrawGifAnimation(pgif,0,0,200,200,100,0,200,200);
 *@param[in] pgif 由gif_animation_load加载的描述符
 *@param[in] sx 动画x坐标
 *@param[in] sy 动画y坐标
 *@param[in] sw 动画宽
 *@param[in] sh 动画高
 *@param[in] dx 屏幕x坐标
 *@param[in] dy 屏幕y坐标
 *@param[in] dw 屏幕宽
 *@param[in] dh 屏幕高
 *@return 无
 */
	NGE_API void DrawGifAnimation(gif_desc_p pgif,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh);

#ifdef __cplusplus
}
#endif

#endif /* _NGE_GIF_H */
