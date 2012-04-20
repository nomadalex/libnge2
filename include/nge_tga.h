/***************************************************************************
 *            nge_tga.h
 *
 *  2011/03/25 05:05:01
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

#ifndef _NGE_TGA_H
#define _NGE_TGA_H

#include "nge_common.h"
#include "nge_image.h"

// ImageType Codes
#define TGA_TYPE_MAPPED		1
#define TGA_TYPE_COLOR		2
#define TGA_TYPE_GRAY		3
#define TGA_TYPE_MAPPED_RLE	9
#define TGA_TYPE_COLOR_RLE	10
#define TGA_TYPE_GRAY_RLE	11

/* Image descriptor:
   3-0: attribute (alpha) bpp
   4:   left-to-right ordering
   5:   top-to-bottom ordering
   7-6: zero
*/
// Image Description Bitmasks
#define TGA_DESC_ABITS		0x0f			// Alpha Bits
#define TGA_DESC_HORIZONTAL	0x10			// Left-Right Ordering: 0 = left to right, 1 = right to left
#define TGA_DESC_VERTICAL	0x20			// Top-Bottom Ordering: 0 = bottom to top, 1 = top to bottom

#pragma pack(push, 1)
typedef struct {
    uint8	ImageIDSize;
    uint8	ColorMapType;
    uint8	ImageTypeCode;                          // Image Type (normal/paletted/grayscale/rle)
    uint8	ColorMapOrigin[2];
    uint8	ColorMapLength[2];                      // Palette Size
    uint8	ColorMapESize;							// Size in bits of one Palette entry
    uint8	OriginX[2];
    uint8	OriginY[2];
    uint8	Width[2];                               // Width of Image
    uint8	Height[2];                              // Height of Image
    uint8	Depth;                                  // Bits per Pixel of Image
    uint8	ImageDescrip;
} TGAFILEHEADER;
#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

/**
 *加载tga图片,从文件加载
 *@param const char* filename,图片文件名
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
	NGE_API image_p image_load_tga(const char* filename, int displaymode);

/**
 *加载tga图片,从内存加载
 *@param const char* mbuf,内存地址
 *@param int bsize,上述buffer大小
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
	NGE_API image_p image_load_tga_buf(const char* mbuf,int bsize, int displaymode);

/**
 *加载tga图片,从文件指针加载
 *@param int handle,文件指针
 *@param int fsize,上述文件大小
 *@param int autoclose,是否关闭文件,是否关闭文件,0不关闭,1关闭
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
	NGE_API image_p image_load_tga_fp(int handle,int fsize, int autoclose,int displaymode);

/**
 * 同image_load_tga函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
	NGE_API image_p image_load_tga_colorkey(const char* filename, int displaymode,int colorkey);

/**
 * 同image_load_tga_buf函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
	NGE_API image_p image_load_tga_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey);

/**
 * 同image_load_png_fp函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
	NGE_API image_p image_load_tga_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey);

/**
 *存储tga图片,将image_p结构内容存储为tga文件
 *@param image_p,image_p结构指针,里面保存为image_t的数据
 *@param const char*,保存文件名
 *@param uint8,是否保存为含alpha通道的图片
 *@param uint8,是否采用rle编码压缩
 *@return int,返回0(加载失败)或1(加载成功)
 */
	NGE_API int image_save_tga(image_p pimage,const char* filename,uint8 alpha,uint8 rle);

#ifdef __cplusplus
}
#endif

#endif /* _NGE_TGA_H */
