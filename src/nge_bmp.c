/***************************************************************************
 *            nge_bmp.c
 *
 *  2011/03/27 05:44:49
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
#include "nge_common.h"
#include "nge_bmp.h"
#include "nge_io_file.h"
#include <stdlib.h>
#include <string.h>

#define BI_RGB          0
#define BI_RLE8         1
#define BI_RLE4         2
#define BI_BITFIELDS    3

#pragma pack(push, 1)
typedef struct tagBITMAPCOREHEADER {
	uint32    bcSize;
	uint16    bcWidth;
	uint16    bcHeight;
	uint16    bcPlanes;
	uint16    bcBitCnt;
} BITMAPCOREHEADER, *PBITMAPCOREHEADER;

typedef struct tagBITMAPINFOHEADER { /* bmih */
	uint32 biSize;
	uint32 biWidth;
	uint32 biHeight;
	uint16 biPlanes;
	uint16 biBitCount;
	uint32 biCompression;
	uint32 biSizeImage;
	uint32 biXPelsPerMeter;
	uint32 biYPelsPerMeter;
	uint32 biClrUsed;
	uint32 biClrImportant;
} BITMAPINFOHEADER,*PBITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER {
	uint16    bfType;
	uint32    bfSize;
	uint16    bfReserved1;
	uint16    bfReserved2;
	uint32    bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;
#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////
//load bmp
//support 24/32 bmp (NOT support 16)
//////////////////////////////////////////////////////////////////////////
image_p image_load_bmp(const char* filename, int displaymode)
{
	image_p pimage = NULL;
	int size ;
	uint8* pbuf;

	int fd = io_fopen(filename,IO_RDONLY);
	if(fd == 0)
		return 0;
	size = io_fsize(fd);
	pbuf = (uint8*)malloc(size);
	io_fread(pbuf,1,size,fd);
	io_fclose(fd);
	pimage = image_load_bmp_buf((const char*)pbuf,size,displaymode);
	SAFE_FREE(pbuf);
	return pimage;
}

image_p image_load_bmp_buf(const char* mbuf,int bsize, int displaymode)
{
	image_p pimage = NULL;
	uint8 *pdata,*data,*line;
	int dsize,w,h,texw,texh,bpb,size,x,y,done = 0;
	uint32* p32;
	uint16* p16;
	uint16 color16;
	uint32 color32;
	PBITMAPFILEHEADER pbfh = (PBITMAPFILEHEADER)mbuf;
	PBITMAPINFOHEADER pbih;

	if(pbfh->bfType !=0x4d42){
		nge_print("not bmp file\n");
		return 0;
	}
	pbih = (PBITMAPINFOHEADER)(mbuf+sizeof(BITMAPFILEHEADER));
	dsize = sizeof(BITMAPFILEHEADER)+pbih->biSize;
	pdata =  (uint8*)mbuf+dsize;
	w = pbih->biWidth;
	h = pbih->biHeight;
	texw = roundpower2(w);
	texh = roundpower2(h);
	bpb  = 4;
	if(displaymode != DISPLAY_PIXEL_FORMAT_8888){
		bpb = 2;
	}
	//int biSizeImage = ((((pbih->biWidth * pbih->biBitCount) + 31) & ~31) / 8) * pbih->biHeight;
	size = texw * texh * bpb;
	data = (uint8*)malloc(size);
	memset(data,0,size);
	p32 = (uint32*)data;
	p16 = (uint16*) p32;
	line = NULL;
	if(pbih->biBitCount == 24){
		for (y = h;y>0;y--){
			line = pdata+(y-1)*w*3;
			for (x=0;x<w;x++){
				switch(displaymode)
				{
				case DISPLAY_PIXEL_FORMAT_8888:
					color32 = MAKE_RGBA_8888(line[2],line[1],line[0],0xff);
					*(p32+x) = color32;
					break;
				case DISPLAY_PIXEL_FORMAT_565:
					color16 = MAKE_RGBA_565(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					color16 = MAKE_RGBA_5551(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					color16 = MAKE_RGBA_4444(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				}
				line+=3;
			}
			p32 += texw;
			p16 += texw;
		}
		done = 1;
	}
	else if(pbih->biBitCount == 32){
		for (y = h;y>0;y--){
			line = pdata+(y-1)*w*4;
			for (x=0;x<w;x++){
				switch(displaymode)
				{
				case DISPLAY_PIXEL_FORMAT_8888:
					color32 = MAKE_RGBA_8888(line[2],line[1],line[0],0xff);
					*(p32+x) = color32;
					break;
				case DISPLAY_PIXEL_FORMAT_565:
					color16 = MAKE_RGBA_565(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					color16 = MAKE_RGBA_5551(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					color16 = MAKE_RGBA_4444(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				}
				line+=4;
			}
			p32 += texw;
			p16 += texw;
		}
		done = 1;
	}

	if (done){
		pimage = (image_p)malloc(sizeof(image_t));
		memset(pimage,0,sizeof(image_t));
		pimage->data = (uint8 *)data;
		pimage->w    = w;
		pimage->h    = h;
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

image_p image_load_bmp_fp(int handle,int fsize, int autoclose,int displaymode)
{
	image_p pimage = NULL;
	char* mbuf;

	if(handle == 0 || fsize == 0)
		return 0;

	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_bmp_buf(mbuf,fsize,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_bmp_colorkey(const char* filename, int displaymode,int colorkey)
{
	image_p pimage = NULL;
	int size;
	uint8* pbuf;
	int fd = io_fopen(filename,IO_RDONLY);

	if(fd == 0)
		return 0;
	size = io_fsize(fd);
	pbuf = (uint8*)malloc(size);
	io_fread(pbuf,1,size,fd);
	io_fclose(fd);
	pimage = image_load_bmp_colorkey_buf((const char*)pbuf,size,displaymode,colorkey);
	SAFE_FREE(pbuf);
	return pimage;
}

image_p image_load_bmp_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey)
{
	image_p pimage = NULL;
	uint8 *pdata,*data,*line;
	int dsize,w,h,texw,texh,bpb,size,x,y,done = 0,pixcolor;
	uint32* p32;
	uint16* p16;
	uint16 color16;
	uint32 color32;
	PBITMAPFILEHEADER pbfh = (PBITMAPFILEHEADER)mbuf;
	PBITMAPINFOHEADER pbih;
	uint8 alpha;

	if(pbfh->bfType !=0x4d42){
		nge_print("not bmp file\n");
		return 0;
	}

	pbih = (PBITMAPINFOHEADER)(mbuf+sizeof(BITMAPFILEHEADER));
	dsize = sizeof(BITMAPFILEHEADER)+pbih->biSize;
	pdata =  (uint8*)mbuf+dsize;
	w = pbih->biWidth;
	h = pbih->biHeight;
	texw = roundpower2(w);
	texh = roundpower2(h);
	bpb  = 4;
	if(displaymode != DISPLAY_PIXEL_FORMAT_8888){
		bpb = 2;
	}
	//int biSizeImage = ((((pbih->biWidth * pbih->biBitCount) + 31) & ~31) / 8) * pbih->biHeight;
	size = texw * texh * bpb;
	data = (uint8*)malloc(size);
	memset(data,0,size);
	p32 = (uint32*)data;
	p16 = (uint16*) p32;
	alpha = 0xff;
	line = NULL;
	if(pbih->biBitCount == 24){
		for (y = h;y>0;y--){
			line = pdata+(y-1)*w*3;
			for (x=0;x<w;x++){
				pixcolor = MAKE_RGB(line[2],line[1],line[0]);
				alpha = 0xff;
				if(colorkey == pixcolor){
					alpha = 0x00;
				}
				switch(displaymode)
				{
				case DISPLAY_PIXEL_FORMAT_8888:
					color32 = MAKE_RGBA_8888(line[2],line[1],line[0],alpha);
					*(p32+x) = color32;
					break;
				case DISPLAY_PIXEL_FORMAT_565:
					color16 = MAKE_RGBA_565(line[2],line[1],line[0],alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					color16 = MAKE_RGBA_5551(line[2],line[1],line[0],alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					color16 = MAKE_RGBA_4444(line[2],line[1],line[0],alpha);
					*(p16+x) = color16;
					break;
				}
				line+=3;
			}
			p32 += texw;
			p16 += texw;
		}
		done = 1;
	}
	else if(pbih->biBitCount == 32){
		for (y = h;y>0;y--){
			line = pdata+(y-1)*w*4;
			for (x=0;x<w;x++){
				pixcolor = MAKE_RGB(line[2],line[1],line[0]);
				alpha = 0xff;
				if(colorkey == pixcolor){
					alpha = 0x00;
				}
				switch(displaymode)
				{
				case DISPLAY_PIXEL_FORMAT_8888:
					color32 = MAKE_RGBA_8888(line[2],line[1],line[0],alpha);
					*(p32+x) = color32;
					break;
				case DISPLAY_PIXEL_FORMAT_565:
					color16 = MAKE_RGBA_565(line[2],line[1],line[0],alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					color16 = MAKE_RGBA_5551(line[2],line[1],line[0],alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					color16 = MAKE_RGBA_4444(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				}
				line+=4;
			}
			p32 += texw;
			p16 += texw;
		}
		done = 1;
	}

	if (done){
		pimage = (image_p)malloc(sizeof(image_t));
		memset(pimage,0,sizeof(image_t));
		pimage->data = (uint8 *)data;
		pimage->w    = w;
		pimage->h    = h;
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

image_p image_load_bmp_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey)
{
	image_p pimage = NULL;
	char *mbuf;

	if(handle == 0 || fsize == 0)
		return 0;

	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_bmp_colorkey_buf(mbuf,fsize,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}
