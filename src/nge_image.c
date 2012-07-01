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
	int size, block = 1, processed = 1;
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
	size = pimage->texw*pimage->texh;
	if(size==0)
		return pimage;
	if(displaymode==DISPLAY_PIXEL_FORMAT_8888){
		img32 = (uint32*)pimage->data;
		*img32 = color;
		while(processed + block <= size)	{
			memcpy(img32 + processed, img32, block * pimage->bpb);
			processed += block;
			block <<= 1;
		}
		memcpy(img32 + processed, img32, (size - processed) * pimage->bpb);
	}
	else{
		img16 = (uint16*)pimage->data;
		*img16 = (uint16)(color&0xFFFF);
		while(processed + block <= size)	{
			memcpy(img16 + processed, img16, block * pimage->bpb);
			processed += block;
			block <<= 1;
		}
		memcpy(img16 + processed, img16, (size - processed) * pimage->bpb);
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

inline uint16 ALPHABLEND_565(uint16 SRC,uint16 DST,int ALPHA) {	
	uint8 h1, h2, h3;
	uint32 s, d;
	int AL;
	ALPHA >>= 2; AL = 64 - ALPHA;
	h2 = ( ((SRC & 0x07E0) >> 5) * AL + ((DST & 0x07E0) >> 5) * ALPHA ) >> 6;
	s = SRC & 0xF81F; d = DST & 0xF81F;
	ALPHA >>= 1; AL >>= 1;
	s *= AL; d *= ALPHA;
	s += d;
	h1 = (s & 0x3FF800) >> 16;
	h3 = (s & 0x7FF) >> 5;
	return (h1 << 11) | (h2 << 5) | (h3);
}

#ifdef NGE_PSP
inline uint16 ALPHABLEND_5551(uint16 SRC,uint16 DST,int ALPHA) {	
	uint32 s, d;
	uint8 h1, h2, h3, a;
	int AL;
	ALPHA >>= 3; AL = 32 - ALPHA;
	s = ((SRC & 0x7C00) << 10) | ((SRC & 0x3E0) << 5) | (SRC & 0x1F);
	d = ((DST & 0x7C00) << 10) | ((DST & 0x3E0) << 5) | (DST & 0x1F);
	s *= AL; d *= ALPHA;
	a = (ALPHA > 15)?(DST >> 15):(SRC >> 15);
	h1 = ((s >> 20) + (d >> 20)) >> 5;
	h2 = (((s & 0xFFC00) >> 10) + ((d & 0xFFC00) >> 10)) >> 5;
	h3 = ((s & 0x3FF) + (d & 0x3FF)) >> 5;
	return (a << 15) | (h1 << 10) | (h2 << 5) | (h3);
}
#else
inline uint16 ALPHABLEND_5551(uint16 SRC,uint16 DST,int ALPHA) {	
	uint32 s, d;
	uint8 h1, h2, h3, a;
	int AL;
	ALPHA >>= 3; AL = 32 - ALPHA;
	s = ((SRC & 0xF800) << 10) | ((SRC & 0x7C0) << 5) | (SRC & 0x3E);
	d = ((DST & 0xF800) << 10) | ((DST & 0x7C0) << 5) | (DST & 0x3E);
	s *= AL; d *= ALPHA;
	a = (ALPHA > 15)?(DST & 0x1):(SRC & 0x1);
	h1 = ((s >> 21) + (d >> 21)) >> 5;
	h2 = (((s & 0x1FF800) >> 11) + ((d & 0x1FF800) >> 11)) >> 5;
	h3 = (((s & 0x7FE) >> 1) + ((d & 0x7FE) >> 1)) >> 5;
	return (h1 << 11) | (h2 << 6) | (h3 << 1) | (a);
}
#endif

inline uint16 ALPHABLEND_4444(uint16 SRC,uint16 DST,int ALPHA) {	
	uint32 s, d;
	uint8 h1, h2, h3, h4;
	int AL;
	ALPHA >>= 4;
	AL = 15 - ALPHA;
	s = ((SRC & 0xF000) << 12) | ((SRC & 0x0F00) << 8) | ((SRC & 0x00F0) << 4) | (SRC & 0x000F);
	d = ((DST & 0xF000) << 12) | ((DST & 0x0F00) << 8) | ((DST & 0x00F0) << 4) | (DST & 0x000F);
	s *= AL; d *= ALPHA;
	h1 = ((s >> 24) + (d >> 24)) /15;
	h2 = (((s >> 16) & 0xFF) + ((d >> 16) & 0xFF)) /15;
	h3 = (((s >> 8) & 0xFF) + ((d >> 8) & 0xFF)) /15;
	h4 = ((s & 0xFF) + (d & 0xFF)) /15;
	return (h1 << 12) | (h2 << 8) | (h3 << 4) | (h4);
}

inline uint32 ALPHABLEND_8888(uint32 SRC,uint32 DST,int ALPHA) {	
	uint16 h1, h2, h3, h4;
	uint32 s, d;
	int AL = 255 - ALPHA;
	s = ((SRC & 0xFF00FF00) >> 8) * AL; d = ((DST & 0xFF00FF00) >> 8) * ALPHA;
	SRC = (SRC & 0x00FF00FF) * AL; DST = (DST & 0x00FF00FF) * ALPHA;
	h1 = ((s >> 16) + (d >> 16)) >> 8;
	h2 = ((SRC >> 16) + (DST >> 16)) >> 8;
	h3 = ((s & 0x0000FFFF) + (d & 0x0000FFFF)) >> 8;
	h4 = ((SRC & 0x0000FFFF) + (DST & 0x0000FFFF)) >> 8;
	return (h1 << 24) | (h2 << 16) | (h3 << 8) | (h4);
}

void image_to_image_alpha_ex(const image_p src,const image_p des,uint32 sx,uint32 sy,uint32 sw,uint32 sh,uint32 dx,uint32 dy,int alpha)
{
	uint32 i,j;
	uint16 *cpbegin16;
	uint16 *bmp16;
	uint32 *cpbegin32,*bmp32;
	if(alpha == 255) {
		image_to_image_ex(src, des, sx, sy, sw, sh, dx, dy);
		return;
	}
	if(alpha == 0)
		return;
	CHECK_AND_UNSWIZZLE_ALL(src, des);
	des->modified = 1;
	if(sw == 0 && sh == 0){
		sw = src->w;
		sh = src->h;
	}
	if(sw + sx > src->texw)
		sw = src->texw - sx;
	if(sh + sy > src->texh)
		sh = src->texh - sy;
	if(sw + dx > des->texw)
		sw = des->texw - dx;
	if(sh + dy > des->texh)
		sh = des->texh - dy;
	if(des->dtype==DISPLAY_PIXEL_FORMAT_4444){
		cpbegin16 = (uint16*)des->data + dy * des->texw + dx;
		bmp16 = (uint16*)src->data + sy * src->texw + sx;
		for(i = 0; i < sh; i++){
			for(j = 0; j < sw; j++, cpbegin16++, bmp16++){
				#ifdef NGE_PSP
				if((*bmp16) & 0xF000)
				#else
				if((*bmp16) & 0x000F)
				#endif
					*cpbegin16 = ALPHABLEND_4444(*bmp16, *cpbegin16, alpha);
			}
			cpbegin16 += des->texw - sw;
			bmp16     += src->texw - sw;
		}

	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_5551){
		cpbegin16 = (uint16*)des->data + dy * des->texw + dx;
		bmp16 = (uint16*)src->data + sy * src->texw + sx;
		for(i = 0; i < sh; i++){
			for(j = 0;j < sw; j++, cpbegin16++, bmp16++){
				#ifdef NGE_PSP
				if((*bmp16) >> 15)
				#else
				if((*bmp16) & 0x1)
				#endif
					*cpbegin16 = ALPHABLEND_5551(*bmp16, *cpbegin16, alpha);
			}
			cpbegin16 += des->texw - sw;
			bmp16     += src->texw - sw;
		}

	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_565){
		cpbegin16 = (uint16*)des->data + dy * des->texw + dx;
		bmp16 = (uint16*)src->data + sy * src->texw + sx;
		for(i = 0; i < sh; i++){
			for(j = 0; j < sw; j++, cpbegin16++, bmp16++)
				*cpbegin16 = ALPHABLEND_565(*bmp16, *cpbegin16, alpha);
			cpbegin16 += des->texw - sw;
			bmp16     += src->texw - sw;
		}
	}
	else{
		cpbegin32 = (uint32*)des->data+dy*des->texw+dx;
		bmp32 = (uint32*)src->data+sy*src->texw+sx;
		for(i = 0;i < sh; i++){
			for(j = 0;j < sw; j++, bmp32++, cpbegin32++){
				if((*bmp32) & 0xFF000000)
					*cpbegin32 = ALPHABLEND_8888(*bmp32, *cpbegin32, alpha);
			}
			cpbegin32 += des->texw - sw;
			bmp32     += src->texw - sw;
		}
	}

}

void image_to_image_alpha(const image_p src,const image_p des,uint32 x,uint32 y,int alpha)
{
	uint32 i,j;
	uint16 *cpbegin16;
	uint16 *bmp16;
	uint32 w = src->w;
	uint32 h = src->h;
	uint32 *cpbegin32,*bmp32;
	if(alpha == 255) {
		image_to_image(src, des, x, y);
		return;
	}
	if(alpha == 0)
		return;
	CHECK_AND_UNSWIZZLE_ALL(src, des);
	des->modified = 1;
	if(w + x > des->texw)
		w = des->texw - x;
	if(h + y > des->texh)
		h = des->texh - y;
	if(des->dtype==DISPLAY_PIXEL_FORMAT_4444){
		cpbegin16 = (uint16*)des->data + y * des->texw + x;
		bmp16 = (uint16*)src->data;
		for(i =0; i < h; i++){
			for(j =0; j < w; j++, bmp16++, cpbegin16++){
				#ifdef NGE_PSP
				if((*bmp16) & 0xF000)
				#else
				if((*bmp16) & 0x000F)
				#endif
					*cpbegin16 = ALPHABLEND_4444(*bmp16, *cpbegin16, alpha);
			}
			bmp16 += src->texw - w;
			cpbegin16 += des->texw - w;
		}

	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_5551){
		cpbegin16 = (uint16*)des->data + y * des->texw + x;
		bmp16 = (uint16*)src->data;
		for(i = 0;i < h; i++){
			for(j = 0;j < w; j++, cpbegin16++, bmp16++){
				#ifdef NGE_PSP
				if((*bmp16) >> 15)
				#else
				if((*bmp16) & 0x1)
				#endif
					*cpbegin16 = ALPHABLEND_5551(*bmp16, *cpbegin16, alpha);
			}
			cpbegin16 += des->texw - w;
			bmp16 += src->texw - w;
		}

	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_565){
		cpbegin16 = (uint16*)des->data + y * des->texw + x;
		bmp16 = (uint16*)src->data;
		for(i = 0; i < h; i++){
			for(j = 0;j < w; j++, cpbegin16++, bmp16++)
				*cpbegin16 = ALPHABLEND_565(*bmp16, *cpbegin16, alpha);
			cpbegin16 += des->texw - w;
			bmp16 += src->texw - w;
		}

	}
	else{
		cpbegin32 = (uint32*)des->data+y*des->texw+x;
		bmp32 = (uint32*)src->data;
		for(i = 0; i < h; i++){
			for(j = 0; j < w; j++, bmp32++, cpbegin32++){
				if((*bmp32) & 0xFF000000)
					*cpbegin32 = ALPHABLEND_8888(*bmp32, *cpbegin32, alpha);
			}
			cpbegin32 += des->texw - w;
			bmp32 += src->texw - w;
		}
	}

}


void image_to_image_ex(const image_p src,const image_p des,uint32 sx,uint32 sy,uint32 sw,uint32 sh,uint32 dx,uint32 dy)
{
	uint16 *cpbegin16,*bmp16;
	uint32 i;
	uint32 *cpbegin32,*bmp32;
	uint32 size;
	if(sw == 0 && sh == 0){
		sw = src->w;
		sh = src->h;
	}
	CHECK_AND_UNSWIZZLE_ALL(src, des);
	des->modified = 1;
	if(sw + sx > src->texw)
		sw = src->texw - sx;
	if(sh + sy > src->texh)
		sh = src->texh - sy;
	if(sw + dx > des->texw)
		sw = des->texw - dx;
	if(sh + dy > des->texh)
		sh = des->texh - dy;
	if(des->bpb==2){
		cpbegin16 = (uint16*)des->data + dy * des->texw + dx;
		bmp16 = (uint16*)src->data + sy * src->texw + sx;
		size = sw * sizeof(uint16);
		for(i = 0; i < sh; i++, cpbegin16 += des->texw, bmp16 += src->texw)
			memcpy(cpbegin16, bmp16, size);
	}
	else{
		cpbegin32 = (uint32*)des->data+dy*des->texw+dx;
		bmp32 = (uint32*)src->data+sy*src->texw+sx;
		size = sw * sizeof(uint32);
		for(i = 0; i < sh; i++, cpbegin32 += des->texw, bmp32 += src->texw)
			memcpy(cpbegin32, bmp32, size);
	}
}


void image_to_image(const image_p src,const image_p des,uint32 x,uint32 y)
{
	uint32 w = src->w;
	uint32 h = src->h;
	uint16 *cpbegin16,*bmp16;
	uint32 i;
	uint32 size;
	uint32 *cpbegin32,*bmp32;
	CHECK_AND_UNSWIZZLE_ALL(src, des);
	des->modified = 1;
	if(w + x > des->texw)
		w = des->texw - x;
	if(h + y > des->texh)
		h = des->texh - y;
	if(des->bpb==2){
		cpbegin16 = (uint16*)des->data + y * des->texw + x;
		bmp16 = (uint16*)src->data;
		size = w * sizeof(uint16);
		for(i = 0; i < h; i++, cpbegin16 += des->texw, bmp16 += src->texw)
			memcpy(cpbegin16, bmp16, size);
	}
	else{
		cpbegin32 = (uint32*)des->data + y * des->texw + x;
		bmp32= (uint32*)src->data;
		size = w * sizeof(uint32);
		for(i = 0; i < h; i++, cpbegin32 += des->texw, bmp32 += src->texw)
			memcpy(cpbegin32, bmp32, size);
	}
}

void rawdata_to_image(void* data,const image_p des,uint32 x,uint32 y,uint32 w,uint32 h)
{
	uint16 *cpbegin16,*bmp16;
	uint32 *cpbegin32,*bmp32;
	uint32 i;
	uint32 size;
	CHECK_AND_UNSWIZZLE(des);
	des->modified = 1;
	if(w + x > des->texw)
		w = des->texw - x;
	if(h + y > des->texh)
		h = des->texh - y;
	if(des->bpb==2){
		cpbegin16 = (uint16*)des->data + y * des->texw + x;
		bmp16 = (uint16*)data;
		size = w * sizeof(uint16);
		for(i = 0; i < h; i++, cpbegin16 += des->texw, bmp16++)
			memcpy(cpbegin16, bmp16, size);
	}
	else{
		cpbegin32 = (uint32*)des->data + y * des->texw + x;
		bmp32= (uint32*)data;
		size = w * sizeof(uint32);
		for(i = 0; i < h; i++, cpbegin32 += des->texw, bmp32++)
			memcpy(cpbegin32, bmp32, size);
	}
}

int image_fliph(image_p pimage)
{
	uint32 line,width,height;
	uint8 *new_bits,*bits;
	uint32 y,c;
	if (!pimage) return 0;
	CHECK_AND_UNSWIZZLE(pimage);
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
	CHECK_AND_UNSWIZZLE(pimage);
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
