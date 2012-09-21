#include "nge_debug_log.h"
#include "nge_tga.h"
#include "nge_bmp.h"
#include "nge_png.h"
#include "nge_jpg.h"
#include "nge_rle.h"
#include "nge_io_file.h"
#include <stdlib.h>
#include <string.h>

uint32_t image_tid = 0;

#if defined(NGE_PSP)
static void swizzle_fast(uint8_t* out, const uint8_t* in, unsigned int width, unsigned int height)
{
	unsigned int blockx, blocky;
	unsigned int j;

	unsigned int width_blocks = (width / 16);
	unsigned int height_blocks = (height / 8);

	unsigned int src_pitch = (width-16)/4;
	unsigned int src_row = width * 8;
	const uint8_t *xsrc;
	const uint32_t *src;
	const uint8_t *ysrc = in;
	uint32_t *dst = (uint32_t*)out;

	for (blocky = 0; blocky < height_blocks; ++blocky)
	{
		xsrc = ysrc;
		for (blockx = 0; blockx < width_blocks; ++blockx)
		{
			src = (uint32_t*)xsrc;
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
static void unswizzle_fast(const uint8_t* out, const uint8_t* in, const int width, const int height)
{
	int blockx, blocky;
	int j;

	int width_blocks = (width / 16);
	int height_blocks = (height / 8);

	int dst_pitch = (width-16)/4;
	int dst_row = width * 8;

	uint32_t* src = (uint32_t*)in;
	uint8_t* ydst = (uint8_t*)out;
	sceKernelDcacheWritebackAll();
	for (blocky = 0; blocky < height_blocks; ++blocky)
	{
		uint8_t* xdst = ydst;
		for (blockx = 0; blockx < width_blocks; ++blockx)
		{
			uint32_t* dst;
			if ((uint32_t)out <= 0x04200000)
				dst = (uint32_t*)((uint32_t)xdst | 0x40000000);
			else
				dst = (uint32_t*)xdst;
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
	uint8_t* buffer = (uint8_t*)malloc(bsize);
	memset(buffer,0,bsize);
	swizzle_fast(buffer,pimage->data,pimage->texw*pimage->bpb,pimage->texh);
	SAFE_FREE(pimage->data);
	pimage->data = buffer;
	pimage->swizzle = 1;
}

void unswizzle_swap(image_p pimage)
{
	int bsize = pimage->texw*pimage->texh*pimage->bpb;
	uint8_t* buffer = (uint8_t*)malloc(bsize);
	memset(buffer,0,bsize);
	unswizzle_fast(buffer,pimage->data,pimage->texw*pimage->bpb,pimage->texh);
	SAFE_FREE(pimage->data);
	pimage->data = buffer;
	pimage->swizzle = 0;
}
#endif

int CreateColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a,int dtype)
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
int image_save(image_p pimage,const char* filename,uint8_t alpha,uint8_t rle)
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
	io_fread(flags,12,1,fd);
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
	else if(flags[6]=='E'||flags[6]=='J'&&flags[7]=='F'&&flags[8]=='I'){
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
	else if(mbuf[6]== 'E'||(mbuf[6]=='J'&&mbuf[7]=='F'&&mbuf[8]=='I')){
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
	io_fread(mbuf,fsize,1,handle);
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
	io_fread(flags,12,1,fd);
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
	else if(flags[6]== 'E'||(flags[6]=='J'&&flags[7]=='F'&&flags[8]=='I')){
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
			nge_print("bmp file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(mbuf[6]== 'E'||(mbuf[6]=='J'&&mbuf[7]=='F'&&mbuf[8]=='I')){
		//printf("jpg\n");
		pimage =  image_load_jpg_colorkey_buf(mbuf,bsize,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("jpg file error!\n");
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
			nge_print("tga file error!\n");
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
	io_fread(mbuf,fsize,1,handle);
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
	pimage->data = (uint8_t*)malloc(size);
	memset(pimage->data,0,size);
	return pimage;
}

image_p image_create_ex(int w,int h,int color,int displaymode)
{
	image_p pimage = (image_p)malloc(sizeof(image_t));
	int size, block = 1, processed = 1;
	uint16_t* img16;
	uint32_t* img32;
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
	pimage->data = (uint8_t*)malloc(size);
	memset(pimage->data,0,size);
	size = pimage->texw*pimage->texh;
	if(size==0)
		return pimage;
	if(displaymode==DISPLAY_PIXEL_FORMAT_8888){
		img32 = (uint32_t*)pimage->data;
		*img32 = color;
		while(processed + block <= size)	{
			memcpy(img32 + processed, img32, block * pimage->bpb);
			processed += block;
			block <<= 1;
		}
		memcpy(img32 + processed, img32, (size - processed) * pimage->bpb);
	}
	else{
		img16 = (uint16_t*)pimage->data;
		*img16 = (uint16_t)(color&0xFFFF);
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
	clone->data = (uint8_t*)malloc(size);
	memcpy(clone->data,pimage->data,size);
	return clone;
}


/*alpha blend!
 *0 完全透明，255不透明
 *Dst=( Src0*(255-Alpha) + Src1*Alpha ) / 255
 *#define MAKEALPHA(SRC,DES,ALPHA) (( SRC*(255-ALPHA) + DES*ALPHA ) /255)*/

inline static uint16_t ALPHABLEND_565(uint16_t SRC,uint16_t DST,int ALPHA) {	
	uint8_t h1, h2, h3;
	uint32_t s, d;
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
inline static uint16_t ALPHABLEND_5551(uint16_t SRC,uint16_t DST,int ALPHA) {	
	uint32_t s, d;
	uint8_t h1, h2, h3, a;
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
inline static uint16_t ALPHABLEND_5551(uint16_t SRC,uint16_t DST,int ALPHA) {	
	uint32_t s, d;
	uint8_t h1, h2, h3, a;
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

inline static uint16_t ALPHABLEND_4444(uint16_t SRC,uint16_t DST,int ALPHA) {	
	uint32_t s, d;
	uint8_t h1, h2, h3, h4;
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

inline static uint32_t ALPHABLEND_8888(uint32_t SRC,uint32_t DST,int ALPHA) {	
	uint16_t h1, h2, h3, h4;
	uint32_t s, d;
	int AL = 255 - ALPHA;
	s = ((SRC & 0xFF00FF00) >> 8) * AL; d = ((DST & 0xFF00FF00) >> 8) * ALPHA;
	SRC = (SRC & 0x00FF00FF) * AL; DST = (DST & 0x00FF00FF) * ALPHA;
	h1 = ((s >> 16) + (d >> 16)) >> 8;
	h2 = ((SRC >> 16) + (DST >> 16)) >> 8;
	h3 = ((s & 0x0000FFFF) + (d & 0x0000FFFF)) >> 8;
	h4 = ((SRC & 0x0000FFFF) + (DST & 0x0000FFFF)) >> 8;
	return (h1 << 24) | (h2 << 16) | (h3 << 8) | (h4);
}

void image_to_image_alpha_ex(const image_p src,const image_p des,int32_t sx,int32_t sy,int32_t sw,int32_t sh,int32_t dx,int32_t dy,int alpha,int flag)
{
	int32_t i,j;
	int32_t d1, d2;
	uint16_t *cpbegin16,*bmp16;
	uint32_t *cpbegin32,*bmp32;
	if(alpha == 0)
		return;
	if(sw <= 0 || sh <= 0)
		return;
	CHECK_AND_UNSWIZZLE_ALL(src, des);
	des->modified = 1;
	if(dx < 0) {
		sw += dx;
		sx = (flag & IMAGE_FLIP_H)?sx:(sx - dx);
		dx = 0;
	}
	if(dy < 0) {
		sh += dy;
		sy = (flag & IMAGE_FLIP_V)?sy:(sy - dy);
		dy = 0;
	}
	if(sx < 0) {
		sw += sx;
		dx = (flag & IMAGE_FLIP_H)?dx:(dx - sx);
		sx = 0;
	}
	if(sy < 0) {
		sh += sy;
		dy = (flag & IMAGE_FLIP_V)?dy:(dy - sy);
		sy = 0;
	}
	if(sw + sx > (int32_t)src->w) {
		dx = (flag & IMAGE_FLIP_H)?(dx + sw + sx - src->w):dx;
		sw = src->w - sx;
	}
	if(sh + sy > (int32_t)src->h) {
		dy = (flag & IMAGE_FLIP_V)?(dy + sh + sy - src->h):dy;
		sh = src->h - sy;
	}
	if(sw + dx > (int32_t)des->w) {
		sx = (flag & IMAGE_FLIP_H)?(sx + sw + dx - src->w):sx;
		sw = des->w - dx;
	}
	if(sh + dy > (int32_t)des->h) {
		sy = (flag & IMAGE_FLIP_V)?(sy + sh + dy - src->h):sy;
		sh = des->h - dy;
	}
	if(sw <= 0 || sh <= 0)
		return;
	if(des->dtype==DISPLAY_PIXEL_FORMAT_4444){
		if(flag & IMAGE_FLIP_V) {
			cpbegin16 = (uint16_t*)des->data + (dy + sh - 1) * des->texw;
			d1 = -(int32_t)des->texw;
		}
		else {
			cpbegin16 = (uint16_t*)des->data + dy * des->texw;
			d1 = des->texw;
		}
		if(flag & IMAGE_FLIP_H) {
			cpbegin16 += dx + sw - 1;
			d2 = -1;
			d1 += sw;
		}
		else {
			cpbegin16 += dx;
			d2 = 1;
			d1 -= sw;
		}
		bmp16 = (uint16_t*)src->data + sy * src->texw + sx;
		for(i = 0; i < sh; i++){
			for(j = 0; j < sw; j++, cpbegin16+=d2, bmp16++){
				#ifdef NGE_PSP
				if((*bmp16) & 0xF000)
				#else
				if((*bmp16) & 0x000F)
				#endif
					*cpbegin16 = ALPHABLEND_4444(*cpbegin16, *bmp16, alpha);
			}
			cpbegin16 += d1;
			bmp16     += src->texw - sw;
		}

	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_5551){
		if(flag & IMAGE_FLIP_V) {
			cpbegin16 = (uint16_t*)des->data + (dy + sh - 1) * des->texw;
			d1 = -(int32_t)des->texw;
		}
		else {
			cpbegin16 = (uint16_t*)des->data + dy * des->texw;
			d1 = des->texw;
		}
		if(flag & IMAGE_FLIP_H) {
			cpbegin16 += dx + sw - 1;
			d2 = -1;
			d1 += sw;
		}
		else {
			cpbegin16 += dx;
			d2 = 1;
			d1 -= sw;
		}
		bmp16 = (uint16_t*)src->data + sy * src->texw + sx;
		for(i = 0; i < sh; i++){
			for(j = 0;j < sw; j++, cpbegin16+=d2, bmp16++){
				#ifdef NGE_PSP
				if((*bmp16) >> 15)
				#else
				if((*bmp16) & 0x1)
				#endif
					*cpbegin16 = ALPHABLEND_5551(*cpbegin16, *bmp16, alpha);
			}
			cpbegin16 += d1;
			bmp16     += src->texw - sw;
		}

	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_565){
		if(flag & IMAGE_FLIP_V) {
			cpbegin16 = (uint16_t*)des->data + (dy + sh - 1) * des->texw;
			d1 = -(int32_t)des->texw;
		}
		else {
			cpbegin16 = (uint16_t*)des->data + dy * des->texw;
			d1 = des->texw;
		}
		if(flag & IMAGE_FLIP_H) {
			cpbegin16 += dx + sw - 1;
			d2 = -1;
			d1 += sw;
		}
		else {
			cpbegin16 += dx;
			d2 = 1;
			d1 -= sw;
		}
		bmp16 = (uint16_t*)src->data + sy * src->texw + sx;
		for(i = 0; i < sh; i++){
			for(j = 0; j < sw; j++, cpbegin16+=d2, bmp16++)
				*cpbegin16 = ALPHABLEND_565(*cpbegin16, *bmp16, alpha);
			cpbegin16 += d1;
			bmp16     += src->texw - sw;
		}
	}
	else{
		if(flag & IMAGE_FLIP_V) {
			cpbegin32 = (uint32_t*)des->data + (dy + sh - 1) * des->texw;
			d1 = -(int32_t)des->texw;
		}
		else {
			cpbegin32 = (uint32_t*)des->data + dy * des->texw;
			d1 = des->texw;
		}
		if(flag & IMAGE_FLIP_H) {
			cpbegin32 += dx + sw - 1;
			d2 = -1;
			d1 += sw;
		}
		else {
			cpbegin32 += dx;
			d2 = 1;
			d1 -= sw;
		}
		bmp32 = (uint32_t*)src->data+sy*src->texw+sx;
		for(i = 0;i < sh; i++){
			for(j = 0;j < sw; j++,cpbegin32+=d2, bmp32++){
				if((*bmp32) & 0xFF000000)
					*cpbegin32 = ALPHABLEND_8888(*cpbegin32, *bmp32, alpha);
			}
			cpbegin32 += d1;
			bmp32     += src->texw - sw;
		}
	}

}

void image_to_image_alpha(const image_p src,const image_p des,int32_t x,int32_t y,int alpha)
{
	int32_t i,j;
	uint16_t *cpbegin16;
	uint16_t *bmp16;
	int32_t w = src->w;
	int32_t h = src->h;
	int32_t sx = 0, sy = 0;
	uint32_t *cpbegin32,*bmp32;
	if(alpha == 0)
		return;
	CHECK_AND_UNSWIZZLE_ALL(src, des);
	des->modified = 1;
	if(x < 0) {
		w += x;
		sx = -x;
		x = 0;
	}
	if(y < 0) {
		h += y;
		sy = -y;
		y = 0;
	}
	if(w + x > (int32_t)des->w)
		w = des->w - x;
	if(h + y > (int32_t)des->h)
		h = des->h - y;
	if(w <= 0 || h <= 0)
		return;
	if(des->dtype==DISPLAY_PIXEL_FORMAT_4444){
		cpbegin16 = (uint16_t*)des->data + y * des->texw + x;
		bmp16 = (uint16_t*)src->data + sy * src->texw + sx;
		for(i =0; i < h; i++){
			for(j =0; j < w; j++, bmp16++, cpbegin16++){
				#ifdef NGE_PSP
				if((*bmp16) & 0xF000)
				#else
				if((*bmp16) & 0x000F)
				#endif
					*cpbegin16 = ALPHABLEND_4444(*cpbegin16, *bmp16, alpha);
			}
			bmp16 += src->texw - w;
			cpbegin16 += des->texw - w;
		}

	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_5551){
		cpbegin16 = (uint16_t*)des->data + y * des->texw + x;
		bmp16 = (uint16_t*)src->data + sy * src->texw + sx;
		for(i = 0;i < h; i++){
			for(j = 0;j < w; j++, cpbegin16++, bmp16++){
				#ifdef NGE_PSP
				if((*bmp16) >> 15)
				#else
				if((*bmp16) & 0x1)
				#endif
					*cpbegin16 = ALPHABLEND_5551(*cpbegin16, *bmp16, alpha);
			}
			cpbegin16 += des->texw - w;
			bmp16 += src->texw - w;
		}

	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_565){
		cpbegin16 = (uint16_t*)des->data + y * des->texw + x;
		bmp16 = (uint16_t*)src->data + sy * src->texw + sx;
		for(i = 0; i < h; i++){
			for(j = 0;j < w; j++, cpbegin16++, bmp16++)
				*cpbegin16 = ALPHABLEND_565(*cpbegin16, *bmp16, alpha);
			cpbegin16 += des->texw - w;
			bmp16 += src->texw - w;
		}

	}
	else{
		cpbegin32 = (uint32_t*)des->data+y*des->texw+x;
		bmp32 = (uint32_t*)src->data + sy * src->texw + sx;
		for(i = 0; i < h; i++){
			for(j = 0; j < w; j++, bmp32++, cpbegin32++){
				if((*bmp32) & 0xFF000000)
					*cpbegin32 = ALPHABLEND_8888(*cpbegin32, *bmp32, alpha);
			}
			cpbegin32 += des->texw - w;
			bmp32 += src->texw - w;
		}
	}

}


void image_to_image_ex(const image_p src,const image_p des,int32_t sx,int32_t sy,int32_t sw,int32_t sh,int32_t dx,int32_t dy, int flag)
{
	int32_t i,j;
	int32_t delta;
	uint16_t *cpbegin16,*bmp16;
	uint32_t *cpbegin32,*bmp32;
	uint32_t size;
	CHECK_AND_UNSWIZZLE_ALL(src, des);
	des->modified = 1;
	if(sw <= 0 || sh <= 0)
		return;
	if(dx < 0) {
		sw += dx;
		sx = (flag & IMAGE_FLIP_H)?sx:(sx - dx);
		dx = 0;
	}
	if(dy < 0) {
		sh += dy;
		sy = (flag & IMAGE_FLIP_V)?sy:(sy - dy);
		dy = 0;
	}
	if(sx < 0) {
		sw += sx;
		dx = (flag & IMAGE_FLIP_H)?dx:(dx - sx);
		sx = 0;
	}
	if(sy < 0) {
		sh += sy;
		dy = (flag & IMAGE_FLIP_V)?dy:(dy - sy);
		sy = 0;
	}
	if(sw + sx > (int32_t)src->w) {
		dx = (flag & IMAGE_FLIP_H)?(dx + sw + sx - src->w):dx;
		sw = src->w - sx;
	}
	if(sh + sy > (int32_t)src->h) {
		dy = (flag & IMAGE_FLIP_V)?(dy + sh + sy - src->h):dy;
		sh = src->h - sy;
	}
	if(sw + dx > (int32_t)des->w) {
		sx = (flag & IMAGE_FLIP_H)?(sx + sw + dx - src->w):sx;
		sw = des->w - dx;
	}
	if(sh + dy > (int32_t)des->h) {
		sy = (flag & IMAGE_FLIP_V)?(sy + sh + dy - src->h):sy;
		sh = des->h - dy;
	}
	if(sw <= 0 || sh <= 0)
		return;
	if(des->bpb==2){
		if(flag & IMAGE_FLIP_V) {
			cpbegin16 = (uint16_t*)des->data + (dy + sh - 1) * des->texw + dx;
			delta = -(int32_t)des->texw;
		}
		else {
			cpbegin16 = (uint16_t*)des->data + dy * des->texw + dx;
			delta = des->texw;
		}
		bmp16 = (uint16_t*)src->data + sy * src->texw + sx;
		size = sw * sizeof(uint16_t);
		if(flag & IMAGE_FLIP_H) {
			for(i = 0; i < sh; i++, cpbegin16 += delta, bmp16 += src->texw)
				for(j = 0; j < sw; j++)
					cpbegin16[j] = bmp16[sw - j - 1];
		}
		else {
			for(i = 0; i < sh; i++, cpbegin16 += delta, bmp16 += src->texw)
				memcpy(cpbegin16, bmp16, size);
		}
	}
	else{
		if(flag & IMAGE_FLIP_V) {
			cpbegin32 = (uint32_t*)des->data + (dy + sh - 1) * des->texw + dx;
			delta = -(int32_t)des->texw;
		}
		else {
			cpbegin32 = (uint32_t*)des->data + dy * des->texw + dx;
			delta = des->texw;
		}
		bmp32 = (uint32_t*)src->data+sy*src->texw+sx;
		size = sw * sizeof(uint32_t);
		if(flag & IMAGE_FLIP_H) {
			for(i = 0; i < sh; i++, cpbegin32 += delta, bmp32 += src->texw)
				for(j = 0; j < sw; j++)
					cpbegin32[j] = bmp32[sw - j - 1];
		}
		else {
			for(i = 0; i < sh; i++, cpbegin32 += delta, bmp32 += src->texw)
				memcpy(cpbegin32, bmp32, size);
		}
	}
}


void image_to_image(const image_p src,const image_p des,int32_t x,int32_t y)
{
	int32_t w = src->w;
	int32_t h = src->h;
	int32_t sx = 0, sy = 0;
	uint16_t *cpbegin16,*bmp16;
	int i, size;
	uint32_t *cpbegin32,*bmp32;
	CHECK_AND_UNSWIZZLE_ALL(src, des);
	des->modified = 1;
	if(x < 0) {
		w += x;
		sx = -x;
		x = 0;
	}
	if(y < 0) {
		h += y;
		sy = -y;
		y = 0;
	}
	if(w + x > (int32_t)des->w)
		w = des->w - x;
	if(h + y > (int32_t)des->h)
		h = des->h - y;
	if(w <= 0 || h <= 0)
		return;
	if(des->bpb==2){
		cpbegin16 = (uint16_t*)des->data + y * des->texw + x;
		bmp16 = (uint16_t*)src->data + sy * src->texw + sx;
		size = w * sizeof(uint16_t);
		for(i = 0; i < h; i++, cpbegin16 += des->texw, bmp16 += src->texw)
			memcpy(cpbegin16, bmp16, size);
	}
	else{
		cpbegin32 = (uint32_t*)des->data + y * des->texw + x;
		bmp32= (uint32_t*)src->data + sy * src->texw + sx;
		size = w * sizeof(uint32_t);
		for(i = 0; i < h; i++, cpbegin32 += des->texw, bmp32 += src->texw)
			memcpy(cpbegin32, bmp32, size);
	}
}

void rawdata_to_image(void* data,const image_p des,int32_t x,int32_t y,int32_t w,int32_t h)
{
	uint16_t *cpbegin16,*bmp16;
	uint32_t *cpbegin32,*bmp32;
	int32_t i;
	int32_t sx = 0, sy = 0;
	int32_t bw = w, bh = h;
	int32_t size;
	CHECK_AND_UNSWIZZLE(des);
	des->modified = 1;
	if(x < 0) {
		w += x;
		sx = -x;
		x = 0;
	}
	if(y < 0) {
		h += y;
		sy = -y;
		y = 0;
	}
	if(w + x > (int32_t)des->w)
		w = des->w - x;
	if(h + y > (int32_t)des->h)
		h = des->h - y;
	if(w <= 0 || h <= 0)
		return;
	if(des->bpb==2){
		cpbegin16 = (uint16_t*)des->data + y * des->texw + x;
		bmp16 = (uint16_t*)data + sy * bw + sx;
		size = w * sizeof(uint16_t);
		for(i = 0; i < h; i++, cpbegin16 += des->texw, bmp16 += bw)
			memcpy(cpbegin16, bmp16, size);
	}
	else{
		cpbegin32 = (uint32_t*)des->data + y * des->texw + x;
		bmp32= (uint32_t*)data + sy * bw + sx;
		size = w * sizeof(uint32_t);
		for(i = 0; i < h; i++, cpbegin32 += des->texw, bmp32 += bw)
			memcpy(cpbegin32, bmp32, size);
	}
}

int image_fliph(image_p pimage)
{
	uint32_t line,width,height;
	uint8_t *new_bits,*bits;
	uint32_t y,c;
	if (!pimage) return 0;
	CHECK_AND_UNSWIZZLE(pimage);
	pimage->modified = 1;
	line   = pimage->texw*pimage->bpb;
	width = pimage->w*pimage->bpb;
	height	= pimage->texh;
	// copy between aligned memories
	new_bits = (uint8_t*)malloc(line * sizeof(uint8_t));
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
	uint8_t *from, *mid;
	uint32_t pitch,height,line_s,line_t;
	uint32_t y;

	if (!pimage) return 0;
	CHECK_AND_UNSWIZZLE(pimage);
	pimage->modified = 1;
	// swap the buffer
	pitch  = pimage->texw*pimage->bpb;
	height = pimage->h;
	// copy between aligned memories
	mid = (uint8_t*)malloc(pitch * sizeof(char));
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

void image_fillrect(image_p pimage, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
	int i, j;
	uint16_t *img16;
	uint32_t *img32;
	if(x < 0) {
		w += x;
		x = 0;
	}
	if(y < 0) {
		h += y;
		y = 0;
	}
	if(w + x > (int32_t)pimage->w)
		w = pimage->w - x;
	if(h + y > (int32_t)pimage->h)
		h = pimage->h - y;
	if(w <= 0 || h <= 0)
		return;
	pimage->modified = 1;
	CHECK_AND_UNSWIZZLE(pimage)
	if(pimage->bpb == 2) {
		img16 = ((uint16_t*)pimage->data) + y * pimage->texw + x;
		for(i = 0; i < h && i < 1; i++)
			for(j = 0; j < w; j++)
				*(img16 + j) = (uint16_t)(color & 0xffff);
		for(; i < h; i++)
			memcpy(img16 + i * pimage->texw, img16, w * pimage->bpb);
	}
	else {
		img32 = ((uint32_t*)pimage->data) + y * pimage->texw + x;
		for(i = 0; i < h && i < 1; i++)
			for(j = 0; j < w; j++)
				*(img32 + j) = color;
		for(; i < h; i++)
			memcpy(img32 + i * pimage->texw, img32, w * pimage->bpb);
	}
}
