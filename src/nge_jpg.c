/***************************************************************************
 *            nge_jpg.c
 *
 *  2011/03/27 06:19:25
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
#include "nge_jpg.h"
#include "nge_io_file.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "jpeglib.h"
#if JPEG_LIB_VERSION > 62
#define USE_HIGH_JPEGLIB
#endif

//////////////////////////////////////////////////////////////////////////
//for jpeg load
//////////////////////////////////////////////////////////////////////////
#ifndef USE_HIGH_JPEGLIB
static void jpg_null(j_decompress_ptr cinfo)
{
}

static boolean jpg_fill_input_buffer(j_decompress_ptr cinfo)
{
	return 1;
}

static void jpg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{

	cinfo->src->next_input_byte += (size_t) num_bytes;
	cinfo->src->bytes_in_buffer -= (size_t) num_bytes;

}

static void jpeg_mem_src(j_decompress_ptr cinfo, uint8_t *mem, int len)
{
	cinfo->src = (struct jpeg_source_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));
	cinfo->src->init_source = jpg_null;
	cinfo->src->fill_input_buffer = jpg_fill_input_buffer;
	cinfo->src->skip_input_data = jpg_skip_input_data;
	cinfo->src->resync_to_restart = jpeg_resync_to_restart;
	cinfo->src->term_source = jpg_null;
	cinfo->src->bytes_in_buffer = len;
	cinfo->src->next_input_byte = mem;
}
#endif

image_p image_load_jpg(const char* filename, int displaymode)
{
	image_p pimage = NULL;
	int size;
	char *mbuf;
	int handle = io_fopen(filename,IO_RDONLY);
	if(handle == 0)
		return 0;
	size = io_fsize(handle);
	mbuf = (char*) malloc(size);
	io_fread(mbuf,size,1,handle);
	io_fclose(handle);
	pimage = image_load_jpg_buf(mbuf,size,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_jpg_buf(const char* mbuf,int bsize, int displaymode)
{
	image_p pimage = NULL;
	struct jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr jerr;
	uint8_t *rawdata,*scanline, *p,*data = NULL;
	int	rawsize,texw,texh,width,height,bpb=2,size;
	int r,g,b,x;
	uint32_t* p32;
	uint16_t* p16;
	uint16_t color16;
	uint32_t color32;

	if(mbuf == NULL||bsize==0)
		return 0;
	memset(&cinfo,0,sizeof(struct jpeg_decompress_struct));
	memset(&jerr,0,sizeof(struct jpeg_error_mgr));

	rawsize = bsize;
	rawdata = (uint8_t*)mbuf;

	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo,JPEG_LIB_VERSION);

	jpeg_mem_src(&cinfo, rawdata, rawsize);

	jpeg_read_header(&cinfo, 1);

	jpeg_start_decompress(&cinfo);

	if(cinfo.output_components != 3 && cinfo.output_components != 4)
	{
		jpeg_destroy_decompress(&cinfo);
		return 0;
	}
	texw = roundpower2(cinfo.output_width);
	texh = roundpower2(cinfo.output_height);
	width = cinfo.output_width;
	height = cinfo.output_height;

	if(displaymode == DISPLAY_PIXEL_FORMAT_8888){
		bpb = 4;
	}
	size = texw * texh * bpb;
	data = (uint8_t*)malloc(size);
	memset(data,0,size);
	scanline = (uint8_t*)malloc(cinfo.output_width * 3);
	if(!scanline){
		jpeg_destroy_decompress(&cinfo);
		SAFE_FREE (data);
		return 0;
	}
	p32 = (uint32_t*)data;
	p16 = (uint16_t*) p32;

	while(cinfo.output_scanline < cinfo.output_height){
		jpeg_read_scanlines(&cinfo, &scanline, 1);
		p = (uint8_t*)scanline;
		for(x=0; x<(int)cinfo.output_width; x++){

			r = p[0];
			g = p[1];
			b = p[2];
			switch (displaymode){
				case DISPLAY_PIXEL_FORMAT_565:
					color16 = MAKE_RGBA_565(r,g,b,0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					color16 = MAKE_RGBA_5551(r,g,b,0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					color16 = MAKE_RGBA_4444(r,g,b,0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_8888:
					color32 = MAKE_RGBA_8888(r,g,b,0xff);
					*(p32+x) = color32;
					break;
			}
			p+=3;
		}
		p32 += texw;
		p16 += texw;
	}
	SAFE_FREE(scanline);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	pimage = (image_p)malloc(sizeof(image_t));
	memset(pimage,0,sizeof(image_t));
	pimage->data = (uint8_t *)data;
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
	return pimage;
}

image_p image_load_jpg_fp(int handle,int fsize, int autoclose,int displaymode)
{
	image_p pimage = NULL;
	char* mbuf;

	if(handle == 0 || fsize == 0)
		return 0;

	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,fsize,1,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_jpg_buf(mbuf,fsize,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_jpg_colorkey(const char* filename, int displaymode,int colorkey)
{
	image_p pimage = NULL;
	int size;
	char *mbuf;
	int handle = io_fopen(filename,IO_RDONLY);

	if(handle == 0)
		return 0;
	size = io_fsize(handle);
	mbuf = (char*) malloc(size);
	io_fread(mbuf,size,1,handle);
	io_fclose(handle);
	pimage = image_load_jpg_colorkey_buf(mbuf,size,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_jpg_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey)
{
	image_p pimage = NULL;
	struct jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr jerr;
	uint8_t *rawdata,*scanline, *p,*data = NULL;
	int	rawsize,texw,texh,width,height,bpb=2,size;
	int r,g,b,x,color;
	uint32_t* p32;
	uint16_t* p16;
	uint16_t color16;
	uint32_t color32;
	uint8_t alpha;

	if(mbuf == NULL||bsize==0)
		return 0;

	rawsize = bsize;
	rawdata = (uint8_t*)mbuf;


	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, rawdata, rawsize);

	jpeg_read_header(&cinfo, 1);

	jpeg_start_decompress(&cinfo);

	if(cinfo.output_components != 3 && cinfo.output_components != 4)
	{
		jpeg_destroy_decompress(&cinfo);
		return 0;
	}

	texw = roundpower2(cinfo.output_width);
	texh = roundpower2(cinfo.output_height);
	width = cinfo.output_width;
	height = cinfo.output_height;

	if(displaymode == DISPLAY_PIXEL_FORMAT_8888){
		bpb = 4;
	}
	size = texw * texh * bpb;
	data = (uint8_t*)malloc(size);
	memset(data,0,size);

	scanline = (uint8_t*)malloc(cinfo.output_width * 3);
	if(!scanline){
		jpeg_destroy_decompress(&cinfo);
		SAFE_FREE (data);
		return 0;
	}
	p32 = (uint32_t*)data;
	p16 = (uint16_t*) p32;
	while(cinfo.output_scanline < cinfo.output_height){
		jpeg_read_scanlines(&cinfo, &scanline, 1);
		p = (uint8_t*)scanline;
		for(x=0; x<(int)cinfo.output_width; x++){

			r = p[0];
			g = p[1];
			b = p[2];
			color = MAKE_RGB(r,g,b);
			alpha = 0xff;
			if(colorkey == color){
				alpha = 0x00;
			}
			switch (displaymode){
				case DISPLAY_PIXEL_FORMAT_565:
					color16 = MAKE_RGBA_565(r,g,b,alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					color16 = MAKE_RGBA_5551(r,g,b,alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					color16 = MAKE_RGBA_4444(r,g,b,alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_8888:
					color32 = MAKE_RGBA_8888(r,g,b,alpha);
					*(p32+x) = color32;
					break;
			}
			p+=3;
		}
		p32 += texw;
		p16 += texw;
	}
	SAFE_FREE(scanline);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	pimage = (image_p)malloc(sizeof(image_t));
	memset(pimage,0,sizeof(image_t));
	pimage->data = (uint8_t *)data;
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
	return pimage;
}

image_p image_load_jpg_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey)
{
	image_p pimage = NULL;
	char *mbuf;

	if(handle == 0 || fsize == 0)
		return 0;

	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,fsize,1,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_jpg_colorkey_buf(mbuf,fsize,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}
