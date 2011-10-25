#include "nge_debug_log.h"
#include "nge_tga.h"
#include "nge_bmp.h"
#include "nge_png.h"
#include "nge_jpg.h"
#include "nge_rle.h"
#include "nge_io_file.h"
#include <stdlib.h>
#include <string.h>

uint32 image_tid = 0;

#if defined(NGE_PSP)
static void swizzle_fast(uint8* out, const uint8* in, unsigned int width, unsigned int height)
{
	unsigned int blockx, blocky;
	unsigned int j;

	unsigned int width_blocks = (width / 16);
	unsigned int height_blocks = (height / 8);

	unsigned int src_pitch = (width-16)/4;
	unsigned int src_row = width * 8;
	const uint8 *xsrc;
	const uint32 *src;
	const uint8 *ysrc = in;
	uint32 *dst = (uint32*)out;

	for (blocky = 0; blocky < height_blocks; ++blocky)
	{
		xsrc = ysrc;
		for (blockx = 0; blockx < width_blocks; ++blockx)
		{
			src = (uint32*)xsrc;
			for (j = 0; j < 8; ++j)
			{
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				src += src_pitch;
			}
			xsrc += 16;
		}
		ysrc += src_row;
	}
}

//Thanks to Raphael
static void unswizzle_fast(const uint8* out, const uint8* in, const int width, const int height)
{
	int blockx, blocky;
	int j;

	int width_blocks = (width / 16);
	int height_blocks = (height / 8);

	int dst_pitch = (width-16)/4;
	int dst_row = width * 8;

	uint32* src = (uint32*)in;
	uint8* ydst = (uint8*)out;
	sceKernelDcacheWritebackAll();
	for (blocky = 0; blocky < height_blocks; ++blocky)
	{
		uint8* xdst = ydst;
		for (blockx = 0; blockx < width_blocks; ++blockx)
		{
			uint32* dst;
			if ((uint32)out <= 0x04200000)
				dst = (uint32*)((uint32)xdst | 0x40000000);
			else
				dst = (uint32*)xdst;
			for (j = 0; j < 8; ++j)
			{
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				dst += dst_pitch;
			}
			xdst += 16;
		}
		ydst += dst_row;
	}
}

void swizzle_swap(image_p pimage)
{
	int bsize = pimage->texw*pimage->texh*pimage->bpb;
	uint8* buffer = (uint8*)malloc(bsize);
	memset(buffer,0,bsize);
	swizzle_fast(buffer,pimage->data,pimage->texw*pimage->bpb,pimage->texh);
	SAFE_FREE(pimage->data);
	pimage->data = buffer;
	pimage->swizzle = 1;
}

void unswizzle_swap(image_p pimage)
{
	int bsize = pimage->texw*pimage->texh*pimage->bpb;
	uint8* buffer = (uint8*)malloc(bsize);
	memset(buffer,0,bsize);
	unswizzle_fast(buffer,pimage->data,pimage->texw*pimage->bpb,pimage->texh);
	SAFE_FREE(pimage->data);
	pimage->data = buffer;
	pimage->swizzle = 0;
}
#endif

int CreateColor(uint8 r,uint8 g,uint8 b,uint8 a,int dtype)
{
	int color = 0;
	switch(dtype)
	{
	case DISPLAY_PIXEL_FORMAT_565:
		color = MAKE_RGBA_565(r,g,b,a);
		break;
	case DISPLAY_PIXEL_FORMAT_5551:
		color = MAKE_RGBA_5551(r,g,b,a);
		break;
	case DISPLAY_PIXEL_FORMAT_4444:
		color = MAKE_RGBA_4444(r,g,b,a);
		break;
	case DISPLAY_PIXEL_FORMAT_8888:
		color = MAKE_RGBA_8888(r,g,b,a);
		break;
	default:
		color = MAKE_RGBA_8888(r,g,b,a);
	}
	return color;
}

int GET_PSM_COLOR_MODE(int dtype)
{
	int psm = 0;
	switch(dtype)
	{
		case DISPLAY_PIXEL_FORMAT_565:
			psm = PSM_565;
			break;
		case DISPLAY_PIXEL_FORMAT_5551:
			psm = PSM_5551;
			break;
		case DISPLAY_PIXEL_FORMAT_4444:
			psm = PSM_4444;
			break;
		case DISPLAY_PIXEL_FORMAT_8888:
			psm = PSM_8888;
			break;
		default:
			psm = PSM_8888;
	}
	return psm;
}

int roundpower2(int width)
{
	int b = width;
	int n;
	for (n = 0; b != 0; n++) b >>= 1;
	b = 1 << n;
	if (b == 2 * width) b >>= 1;
	return b;
}

//////////////////////////////////////////////////////////////////////////
//save image_p
//support:png tga
//////////////////////////////////////////////////////////////////////////
int image_save(image_p pimage,const char* filename,uint8 alpha,uint8 rle)
{
	char* p = strrchr(filename, '.');
	if(!strcmp(p+1, "tga"))
		return image_save_tga(pimage, filename, alpha, rle);
	return image_save_png(pimage, filename, alpha);
}

image_p image_load(const char* filename, int displaymode,int swizzle)
{
	image_p pimage = NULL;
	char flags[12]={0};
	int fd = io_fopen(filename,IO_RDONLY);

	if(fd ==0 )
		return 0;
	io_fread(flags,1,12,fd);
	io_fclose(fd);
	if(flags[0]==(char)0x89&&flags[1]=='P'&&flags[2]=='N'&&flags[3]=='G'){
		pimage = image_load_png(filename,displaymode);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(flags[0]=='B'&&flags[1]=='M'){
		pimage = image_load_bmp(filename,displaymode);
		if(pimage == NULL){
			nge_print("bmp file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(flags[6]=='J'&&flags[7]=='F'&&flags[8]=='I'){
		pimage = image_load_jpg(filename,displaymode);
		if(pimage == NULL){
			nge_print("jpg file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(flags[2]==(char)0x02||flags[2]==(char)0x0a){
		pimage = image_load_tga(filename,displaymode);
		if(pimage == NULL){
			nge_print("tga file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else{
		nge_print("not support file type!\n");
	}
	return 0;
}

image_p image_load_buf(const char* mbuf,int bsize, int displaymode,int swizzle)
{
	image_p pimage = NULL;
	if(bsize < 12||mbuf == 0)
		return 0;
	if(mbuf[0]==(char)0x89&&mbuf[1]=='P'&&mbuf[2]=='N'&&mbuf[3]=='G'){
		pimage = image_load_png_buf(mbuf,bsize,displaymode);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(mbuf[0]=='B'&&mbuf[1]=='M'){
		pimage = image_load_bmp_buf(mbuf,bsize,displaymode);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(mbuf[6]=='J'&&mbuf[7]=='F'&&mbuf[8]=='I'){
		pimage =  image_load_jpg_buf(mbuf,bsize,displaymode);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(mbuf[2]==(char)0x02||mbuf[2]==(char)0x0a){
		pimage =  image_load_tga_buf(mbuf,bsize,displaymode);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else{
		nge_print("not support file type!\n");
	}
	return 0;
}

image_p image_load_fp(int handle,int fsize, int autoclose,int displaymode,int swizzle)
{
	image_p pimage = NULL;
	char *mbuf;

	if(handle == 0 || fsize == 0)
		return 0;

	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_buf(mbuf,fsize,displaymode,swizzle);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_colorkey(const char* filename, int displaymode,int colorkey,int swizzle)
{
	image_p pimage = NULL;
	char flags[12]={0};
	int fd = io_fopen(filename,IO_RDONLY);

	if(fd==0)
		return 0;
	io_fread(flags,1,12,fd);
	io_fclose(fd);
	if(flags[0]==(char)0x89&&flags[1]=='P'&&flags[2]=='N'&&flags[3]=='G'){
		pimage = image_load_png_colorkey(filename,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(flags[0]=='B'&&flags[1]=='M'){
		pimage =  image_load_bmp_colorkey(filename,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(flags[6]=='J'&&flags[7]=='F'&&flags[8]=='I'){
		pimage = image_load_jpg_colorkey(filename,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(flags[2]==(char)0x02||flags[2]==(char)0x0a){
		pimage =  image_load_tga_colorkey(filename,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else{
		nge_print("not support file type!\n");
	}
	return 0;
}

image_p image_load_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey,int swizzle)
{
	image_p pimage = NULL;
	if(bsize < 12||mbuf == 0)
		return 0;
	if(mbuf[0]==(char)0x89&&mbuf[1]=='P'&&mbuf[2]=='N'&&mbuf[3]=='G'){

		pimage =  image_load_png_colorkey_buf(mbuf,bsize,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(mbuf[0]=='B'&&mbuf[1]=='M'){
		//printf("bmp\n");
		pimage =  image_load_bmp_colorkey_buf(mbuf,bsize,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(mbuf[6]=='J'&&mbuf[7]=='F'&&mbuf[8]=='I'){
		//printf("jpg\n");
		pimage =  image_load_jpg_colorkey_buf(mbuf,bsize,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(mbuf[2]==(char)0x02||mbuf[2]==(char)0x0a){
		//printf("tga\n");
		pimage =  image_load_tga_colorkey_buf(mbuf,bsize,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	return 0;
}

image_p image_load_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey,int swizzle)
{
	image_p pimage = NULL;
	char *mbuf;

	if(handle == 0 || fsize == 0)
		return 0;

	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_colorkey_buf(mbuf,fsize,displaymode,colorkey,swizzle);
	SAFE_FREE(mbuf);
	return pimage;
}



image_p image_create(int w,int h,int displaymode)
{
	image_p pimage = (image_p)malloc(sizeof(image_t));
	int size;
	memset(pimage,0,sizeof(image_t));
	pimage->w = w;
	pimage->h = h;
	pimage->texw = roundpower2(w);
	pimage->texh = roundpower2(h);
	pimage->bpb  = (displaymode==DISPLAY_PIXEL_FORMAT_8888)?4:2;
	pimage->dtype = displaymode;
	pimage->rcentrex = pimage->w*1.0f/2;
	pimage->rcentrey = pimage->h*1.0f/2;
	pimage->mode = GET_PSM_COLOR_MODE(displaymode);
	pimage->mask = CreateColor(255,255,255,255,displaymode);
	pimage->texid = image_tid++;
	size = pimage->texw*pimage->texh*pimage->bpb;
	pimage->data = (uint8*)malloc(size);
	memset(pimage->data,0,size);
	return pimage;
}

image_p image_create_ex(int w,int h,int color,int displaymode)
{
	image_p pimage = (image_p)malloc(sizeof(image_t));
	int size,i,j;
	uint16* img16;
	uint32* img32;
	memset(pimage,0,sizeof(image_t));
	pimage->w = w;
	pimage->h = h;
	pimage->texw = roundpower2(w);
	pimage->texh = roundpower2(h);
	pimage->bpb  = (displaymode==DISPLAY_PIXEL_FORMAT_8888)?4:2;
	pimage->dtype = displaymode;
	pimage->rcentrex = pimage->w*1.0f/2;
	pimage->rcentrey = pimage->h*1.0f/2;
	pimage->mode = GET_PSM_COLOR_MODE(displaymode);
	pimage->mask = CreateColor(255,255,255,255,displaymode);
	pimage->texid = image_tid++;
	size = pimage->texw*pimage->texh*pimage->bpb;
	pimage->data = (uint8*)malloc(size);
	memset(pimage->data,0,size);
	if(displaymode==DISPLAY_PIXEL_FORMAT_8888){
		img32 = (uint32*)pimage->data;
		for(i=0;i<pimage->h;i++)
			for(j=0;j<pimage->w;j++){
				*(img32+i*pimage->texw+j) = color;
			}
	}
	else{
		img16 = (uint16*)pimage->data;
		for(i=0;i<pimage->h;i++)
			for(j=0;j<pimage->w;j++){
				*(img16+i*pimage->texw+j) = (uint16)(color&0xffff);
			}
	}
	return pimage;
}


void image_free(image_p pimage)
{
	if(pimage){
		SAFE_FREE(pimage->data);
		SAFE_FREE(pimage);
	}
}

void image_clear(image_p pimage)
{
	memset(pimage->data,0,pimage->texw*pimage->texh*pimage->bpb);
	pimage->modified = 1;
}

image_p image_clone(image_p pimage)
{
	int size;
	image_p clone = (image_p)malloc(sizeof(image_t));

	memcpy(clone,pimage,sizeof(image_t));
	clone->texid = image_tid++;
	size = pimage->texw*pimage->texh*pimage->bpb;
	clone->data = (uint8*)malloc(size);
	memcpy(clone->data,pimage->data,size);
	return clone;
}


/*alpha blend!
 *0 完全透明，255不透明
 *Dst=( Src0*(255-Alpha) + Src1*Alpha ) / 255
 *#define MAKEALPHA(SRC,DES,ALPHA) (( SRC*(255-ALPHA) + DES*ALPHA ) /255)*/
#define MAKEALPHA(SRC,DES,ALPHA) (( SRC*ALPHA + DES*(255-ALPHA) ) /255)

void image_to_image_alpha_ex(const image_p src,const image_p des,uint32 sx,uint32 sy,uint32 sw,uint32 sh,uint32 dx,uint32 dy,int alpha)
{
	uint8 sr,sg,sb,sa;
	uint8 dr,dg,db,da;
	uint32 i,j;
	uint16 *cpbegin16;
	uint16 *bmp16;

	uint32 *cpbegin32,*bmp32;
	if(src->swizzle ==1)
		unswizzle_swap(src);
	if(des->swizzle ==1)
		unswizzle_swap(des);
	des->modified = 1;
	if(sw == 0 && sh == 0){
		sw = src->w;
		sh = src->h;
	}

	if(des->dtype==DISPLAY_PIXEL_FORMAT_4444){
		cpbegin16 = (uint16*)des->data+dy*des->texw+dx;
		bmp16 = (uint16*)src->data+sy*src->texw+sx;
		for(i = 0;i<sh;i++){
			for(j = 0;j<sw;j++){
				if((j+dx>des->texw)||(i+dy>des->texh))
					continue;
				GET_RGBA_4444(bmp16[j],sr,sg,sb,sa);
				if(alpha == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin16[j] = bmp16[j];
				}
				else{
					GET_RGBA_4444(cpbegin16[j],dr,dg,db,da);
					cpbegin16[j] = MAKE_RGBA_4444(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin16 += des->texw;
			bmp16     += src->texw;
		}

	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_5551){
		cpbegin16 = (uint16*)des->data+dy*des->texw+dx;
		bmp16 = (uint16*)src->data+sy*src->texw+sx;
		for(i = 0;i<sh;i++){
			for(j = 0;j<sw;j++){
				if((j+dx>des->texw)||(i+dy>des->texh))
					continue;
				GET_RGBA_5551(bmp16[j],sr,sg,sb,sa);
				if(alpha == 0||sa == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin16[j] = bmp16[j];
				}
				else{
					GET_RGBA_5551(cpbegin16[j],dr,dg,db,da);
					cpbegin16[j] = MAKE_RGBA_5551(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin16 += des->texw;
			bmp16     += src->texw;
		}

	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_565){
		cpbegin16 = (uint16*)des->data+dy*des->texw+dx;
		bmp16 = (uint16*)src->data+sy*src->texw+sx;
		for(i = 0;i<sh;i++){
			for(j = 0;j<sw;j++){
				if((j+dx>des->texw)||(i+dy>des->texh))
					continue;
				GET_RGBA_565(bmp16[j],sr,sg,sb,sa);
				if(alpha == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin16[j] = bmp16[j];
				}
				else{
					GET_RGBA_565(cpbegin16[j],dr,dg,db,da);
					cpbegin16[j] = MAKE_RGBA_565(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin16 += des->texw;
			bmp16     += src->texw;
		}

	}
	else{
		cpbegin32 = (uint32*)des->data+dy*des->texw+dx;
		bmp32 = (uint32*)src->data+sy*src->texw+sx;
		for(i = 0;i<sh;i++){
			for(j = 0;j<sw;j++){
				if((j+dx>des->texw)||(i+dy>des->texh))
					continue;
				GET_RGBA_8888(bmp32[j],sr,sg,sb,sa);
				if(alpha == 0||sa == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin32[j] = bmp32[j];
				}
				else{
					GET_RGBA_8888(cpbegin32[j],dr,dg,db,da);
					cpbegin32[j] = MAKE_RGBA_8888(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin32 += des->texw;
			bmp32     += src->texw;
		}
	}

}

void image_to_image_alpha(const image_p src,const image_p des,uint32 x,uint32 y,int alpha)
{
	uint8 sr,sg,sb,sa;
	uint8 dr,dg,db,da;
	uint32 i,j;
	uint16 *cpbegin16;
	uint16 *bmp16;
	uint32 w = src->w;
	uint32 h = src->h;
	uint32 *cpbegin32,*bmp32;

	if(src->swizzle ==1)
		unswizzle_swap(src);
	if(des->swizzle ==1)
		unswizzle_swap(des);
	des->modified = 1;
	if(des->dtype==DISPLAY_PIXEL_FORMAT_4444){
		cpbegin16 = (uint16*)des->data+y*des->texw+x;
		bmp16 = (uint16*)src->data;
		for(i =0;i<h;i++){
			for(j =0;j<w;j++){
				if((j+x>des->texw)||(i+y>des->texh))
					continue;
				GET_RGBA_4444(bmp16[i*src->texw+j],sr,sg,sb,sa);
				if(alpha == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin16[j] = bmp16[i*src->texw+j];
				}
				else{
					GET_RGBA_4444(cpbegin16[j],dr,dg,db,da);
					cpbegin16[j] = MAKE_RGBA_4444(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin16 += des->texw;
		}

	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_5551){
		cpbegin16 = (uint16*)des->data+y*des->texw+x;
		bmp16 = (uint16*)src->data;
		for(i =0;i<h;i++){
			for(j =0;j<w;j++){
				if((j+x>des->texw)||(i+y>des->texh))
					continue;
				GET_RGBA_5551(bmp16[i*src->texw+j],sr,sg,sb,sa);
				if(alpha == 0||sa == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin16[j] = bmp16[i*src->texw+j];
				}
				else{
					GET_RGBA_5551(cpbegin16[j],dr,dg,db,da);
					cpbegin16[j] = MAKE_RGBA_5551(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin16 += des->texw;
		}

	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_565){
		cpbegin16 = (uint16*)des->data+y*des->texw+x;
		bmp16 = (uint16*)src->data;
		for(i =0;i<h;i++){
			for(j =0;j<w;j++){
				if((j+x>des->texw)||(i+y>des->texh))
					continue;
				GET_RGBA_565(bmp16[i*src->texw+j],sr,sg,sb,sa);
				if(alpha == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin16[j] = bmp16[i*src->texw+j];
				}
				else{
					GET_RGBA_565(cpbegin16[j],dr,dg,db,da);
					cpbegin16[j] = MAKE_RGBA_565(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin16 += des->texw;
		}

	}
	else{
		cpbegin32 = (uint32*)des->data+y*des->texw+x;
		bmp32 = (uint32*)src->data;
		for(i =0;i<h;i++){
			for(j =0;j<w;j++){
				if((j+x>des->texw)||(i+y>des->texh))
					continue;
				GET_RGBA_8888(bmp32[i*src->texw+j],sr,sg,sb,sa);
				if(alpha == 0||sa == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin32[j] = bmp32[i*src->texw+j];
				}
				else{
					GET_RGBA_8888(cpbegin32[j],dr,dg,db,da);
					cpbegin32[j] = MAKE_RGBA_8888(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin32 += des->texw;
		}
	}

}


void image_to_image_ex(const image_p src,const image_p des,uint32 sx,uint32 sy,uint32 sw,uint32 sh,uint32 dx,uint32 dy)
{
	uint16 *cpbegin16,*bmp16;
	uint32 i,j;
	uint32 *cpbegin32,*bmp32;
	if(sw == 0 && sh == 0){
		sw = src->w;
		sh = src->h;
	}
	if(src->swizzle ==1)
		unswizzle_swap(src);
	if(des->swizzle ==1)
		unswizzle_swap(des);
	des->modified = 1;
	if(des->bpb==2){
		cpbegin16 = (uint16*)des->data+dy*des->texw+dx;
		bmp16 = (uint16*)src->data+sy*src->texw+sx;
		for(i = 0;i<sh;i++){
			for(j = 0;j<sw;j++){
				if((j+dx>des->texw)||(i+dy>des->texh))
					continue;
				cpbegin16[j] = bmp16[j];
			}
			cpbegin16 += des->texw;
			bmp16     += src->texw;
		}
	}
	else{
		cpbegin32 = (uint32*)des->data+dy*des->texw+dx;
		bmp32 = (uint32*)src->data+sy*src->texw+sx;
		for(i = 0;i<sh;i++){
			for(j = 0;j<sw;j++){
				if((j+dx>=des->texw)||(i+dy>=des->texh))
					continue;
				cpbegin32[j] = bmp32[j];
			}
			cpbegin32 += des->texw;
			bmp32     += src->texw;
		}
	}
}


void image_to_image(const image_p src,const image_p des,uint32 x,uint32 y)
{
	uint32 w = src->w;
	uint32 h = src->h;
	uint16 *cpbegin16,*bmp16;
	uint32 i,j;
	uint32 *cpbegin32,*bmp32;
	if(src->swizzle ==1)
		unswizzle_swap(src);
	if(des->swizzle ==1)
		unswizzle_swap(des);
	des->modified = 1;
	if(des->bpb==2){
		cpbegin16 = (uint16*)des->data+y*des->texw+x;
		bmp16 = (uint16*)src->data;
		for(i =0;i<h;i++){
			for(j =0;j<w;j++){
				if((j+x>des->texw)||(i+y>des->texh))
					continue;
				cpbegin16[j] = bmp16[i*src->texw+j];
			}
			cpbegin16 += des->texw;
		}

	}
	else{
		cpbegin32 = (uint32*)des->data+y*des->texw+x;
		bmp32 = (uint32*)src->data;
		for(i =0;i<h;i++){
			for(j =0;j<w;j++){
				if((j+x>=des->texw)||(i+y>=des->texh))
					continue;
				cpbegin32[j] = bmp32[i*src->texw+j];
			}
			cpbegin32 += des->texw;
		}
	}
}

void rawdata_to_image(void* data,const image_p des,uint32 x,uint32 y,uint32 w,uint32 h)
{
	uint16 *cpbegin16,*bmp16;
	uint32 *cpbegin32,*bmp32;
	uint32 i,j;
	if(des->swizzle ==1)
		unswizzle_swap(des);
	des->modified = 1;
	if(des->bpb==2){
		cpbegin16 = (uint16*)des->data+y*des->texw+x;
		bmp16 = (uint16*)data;
		for(i = 0;i<h;i++){
			for(j = 0;j<w;j++){
				if((j+x>des->texw)||(i+y>des->texh))
					continue;
				cpbegin16[j] = bmp16[i*w+j];
			}
			cpbegin16 += des->texw;
		}

	}
	else{
		cpbegin32 = (uint32*)des->data+y*des->texw+x;
		bmp32= (uint32*)data;
		for(i = 0;i < h;i++){
			for(j = 0;j < w;j++){
				if((j+x >= des->texw)||(i+y >= des->texh))
					continue;
				cpbegin32[j] = bmp32[i*w+j];
			}
			cpbegin32 += des->texw;
		}
	}
}

int image_fliph(image_p pimage)
{
	uint32 line,width,height;
	uint8 *new_bits,*bits;
	uint32 y,c;
	if (!pimage) return 0;
	if(pimage->swizzle ==1)
		unswizzle_swap(pimage);
	pimage->modified = 1;
	line   = pimage->texw*pimage->bpb;
	width = pimage->w*pimage->bpb;
	height	= pimage->texh;
	// copy between aligned memories
	new_bits = (uint8*)malloc(line * sizeof(uint8));
	if (!new_bits) return 0;
	// mirror the buffer
	for (y = 0; y < height; y++) {
		bits = pimage->data+y*line;
		memcpy(new_bits, bits, width);
		for(c = 0; c < width; c += pimage->bpb) {
					memcpy(bits + c, new_bits + width - c - pimage->bpb, pimage->bpb);
		}
	}
	free(new_bits);
	return 1;
}

int image_flipv(image_p pimage)
{
	uint8 *from, *mid;
	uint32 pitch,height,line_s,line_t;
	uint32 y;

	if (!pimage) return 0;
	if(pimage->swizzle ==1)
		unswizzle_swap(pimage);
	pimage->modified = 1;
	// swap the buffer
	pitch  = pimage->texw*pimage->bpb;
	height = pimage->h;
	// copy between aligned memories
	mid = (uint8*)malloc(pitch * sizeof(char));
	if (!mid) return 0;
	from = pimage->data;
	line_s = 0;
	line_t = (height-1) * pitch;
	for(y = 0; y < height/2; y++) {
		memcpy(mid, from + line_s, pitch);
		memcpy(from + line_s, from + line_t, pitch);
		memcpy(from + line_t, mid, pitch);
		line_s += pitch;
		line_t -= pitch;
	}
	free(mid);

	return 1;
}
