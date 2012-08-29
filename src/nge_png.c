/***************************************************************************
 *            nge_png.c
 *
 *  2011/03/27 05:46:06
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
#include "nge_debug_log.h"
#include "nge_png.h"
#include "nge_io_file.h"
#include <stdlib.h>

#include "png.h"
#if PNG_LIBPNG_VER_MAJOR > 1 || PNG_LIBPNG_VER_MINOR > 2 || PNG_LIBPNG_VER_RELEASE > 46
#define USE_HIGH_LIBPNG
#endif

#ifdef USE_HIGH_LIBPNG // these macro deprecated in libpng1.4.4, so
#define int_p_NULL                NULL
#define png_bytep_NULL            NULL
#define png_bytepp_NULL           NULL
#define png_doublep_NULL          NULL
#define png_error_ptr_NULL        NULL
#define png_flush_ptr_NULL        NULL
#define png_free_ptr_NULL         NULL
#define png_infopp_NULL           NULL
#define png_malloc_ptr_NULL       NULL
#define png_read_status_ptr_NULL  NULL
#define png_rw_ptr_NULL           NULL
#define png_structp_NULL          NULL
#define png_uint_16p_NULL         NULL
#define png_voidp_NULL            NULL
#define png_write_status_ptr_NULL NULL
#define png_set_gray_1_2_4_to_8 png_set_expand_gray_1_2_4_to_8
#endif

//////////////////////////////////////////////////////////////////////////
//for png load
//////////////////////////////////////////////////////////////////////////
static void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	// ignore PNG warnings
}
static int offset = 0;

static void png_custom_mread_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{

	uint8_t* handle = (uint8_t*)png_ptr->io_ptr;
	uint8_t* workptr = handle + offset;
	uint32_t i;
   if (handle == NULL)
   {
	  png_error(png_ptr, "Read Error!");
   }
   offset +=length;
   for(i=0;i<length;i++){
		data[i]=workptr[i];
   }

}

image_p image_load_png(const char* filename, int displaymode)
{
	image_p pimage = NULL;
	int size = 0;
	char* mbuf = NULL;
	int handle = io_fopen(filename,IO_RDONLY);

	if(handle == 0)
		return 0;
	size = io_fsize(handle);
	mbuf = (char*) malloc(size);
	io_fread(mbuf,1,size,handle);
	io_fclose(handle);
	pimage = image_load_png_buf(mbuf,size,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_png_fp(int handle,int fsize, int autoclose,int displaymode)
{
	image_p pimage = NULL;
	char* mbuf;

	if(handle == 0 || fsize == 0)
		return 0;

	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_png_buf(mbuf,fsize,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_png_buf(const char* mbuf, int bsize, int displaymode)
{
	uint32_t* p32;
	uint16_t* p16;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, x, y;
	uint32_t* line;
	image_p pimage = NULL;
	int texw,texh,bpb,size;
	uint8_t done = 0;
	uint8_t* buffer = NULL;
	uint32_t color32;
	uint16_t color16;
	static uint8_t r,g,b,a;
	offset = 0;

	if (mbuf == 0) return 0;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
			return 0;
	}
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return 0;
	}
	png_init_io(png_ptr, NULL);
	png_set_read_fn(png_ptr, (png_voidp)mbuf, png_custom_mread_fn);

	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	line = (uint32_t*) malloc(width * 4);
	if (!line) {
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return 0;
	}

	texw = roundpower2(width);
	texh = roundpower2(height);
	bpb = 4;
	if(displaymode !=DISPLAY_PIXEL_FORMAT_8888){
		bpb = 2;
	}
	size = texw * texh * bpb;
	buffer = (uint8_t*) malloc(size);
	memset(buffer,0,size);
	if (buffer){
		p32 = (uint32_t*) buffer;
		p16 = (uint16_t*) p32;
		for (y = 0; y < (int)height; y++){
		png_read_row(png_ptr, (uint8_t*) line, png_bytep_NULL);
		for (x = 0; x < (int)width; x++)  {
			color32 = line[x];
			a = (color32 >> 24) & 0xff;
			r = color32 & 0xff;
			g = (color32 >> 8) & 0xff;
			b = (color32 >> 16) & 0xff;
			switch (displaymode){
				case DISPLAY_PIXEL_FORMAT_565:
					color16 = MAKE_RGBA_565(r,g,b,a);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					color16 = MAKE_RGBA_5551(r,g,b,a);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					color16 = MAKE_RGBA_4444(r,g,b,a);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_8888:
					color32 = MAKE_RGBA_8888(r,g,b,a);
					*(p32+x) = color32;
					break;
		   }
		}
			p32 += texw;
			p16 += texw;
		   }
		   done = 1;
	 }
	SAFE_FREE (line);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

	if (done){
		pimage = (image_p)malloc(sizeof(image_t));
		memset(pimage,0,sizeof(image_t));
		pimage->data = (uint8_t *)buffer;
		pimage->w    = width;
		pimage->h    = height;
		pimage->texw = texw;
		pimage->texh = texh;
		pimage->bpb  = bpb;
		pimage->dtype = displaymode;
		pimage->rcentrex = pimage->w*1.0f/2;
		pimage->rcentrey = pimage->h*1.0f/2;
		pimage->mode = GET_PSM_COLOR_MODE(displaymode);
		pimage->mask = CreateColor(255,255,255,255,displaymode);
		pimage->texid = image_tid++;
	}
	return pimage;
}




image_p image_load_png_colorkey(const char* filename, int displaymode,int colorkey)
{
	image_p pimage = NULL;
	int handle = io_fopen(filename,IO_RDONLY);
	int size;
	char* mbuf;

	if(handle == 0)
		return 0;
	size = io_fsize(handle);
	mbuf = (char*) malloc(size);
	io_fread(mbuf,1,size,handle);
	io_fclose(handle);
	pimage = image_load_png_colorkey_buf(mbuf,size,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_png_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey)
{
	image_p pimage = NULL;
	char *mbuf;

	if(handle == 0 || fsize == 0)
		return 0;

	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_png_colorkey_buf(mbuf,fsize,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_png_colorkey_buf(const char* mbuf, int bsize, int displaymode,int colorkey)
{

	uint32_t* p32;
	uint16_t* p16;
	uint8_t* buffer;
	uint32_t* line;
	image_p pimage = NULL;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, x, y;
	int texw,texh,bpb,size;
	uint8_t done = 0;
	int pixcolor;
	uint32_t color32;
	uint16_t color16;
	static uint8_t r,g,b,a,alpha;

	offset = 0;/*global*/

	if (mbuf == 0) return 0;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
			return 0;
	}
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return 0;
	}
	png_init_io(png_ptr, NULL);
	png_set_read_fn(png_ptr, (png_voidp)mbuf, png_custom_mread_fn);

	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	line = (uint32_t*) malloc(width * 4);
	if (!line) {
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return 0;
	}

	texw = roundpower2(width);
	texh = roundpower2(height);
	bpb = 4;
	if(displaymode !=DISPLAY_PIXEL_FORMAT_8888){
		bpb = 2;
	}
	size = texw * texh * bpb;
	buffer = (uint8_t*) malloc(size);
	memset(buffer,0,size);
	alpha = 0xff;

	if (buffer){
		p32 = (uint32_t*) buffer;
		p16 = (uint16_t*) p32;
		for (y = 0; y < (int)height; y++){
		png_read_row(png_ptr, (uint8_t*) line, png_bytep_NULL);
		for (x = 0; x < (int)width; x++)  {
			color32 = line[x];
					a = (color32 >> 24) & 0xff;
			r = color32 & 0xff;
			g = (color32 >> 8) & 0xff;
			b = (color32 >> 16) & 0xff;
					pixcolor = MAKE_RGB(r,g,b);
					alpha = 0xff;
					if(colorkey == pixcolor){
						alpha = 0x00;
					}
			switch (displaymode){
				case DISPLAY_PIXEL_FORMAT_565:
									if(alpha==0x00)
										color16 = MAKE_RGBA_565(r,g,b,alpha);
									else
										color16 = MAKE_RGBA_565(r,g,b,a);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
									if(alpha==0x00)
										color16 = MAKE_RGBA_5551(r,g,b,alpha);
									else
										color16 = MAKE_RGBA_5551(r,g,b,a);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					if(alpha==0x00)
										color16 = MAKE_RGBA_4444(r,g,b,alpha);
									else
										color16 = MAKE_RGBA_4444(r,g,b,a);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_8888:
					if(alpha==0x00)
										color32 = MAKE_RGBA_8888(r,g,b,alpha);
									else
										color32 = MAKE_RGBA_8888(r,g,b,a);
					*(p32+x) = color32;
					break;
		   }
		}
			p32 += texw;
			p16 += texw;
		   }
		   done = 1;
	 }
	SAFE_FREE (line);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

	if (done){
		pimage = (image_p)malloc(sizeof(image_t));
		memset(pimage,0,sizeof(image_t));
		pimage->data = (uint8_t *)buffer;
		pimage->w    = width;
		pimage->h    = height;
		pimage->texw = texw;
		pimage->texh = texh;
		pimage->bpb  = bpb;
		pimage->dtype = displaymode;
		pimage->rcentrex = pimage->w*1.0f/2;
		pimage->rcentrey = pimage->h*1.0f/2;
		pimage->mode = GET_PSM_COLOR_MODE(displaymode);
		pimage->mask = CreateColor(255,255,255,255,displaymode);
		pimage->texid = image_tid++;
	}
	return pimage;
}

//////////////////////////////////////////////////////////////////////////
//to png
//////////////////////////////////////////////////////////////////////////
static void png_custom_fwrite_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t check;

	int handle = (int)(png_ptr->io_ptr);

	check = io_fwrite(data,1,length,handle);
	if (check != length)
	{
		nge_print("Read Error!");
		png_error(png_ptr, "Read Error!");
	}
}

int image_save_png(image_p pimage,const char* filename,uint8_t alpha)
{
	png_structp png_ptr;
	png_infop info_ptr;
	uint8_t* line;
	uint32_t col_type;
	int handle;
	uint8_t *src;
	uint16_t *src16 ;
	uint32_t *src32 ;
	uint32_t x,y,i;
	uint8_t r = 0, g = 0, b = 0, a = 0;
	uint16_t col16;
	uint32_t col32;


	if (pimage==0)
		return 0;
	if (pimage->data==0 || pimage->w==0 || pimage->h==0)
		return 0;


	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return 0;

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return 0;
	}

	col_type = PNG_COLOR_TYPE_RGB;
	if (alpha!=0)
		col_type = PNG_COLOR_TYPE_RGBA;

	handle = io_fopen(filename, IO_WRONLY);
	if (handle == 0)
		return 0 ;

	png_init_io(png_ptr, NULL);
	png_set_write_fn(png_ptr, (png_voidp)handle, png_custom_fwrite_fn,NULL);
	png_set_IHDR(png_ptr, info_ptr, pimage->w, pimage->h,
		8, col_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);

	line = (uint8_t*) malloc(pimage->w * 4);
	if (line==0)
	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		io_fclose(handle);
		return 0;
	}

	src = (uint8_t*)pimage->data;
	src16 = (uint16_t*)src;
	src32 = (uint32_t*)src;
	for (y = 0; y < pimage->h; y++)
	{
		//uint32_t swap = 0;
		src16 = (uint16_t*)src;
		src32 = (uint32_t*)src;
		for (i = 0, x = 0; x < pimage->w; x++)
		{
			switch (pimage->dtype)
			{
				case DISPLAY_PIXEL_FORMAT_565:
					col16 = *src16++;
					GET_RGBA_565(col16,r,g,b,a);
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					col16 = *src16++;
					GET_RGBA_5551(col16,r,g,b,a);
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					col16 = *src16++;
					GET_RGBA_4444(col16,r,g,b,a);
					break;
				case DISPLAY_PIXEL_FORMAT_8888:
					col32 = *src32++;
					GET_RGBA_8888(col32,r,g,b,a);
					break;
			}
			line[i++] = r;
			line[i++] = g;
			line[i++] = b;
			if (alpha!=0)
				line[i++] = a;
		}
		png_write_row(png_ptr, line);
		src += ((pimage->texw*pimage->bpb));
	}
	SAFE_FREE(line);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	io_fclose(handle);
	return 1;
}
