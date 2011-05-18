/***************************************************************************
 *            nge_image.h
 *
 *  2011/03/25 04:56:54
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

#ifndef _NGE_IMAGE_H
#define _NGE_IMAGE_H

#include "nge_common.h"
#include "nge_color.h"

typedef struct tagImage{
	uint32  w;        /**< 图片原宽 */
	uint32  h;        /**< 图片原高 */
	uint32  texw;     /**< 图片贴图宽 */
	uint32  texh;     /**< 图片贴图高 */
	uint8   bpb;      /**< 每像素字节数(16位2字节,32位4字节) */
	uint8   swizzle;  /**< 是否对data swizzle 优化*/
	uint32  mode;     /**< GSM显示mode for psp */
	uint32  dtype;    /**< 显示模式(DISPLAY_PIXEL_FORMAT_XXX的一种) */
	uint8*  data;     /**< 图像数据 */
	float   rcentrex; /**< 图像中心x */
	float   rcentrey; /**< 图像中心y */
	uint32  texid;    /**< 图像id */
	uint8   modified; /**< 强制更新显存*/
	uint8   dontswizzle; /**强制不swizzle 优化*/
	int     mask;     /**< 颜色遮罩,用于显示各种效果*/
}image_t,*image_p;

extern uint32 image_tid;

#ifdef __cplusplus
extern "C" {
#endif

/**
 *加载图片,从文件加载
 *@param const char* filename,图片文件名
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@param int swizzle,是否做优化,填0不做优化,通常填1
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load(const char* filename, int displaymode,int swizzle);

/**
 *加载图片,从内存加载
 *@param const char* mbuf,内存地址
 *@param int bsize,上述buffer大小
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@param int swizzle,是否做优化,填0不做优化,通常填1
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_buf(const char* mbuf,int bsize, int displaymode,int swizzle);

/**
 *加载图片,从文件指针加载
 *@param int handle,文件指针
 *@param int fsize,上述文件大小
 *@param int autoclose,是否关闭文件,是否关闭文件,0不关闭,1关闭
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@param int swizzle,是否做优化,填0不做优化,通常填1
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_fp(int handle,int fsize, int autoclose,int displaymode,int swizzle);

/**
 * 同image_load函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_colorkey(const char* filename, int displaymode,int colorkey,int swizzle);

/**
 * 同image_load_buf函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey,int swizzle);

/**
 *创建一个image结构,显示模式是displaymode
 *@param int w,image的宽
 *@param int h,image的高
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p pimage,返回image指针
 */
image_p image_create(int w,int h,int displaymode);

/**
 *创建一个image结构,显示模式是displaymode
 *@param int w,image的宽
 *@param int h,image的高
 *@param int color,颜色,为保证兼容性请用MAKE_RGBA_XXXX生成这个颜色
 *@param int displaymode,显示模式,与颜色一致,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p pimage,返回image指针
 */
image_p image_create_ex(int w,int h,int color,int displaymode);

/**
 *将image保存，根据文件名判断保存的格式，目前支持png和tga格式
 *@param image_p pimage,待保存的image指针
 *@param const char* filename, 保存的文件名
 *@param uint8 alpha,是否保存为含alpha通道的图片
 *@param uint8 rle,是否保存为rle压缩格式的图片
 *@return int,返回0(加载失败)或1(加载成功)
 */
int image_save(image_p pimage,const char* filename,uint8 alpha,uint8 rle);
	
/**
 *释放一个image
 *@param image_p pimage,待释放的image指针
 *@return void,无返回
 */
void image_free(image_p pimage);

/**
 *clone一个image,并把image_p返回一个pimage的深拷贝
 *@param image_p pimage,待clone的image指针
 *@return image_p,pimage的深拷贝
 */
image_p image_clone(image_p pimage);

/**
 *将数据清空,图像变为无色透明
 *@param image_p pimage,待清除的image指针
 *@return void,无返回
 */
void image_clear(image_p pimage);

/**
 *将源image_p src拷贝到image_p des.注意此函数通常只用作兼容其他使用
 *通常直接用DrawImage到screen是硬件加速.
 *@param const image_p src,源内存图
 *@param const image_p des,目的内存图
 *@param uint32 dx,目的内存图的x坐标
 *@param uint32 dy,目的内存图的y坐标
 *@return
 */
void image_to_image(const image_p src,const image_p des,uint32 dx,uint32 dy);

/**
 *将源image_p src的sx,sh,sw,sh拷贝到image_p des.注意此函数通常只用作兼容其他使用
 *通常直接用DrawImage到screen是硬件加速.
 *@param const image_p src,源内存图
 *@param const image_p des,目的内存图
 *@param uint32 sx,源内存图的x坐标
 *@param uint32 sy,源内存图的y坐标
 *@param uint32 sw,源内存图的宽
 *@param uint32 sh,源内存图的高
 *@param uint32 dx,目的内存图的x坐标
 *@param uint32 dy,目的内存图的y坐标
 *@return
 */
void image_to_image_ex(const image_p src,const image_p des,uint32 sx,uint32 sy,uint32 sw,uint32 sh,uint32 dx,uint32 dy);

/**
 *将源image_p src拷贝到image_p des 的alpha混合.注意此函数通常只用作兼容其他使用
 *通常直接用DrawImage到screen是硬件加速.
 *@param const image_p src,源内存图
 *@param const image_p des,目的内存图
 *@param uint32 dx,目的内存图的x坐标
 *@param uint32 dy,目的内存图的y坐标
 *@param int alpha,alpha混合值,0-255(透明->不透明),例如128就是半透明
 *@return
 */
void image_to_image_alpha(const image_p src,const image_p des,uint32 dx,uint32 dy,int alpha);

/**
 *将源image_p src的sx,sh,sw,sh拷贝到image_p des.注意此函数通常只用作兼容其他使用
 *通常直接用DrawImage到screen是硬件加速.
 *@param const image_p src,源内存图
 *@param const image_p des,目的内存图
 *@param uint32 sx,源内存图的x坐标
 *@param uint32 sy,源内存图的y坐标
 *@param uint32 sw,源内存图的宽
 *@param uint32 sh,源内存图的高
 *@param uint32 dx,目的内存图的x坐标
 *@param uint32 dy,目的内存图的y坐标
 *@param int alpha,alpha混合值,0-255(透明->不透明),例如128就是半透明
 *@return
 */
void image_to_image_alpha_ex(const image_p src,const image_p des,uint32 sx,uint32 sy,uint32 sw,uint32 sh,uint32 dx,uint32 dy,int alpha);

/**
 *将原始位图格式数据拷贝到image,考虑到效率问题,只支持同种displaymode,
 *注意此处未加判定,所以需要保证data数据正确性
 *@param void* data,位图数据
 *@param image_p ,image指针,目标image
 *@param int x,拷贝到image上的dx位置
 *@param int y,拷贝到image上的dy位置
 *@param int w,data的w
 *@param int h,data的h
 *@return void,无返回
 */
void rawdata_to_image(void* data,const image_p des,uint32 x,uint32 y,uint32 w,uint32 h);

/**
 * 将图像做水平翻转，图像坐标x不变
 *@param image_p pimage,待翻转图像
 *@return int,成功1,失败0
 */
int image_fliph(image_p pimage);

/**
 * 将图像做垂直翻转，翻转后x轴对称
 *@param image_p pimage,待翻转图像
 *@return int,成功1,失败0
 */
int image_flipv(image_p pimage); 

//以下函数为内部使用
int GET_PSM_COLOR_MODE(int dtype);
int roundpower2(int width);

#ifdef _PSP
void swizzle_swap(image_p pimage);
void unswizzle_swap(image_p pimage);
#else
#define swizzle_swap(p)
#define unswizzle_swap(p)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _NGE_IMAGE_H */
