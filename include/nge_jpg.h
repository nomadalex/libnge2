/***************************************************************************
 *            nge_jpg.h
 *
 *  2011/03/25 05:05:52
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

#ifndef _NGE_JPG_H
#define _NGE_JPG_H

#include "nge_image.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *加载jpg图片,从文件加载
 *@param const char* filename,图片文件名
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_jpg(const char* filename, int displaymode);

/**
 *加载jpg图片,从内存加载
 *@param const char* mbuf,内存地址
 *@param int bsize,上述buffer大小
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_jpg_buf(const char* mbuf,int bsize, int displaymode);

/**
 *加载jpg图片,从文件指针加载
 *@param int handle,文件指针
 *@param int fsize,上述文件大小
 *@param int autoclose,是否关闭文件,是否关闭文件,0不关闭,1关闭
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_jpg_fp(int handle,int fsize, int autoclose,int displaymode);

/**
 * 同image_load_jpg函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_jpg_colorkey(const char* filename, int displaymode,int colorkey);

/**
 * 同image_load_jpg_buf函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_jpg_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey);

/**
 * 同image_load_jpg_fp函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_jpg_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey);

#ifdef __cplusplus
}
#endif

#endif /* _NGE_JPG_H */
