/***************************************************************************
 *            nge_tga.c
 *
 *  2011/03/27 06:18:18
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
#include "nge_tga.h"
#include "nge_debug_log.h"
#include "nge_io_file.h"
#include "nge_rle.h"

//////////////////////////////////////////////////////////////////////////
//for tga load
//////////////////////////////////////////////////////////////////////////
#define RD16(x) (uint16)((uint16)x[0] | ((uint16)x[1] << 8))
image_p image_load_tga(const char* filename, int displaymode)
{
	image_p pimage = NULL;
	int size;
	char *mbuf;
	int handle;

	handle = io_fopen(filename,IO_RDONLY);
	if(handle == 0)
		return 0;
	size = io_fsize(handle);
	mbuf = (char*) malloc(size);
	io_fread(mbuf,1,size,handle);
	io_fclose(handle);
	pimage = image_load_tga_buf(mbuf,size,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_tga_buf(const char* mbuf,int bsize, int displaymode)
{
	image_p pimage = NULL;
	char *workptr;
	sint32 width,height,bytesperline,rlesize;
	uint32 texw,texh,fpos,color32,col32;
	uint8 *tdata,*dst,*rle,*src,*xsrc;
	uint8 done,bpb;
	char TRUEVIS[] = "TRUEVISION-XFILE.";
	long extoffs,devoffs;
	uint32 *p32;
	uint16 *p16;
	int x = 0,y = 0,size;
	sint32 ddelta,hdelta,vdelta;
	uint16 color16;
	static uint8 r,g,b,a;
	TGAFILEHEADER tfh;

	offset = 0;
	if (mbuf==NULL){
		return 0;
	}
    memset( &tfh, 0, sizeof(TGAFILEHEADER) );
    fpos = 0;
	workptr = (char*)mbuf;

	memcpy(&tfh,workptr,sizeof(TGAFILEHEADER));
    fpos += sizeof(TGAFILEHEADER)+tfh.ImageIDSize;

	workptr += fpos;

    if (tfh.ImageTypeCode!=1 && tfh.ImageTypeCode!=2 && tfh.ImageTypeCode!=3 &&
		tfh.ImageTypeCode!=9 && tfh.ImageTypeCode!=10 && tfh.ImageTypeCode!=11)
	{
		nge_print("Unknown ImageTypeCode.\n");
		return 0;
	}
	if (tfh.Depth!=8 && tfh.Depth!=15 && tfh.Depth!=16 && tfh.Depth!=24 && tfh.Depth!=32)
	{
		nge_print("Unknown Bit Depth.\n");
		return 0;
	}

	width = RD16(tfh.Width);// - RD16(tfh.OriginX);
	height = RD16(tfh.Height);// - RD16(tfh.OriginY);
	texw = roundpower2(width);
	texh = roundpower2(height);
	bytesperline = ((width*tfh.Depth)>>3);
	tdata = (uint8*) malloc( height * bytesperline );
	dst = NULL;
	done = 0;
	bpb = 4;
    if (tdata==0){
    	nge_print("malloc failed on tdata.\n");
		return 0;
	}

	if ( (tfh.ImageTypeCode&8) == 8 && (tfh.ImageTypeCode&3) > 0 )
	{
		// READ RLE ENCODED
		rlesize = bsize-fpos;
		rle = (uint8*) malloc( rlesize );
		if (rle==0)
		{
			free( tdata );
			return 0 ;
		}

		memcpy(rle,workptr,rlesize);
		workptr += rlesize;

		if (memcmp( &rle[rlesize-26+8], TRUEVIS, 18 )==0)
		{
			// TRUEVISION-XFILE and may contain Developer and Extension Areas
			rlesize -= 26;
			extoffs = rle[rlesize+0] + (rle[rlesize+1]<<8) + (rle[rlesize+2]<<16) + (rle[rlesize+3]<<24);
			devoffs = rle[rlesize+4] + (rle[rlesize+5]<<8) + (rle[rlesize+6]<<16) + (rle[rlesize+7]<<24);
			if (extoffs!=0 || devoffs!=0)
			{
				// contains developer and/or extension area :/
				// For now we just assume that the developer or extension area is at the start of the whole extension block
				// Actually, we'd need to go through all following area blocks and find the one that comes first (lowest offset)
				if (devoffs<extoffs)
					rlesize = devoffs-fpos;
				else
					rlesize = extoffs-fpos;
			}
		}
		// DECODE RLE:
		decodeRLE( rle, rlesize, tdata, height * bytesperline, tfh.Depth );
		SAFE_FREE( rle );
	}
	else if ( ( tfh.ImageTypeCode == 1 ) || ( tfh.ImageTypeCode == 2 ) || ( tfh.ImageTypeCode == 3 ) )
	{
		memcpy(tdata,workptr,height*bytesperline);
		workptr += height*bytesperline;

	}
	else
	{
		SAFE_FREE( tdata );
		nge_print("Unknown ImageTypeCode.\n");
		return 0;
	}


	if((tfh.Depth==24)||(tfh.Depth==32)){
		bpb = 4;
		if(displaymode !=DISPLAY_PIXEL_FORMAT_8888){
			bpb = 2;
		}
		size = texw * texh * bpb;
		dst = (uint8*)malloc(size);
		memset(dst,0,size);
		src = (uint8*)tdata;
		ddelta = (texw-width)*bpb;
		hdelta = bytesperline;
		if ((tfh.ImageDescrip & TGA_DESC_HORIZONTAL) == 0){
			src += (height-1) * bytesperline;
			hdelta = -bytesperline;
		}
		vdelta = (tfh.Depth >> 3);
		if ((tfh.ImageDescrip & TGA_DESC_VERTICAL) != 0){
			src += bytesperline-(tfh.Depth >> 3);
			vdelta = -(tfh.Depth >> 3);
		}

		// convert the crappy image and do flipping if neccessary

		switch (tfh.Depth)
		{
			case 8:
			case 15:
			case 16:
				break;
			case 24:
				p32 = (uint32*) dst;
				p16 = (uint16*) p32;
				for (y = 0; y < (int)height; y++){
					xsrc = src;
					for (x = 0; x < (int)width; x++)  {

						col32 = (uint32)(((uint32)xsrc[2]<<16) | ((uint32)xsrc[1]<<8) | (uint32)xsrc[0] | (0xFF<<24));
						//BGRA
						a = (col32 >> 24) & 0xff;
						b = col32 & 0xff;
						g = (col32 >> 8) & 0xff;
						r = (col32 >> 16) & 0xff;
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
						xsrc += vdelta;
					}
					src += hdelta;
					p32 += texw;
					p16 += texw;
				}
				break;
			case 32:
				p32 = (uint32*) dst;
				p16 = (uint16*) p32;
				for (y = 0; y < (int)height; y++){
					xsrc = src;
					for (x = 0; x < (int)width; x++)  {
						col32 = *(uint32*)(xsrc);
						//BGRA
						a = (col32 >> 24) & 0xff;
						b = col32 & 0xff;
						g = (col32 >> 8) & 0xff;
						r = (col32 >> 16) & 0xff;
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
						xsrc += vdelta;
					}//end for
					src += hdelta;
					p32 += texw;
					p16 += texw;
			   }//end for
			}//end switch
		done = 1;
	}//end if

	if (done){
		pimage = (image_p)malloc(sizeof(image_t));
		memset(pimage,0,sizeof(image_t));
		pimage->data = (uint8 *)dst;
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

	SAFE_FREE(tdata);
	return pimage;
}

image_p image_load_tga_fp(int handle,int fsize, int autoclose,int displaymode)
{
	image_p pimage = NULL;
	char* mbuf;

	if(handle == 0 || fsize == 0)
		return 0;

	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_tga_buf(mbuf,fsize,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}



image_p image_load_tga_colorkey(const char* filename, int displaymode,int colorkey)
{
	image_p pimage;
	int size;
	char *mbuf;
	int handle = io_fopen(filename,IO_RDONLY);

	if(handle == 0)
		return 0;
	size= io_fsize(handle);
	mbuf = (char*) malloc(size);
	io_fread(mbuf,1,size,handle);
	io_fclose(handle);
	pimage = image_load_tga_colorkey_buf(mbuf,size,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_tga_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey)
{
	image_p pimage = NULL;
	char *workptr;
	sint32 width,height,bytesperline,rlesize,pixcolor;
	uint32 texw,texh,fpos,color32,col32;
	uint8 *tdata,*dst,*rle,*src,*xsrc;
	uint8 done,bpb;
	char TRUEVIS[] = "TRUEVISION-XFILE.";
	long extoffs,devoffs;
	uint32 *p32;
	uint16 *p16;
	int x = 0,y = 0,size;
	sint32 ddelta,hdelta,vdelta;
	uint16 color16;
	static uint8 r,g,b,a;
	TGAFILEHEADER tfh;

	if (mbuf==NULL){
		return 0;
	}

	offset = 0;
    memset( &tfh, 0, sizeof(TGAFILEHEADER) );
    fpos  = 0;
	workptr = (char*)mbuf;

	memcpy(&tfh,workptr,sizeof(TGAFILEHEADER));
    fpos += sizeof(TGAFILEHEADER)+tfh.ImageIDSize;

	workptr += fpos;

    if (tfh.ImageTypeCode!=1 && tfh.ImageTypeCode!=2 && tfh.ImageTypeCode!=3 &&
		tfh.ImageTypeCode!=9 && tfh.ImageTypeCode!=10 && tfh.ImageTypeCode!=11)
	{
		nge_print("Unknown ImageTypeCode.\n");
		return 0;
	}
	if (tfh.Depth!=8 && tfh.Depth!=15 && tfh.Depth!=16 && tfh.Depth!=24 && tfh.Depth!=32)
	{
		nge_print("Unknown Bit Depth.\n");
		return 0;
	}

	width = RD16(tfh.Width);// - RD16(tfh.OriginX);
	height = RD16(tfh.Height);// - RD16(tfh.OriginY);
	texw = roundpower2(width);
	texh = roundpower2(height);
	bytesperline = ((width*tfh.Depth)>>3);
	tdata = (uint8*) malloc( height * bytesperline );
	dst = NULL;
	done = 0;
	bpb = 4;
    if (tdata == 0){
    	nge_print("malloc failed on tdata.\n");
		return 0;
	}

	if ( (tfh.ImageTypeCode&8) == 8 && (tfh.ImageTypeCode&3) > 0 )
	{
		// READ RLE ENCODED
		rlesize = bsize-fpos;
		rle = (uint8*) malloc( rlesize );
		if (rle==0)
		{
			free( tdata );
			return 0 ;
		}

		memcpy(rle,workptr,rlesize);
		workptr += rlesize;

		if (memcmp( &rle[rlesize-26+8], TRUEVIS, 18 )==0)
		{
			// TRUEVISION-XFILE and may contain Developer and Extension Areas
			rlesize -= 26;
			extoffs = rle[rlesize+0] + (rle[rlesize+1]<<8) + (rle[rlesize+2]<<16) + (rle[rlesize+3]<<24);
			devoffs = rle[rlesize+4] + (rle[rlesize+5]<<8) + (rle[rlesize+6]<<16) + (rle[rlesize+7]<<24);
			if (extoffs!=0 || devoffs!=0)
			{
				// contains developer and/or extension area :/
				// For now we just assume that the developer or extension area is at the start of the whole extension block
				// Actually, we'd need to go through all following area blocks and find the one that comes first (lowest offset)
				if (devoffs<extoffs)
					rlesize = devoffs-fpos;
				else
					rlesize = extoffs-fpos;
			}
		}
		// DECODE RLE:
		decodeRLE( rle, rlesize, tdata, height * bytesperline, tfh.Depth );
		SAFE_FREE( rle );
	}
	else if ( ( tfh.ImageTypeCode == 1 ) || ( tfh.ImageTypeCode == 2 ) || ( tfh.ImageTypeCode == 3 ) )
	{
		memcpy(tdata,workptr,height*bytesperline);
		workptr += height*bytesperline;

	}
	else
	{
		SAFE_FREE( tdata );
		nge_print("Unknown ImageTypeCode.\n");
		return 0;
	}


	if((tfh.Depth==24)||(tfh.Depth==32)){
		bpb = 4;
		if(displaymode !=DISPLAY_PIXEL_FORMAT_8888){
			bpb = 2;
		}
		size = texw * texh * bpb;
		dst = (uint8*)malloc(size);
		memset(dst,0,size);
		src = (uint8*)tdata;

		ddelta = (texw-width)*bpb;

		hdelta = bytesperline;
		if ((tfh.ImageDescrip & TGA_DESC_HORIZONTAL) == 0){
			src += (height-1) * bytesperline;
			hdelta = -bytesperline;
		}

		vdelta = (tfh.Depth >> 3);
		if ((tfh.ImageDescrip & TGA_DESC_VERTICAL) != 0){
			src += bytesperline-(tfh.Depth >> 3);
			vdelta = -(tfh.Depth >> 3);
		}

		// convert the crappy image and do flipping if neccessary
		switch (tfh.Depth)
		{
			case 8:
			case 15:
			case 16:
				break;
			case 24:
				p32 = (uint32*) dst;
				p16 = (uint16*) p32;
				for (y = 0; y < (int)height; y++){
					xsrc = src;
					for (x = 0; x < (int)width; x++)  {

						col32 = (uint32)(((uint32)xsrc[2]<<16) | ((uint32)xsrc[1]<<8) | (uint32)xsrc[0] | (0xFF<<24));
						//BGRA
						a = (col32 >> 24) & 0xff;
						b = col32 & 0xff;
						g = (col32 >> 8) & 0xff;
						r = (col32 >> 16) & 0xff;
						pixcolor = MAKE_RGB(r,g,b);
						if(colorkey == pixcolor){
							a = 0x00;
						}
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
						xsrc += vdelta;
					}
					src += hdelta;
					p32 += texw;
					p16 += texw;
				}
				break;
			case 32:
				p32 = (uint32*) dst;
				p16 = (uint16*) p32;
				for (y = 0; y < (int)height; y++){
					xsrc = src;
					for (x = 0; x < (int)width; x++)  {
						col32 = *(uint32*)(xsrc);
						//BGRA
						a = (col32 >> 24) & 0xff;
						b = col32 & 0xff;
						g = (col32 >> 8) & 0xff;
						r = (col32 >> 16) & 0xff;
						pixcolor = MAKE_RGB(r,g,b);
						if(colorkey == pixcolor){
							a = 0x00;
						}
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
						xsrc += vdelta;
					}//end for
					src += hdelta;
					p32 += texw;
					p16 += texw;
			   }//end for
			}//end switch
		done = 1;
	}//end if

	if (done){
		pimage = (image_p)malloc(sizeof(image_t));
		memset(pimage,0,sizeof(image_t));
		pimage->data = (uint8 *)dst;
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

	SAFE_FREE(tdata);
	return pimage;
}

image_p image_load_tga_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey)
{
	image_p pimage = NULL;
	char *mbuf;

	if(handle == 0 || fsize == 0)
		return 0;

	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_tga_colorkey_buf(mbuf,fsize,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}

//////////////////////////////////////////////////////////////////////////
//to tga
//////////////////////////////////////////////////////////////////////////
int image_save_tga(image_p pimage,const char* filename,uint8 alpha,uint8 rle)
{
	int fd;
	uint8 *line,*rleline;
	TGAFILEHEADER tfh;
	uint32 x,y;
	static uint8 b,g,r,a;
	long pos;
	uint8 *src;
	uint16 col16;

	uint8* save_buf = NULL;
	int   save_pos = 0;
	if (pimage == 0)
		return 0;
	if (pimage->data==0 || pimage->w==0 || pimage->h==0)
		return 0;
	fd = io_fopen(filename, IO_WRONLY);
	if (!fd)
		return 0;
    memset( &tfh, 0, sizeof(TGAFILEHEADER) );
	tfh.ImageTypeCode = 2;
	tfh.Depth = (alpha?32:24);
	if (rle)
		tfh.ImageTypeCode |= 8;
	tfh.ImageDescrip = TGA_DESC_HORIZONTAL;
	if (alpha)
		tfh.ImageDescrip |= TGA_DESC_ABITS;
	tfh.Width[1] = pimage->w/256;
	tfh.Width[0] = pimage->w%256;
	tfh.Height[1] = pimage->h/256;
	tfh.Height[0] = pimage->h%256;
	//io_fwrite( &tfh, 1, sizeof(tfh), fd );

	line = (uint8*) malloc(pimage->w * 4);
	if (line == 0){
		io_fclose(fd);
		return 0;
	}
	save_buf = (char*)malloc(pimage->w*pimage->h*4+sizeof(TGAFILEHEADER));
	memset(save_buf,0,pimage->w*pimage->h*4+sizeof(TGAFILEHEADER));
	memcpy(save_buf+save_pos,&tfh,sizeof(TGAFILEHEADER));
	save_pos += sizeof(TGAFILEHEADER);
	memset( line, 0, pimage->w*4 );

	rleline = 0;
	if (rle){
		rleline = (uint8*) malloc(pimage->w * 6);
	}

	src = (uint8*)pimage->data;

	for (y = 0;y<pimage->h;y++)
	{
		switch (pimage->dtype)
		{
			case DISPLAY_PIXEL_FORMAT_5551:
				if (alpha)
					for (x=0;x<pimage->w;x++){
						col16 = (src[0 + x*2] | (src[1 + x*2] << 8));
						GET_RGBA_5551(col16,r,g,b,a);
						line[0 + x*4] = b;
						line[1 + x*4] = g;
						line[2 + x*4] = r;
						line[3 + x*4] = a;
					}
				else
					for (x=0;x<pimage->w;x++){
						col16 = (src[0 + x*2] | (src[1 + x*2] << 8));
						GET_RGBA_5551(col16,r,g,b,a);
						line[0 + x*3] = b;
						line[1 + x*3] = g;
						line[2 + x*3] = r;
					}
				break;
			case DISPLAY_PIXEL_FORMAT_565:
				if (alpha)
					for (x=0;x<pimage->w;x++){
						col16 = (src[0 + x*2] | (src[1 + x*2] << 8));
						GET_RGBA_565(col16,r,g,b,a);
						line[0 + x*4] = b;
						line[1 + x*4] = g;
						line[2 + x*4] = r;
						line[3 + x*4] = a;
					}
					else
						for (x=0;x<pimage->w;x++){
							col16 = (src[0 + x*2] | (src[1 + x*2] << 8));
							GET_RGBA_565(col16,r,g,b,a);
							line[0 + x*3] = b;
							line[1 + x*3] = g;
							line[2 + x*3] = r;
						}
				break;
			case DISPLAY_PIXEL_FORMAT_8888:
				if (alpha)
					for (x=0;x<pimage->w;x++){
						line[0 + x*4] = src[2 + x*4];
						line[1 + x*4] = src[1 + x*4];
						line[2 + x*4] = src[0 + x*4];
						line[3 + x*4] = src[3 + x*4];
					}
				else
					for (x=0;x<pimage->w;x++){
						line[0 + x*3] = src[2 + x*4];
						line[1 + x*3] = src[1 + x*4];
						line[2 + x*3] = src[0 + x*4];
					}
				break;
			case DISPLAY_PIXEL_FORMAT_4444:
				if (alpha)
					for (x=0;x<pimage->w;x++){
						col16 = (src[0 + x*2] | (src[1 + x*2] << 8));
						GET_RGBA_4444(col16,r,g,b,a);
						line[0 + x*4] = b;
						line[1 + x*4] = g;
						line[2 + x*4] = r;
						line[3 + x*4] = a;
					}
				else
					for (x=0;x<pimage->w;x++){
						col16 = (src[0 + x*2] | (src[1 + x*2] << 8));
						GET_RGBA_4444(col16,r,g,b,a);
						line[0 + x*3] = b;
						line[1 + x*3] = g;
						line[2 + x*3] = r;
					}
				break;
		}
		if (rle)
		{
			long sz = encodeRLE( line, pimage->w*tfh.Depth>>3, 0, rleline, pimage->w*6, tfh.Depth );
			//io_fwrite( rleline, 1, sz, fd );
			memcpy(save_buf+save_pos,rleline,sz);
			save_pos += sz;
		}
		else{
			io_fwrite( line, 1, pimage->w*tfh.Depth>>3, fd );
			memcpy(save_buf+save_pos,line,pimage->w*tfh.Depth>>3);
			save_pos += pimage->w*tfh.Depth>>3;
		}
		src += pimage->texw*pimage->bpb;
	}
	if (rle)
		SAFE_FREE(rleline);
	SAFE_FREE(line);
	io_fwrite(save_buf,1,save_pos,fd);
	free(save_buf);
	io_fclose(fd);
	return 1;
}
