#include "nge_debug_log.h"
#include "nge_image_ext.h"
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define _min(a,b) ((a)<(b)?(a):(b))
#define _max(a,b) ((a)>(b)?(a):(b))

int get_gray_color(int dtype, int scol, int gray)
{
	int gcol = 0, r = 0, g = 0, b = 0, a = 0;
	if(gray==0)
		return scol;
	switch(dtype)
	{
	case DISPLAY_PIXEL_FORMAT_8888:
		GET_RGBA_8888(scol, r, g, b, a);
		break;
	case DISPLAY_PIXEL_FORMAT_4444:
		GET_RGBA_4444(scol, r, g, b, a);
		break;
	case DISPLAY_PIXEL_FORMAT_5551:
		GET_RGBA_5551(scol, r, g, b, a);
		break;
	case DISPLAY_PIXEL_FORMAT_565:
		GET_RGBA_565(scol, r, g, b, a);
		break;
	}

	gcol = ((r + g + b) / 3) & 0xff;
	if(gray==100)
	{
		switch(dtype)
		{
			case DISPLAY_PIXEL_FORMAT_8888:
				return MAKE_RGBA_8888(gcol, gcol, gcol, a);
				break;
			case DISPLAY_PIXEL_FORMAT_4444:
				return MAKE_RGBA_4444(gcol, gcol, gcol, a);
				break;
			case DISPLAY_PIXEL_FORMAT_5551:
				return MAKE_RGBA_5551(gcol, gcol, gcol, a);
				break;
			case DISPLAY_PIXEL_FORMAT_565:
				return MAKE_RGBA_565(gcol, gcol, gcol, a);
				break;
		}
	}
	r += ((gcol - r) * gray / 100) & 0xff;
	g += ((gcol - g) * gray / 100) & 0xff;
	b += ((gcol - b) * gray / 100) & 0xff;

	switch(dtype)
	{
		case DISPLAY_PIXEL_FORMAT_8888:
			return MAKE_RGBA_8888(r, g, b, a);
			break;
		case DISPLAY_PIXEL_FORMAT_4444:
			return MAKE_RGBA_4444(r, g, b, a);
			break;
		case DISPLAY_PIXEL_FORMAT_5551:
			return MAKE_RGBA_5551(r, g, b, a);
			break;
		case DISPLAY_PIXEL_FORMAT_565:
			return MAKE_RGBA_565(r, g, b, a);
			break;
	}

	return 0;
}

/**
 * 建立 src 图像的灰度版本
 *
 * @param src 源图像
 * @param gray 灰度 0 到 100, 0 为源图像, 100 为纯灰色图像
 */
image_p create_gray_image(image_p src, int gray)
{
	uint32 y,x;
	image_p pimg;
	uint16 *p16 = NULL,*psrc16 = NULL;
	uint32 *p32 = NULL,*psrc32 = NULL;

	if(gray<0 || gray>100)
	{
		nge_print("create_gray_image arg 'gray' must between 0 to 100!");
		return NULL;
	}
	CHECK_AND_UNSWIZZLE(src);
	pimg = image_create(src->w, src->h, src->dtype);

	for(y=0;y<src->h;y++)
	{
		if(src->dtype == DISPLAY_PIXEL_FORMAT_8888)
		{
			p32 = (uint32 *)pimg->data + y*src->texw;
			psrc32 = (uint32 *)src->data + y*src->texw;
		}
		else
		{
			p16 = (uint16 *)pimg->data + y*src->texw;
			psrc16 = (uint16 *)src->data + y*src->texw;
		}

		for(x=0;x<src->w;x++)
		{
			if(src->dtype == DISPLAY_PIXEL_FORMAT_8888)
			{
				*(p32 + x) = (uint32)get_gray_color(pimg->dtype, *(psrc32 + x), gray);
			}
			else
			{
				*(p16 + x) = (uint16)get_gray_color(pimg->dtype, *(psrc16 + x), gray);
			}
		}
	}
	CHECK_AND_SWIZZLE(src);

	return pimg;
}

int get_saturation_brightness_color(int dtype, int scol,int saturation, int brightness)
{
	int r, g, b, a, mincol, maxcol;
	switch(dtype)
	{
	case DISPLAY_PIXEL_FORMAT_8888:
		GET_RGBA_8888(scol, r, g, b, a);
		break;
	case DISPLAY_PIXEL_FORMAT_4444:
		GET_RGBA_4444(scol, r, g, b, a);
		break;
	case DISPLAY_PIXEL_FORMAT_5551:
		GET_RGBA_5551(scol, r, g, b, a);
		break;
	case DISPLAY_PIXEL_FORMAT_565:
		GET_RGBA_565(scol, r, g, b, a);
		break;
	default:
		return 0;
	}
	// 处理饱和度
	if(saturation!=0)
	{
		maxcol = r>g ? (r>b?r:b):g;
		mincol = r>g ? (g>b?b:g):r;
		if(saturation>0)
		{
			if(maxcol == 0xff)
			{
				goto cont001;
			}
		}
		else
		{
			if(maxcol == 0)
			{
				goto cont001;
			}
		}
		r += mincol + (r - mincol)*(100 + saturation)/100;
		g = mincol + (g - mincol)*(100 + saturation)/100;
		b = mincol + (b - mincol)*(100 + saturation)/100;
		if(r>0xff)r=0xff;
		if(g>0xff)g=0xff;
		if(b>0xff)b=0xff;
		if(r<0)r=0;
		if(g<0)g=0;
		if(b<0)b=0;
	}
cont001:
	// 处理亮度
	if(brightness!=0)
	{
		//gcol = ((r + g + b) / 3) & 0xff;
		//maxcol = r>g ? (r>b?r:b):g;
		mincol = r>g ? (g>b?b:g):r;
		r += (0xff-mincol) * (brightness)/100;
		g += (0xff-mincol) * (brightness)/100;
		b += (0xff-mincol) * (brightness)/100;
		if(r>0xff)r=0xff;
		if(g>0xff)g=0xff;
		if(b>0xff)b=0xff;
		if(r<0)r=0;
		if(g<0)g=0;
		if(b<0)b=0;
	}

	// 输出
	switch(dtype)
	{
		case DISPLAY_PIXEL_FORMAT_8888:
			return MAKE_RGBA_8888(r, g, b, a);
			break;
		case DISPLAY_PIXEL_FORMAT_4444:
			return MAKE_RGBA_4444(r, g, b, a);
			break;
		case DISPLAY_PIXEL_FORMAT_5551:
			return MAKE_RGBA_5551(r, g, b, a);
			break;
		case DISPLAY_PIXEL_FORMAT_565:
			return MAKE_RGBA_565(r, g, b, a);
			break;
	}

	return 0;
}

/**
 * 建立 src 图像的亮度，饱和度版本
 *
 * @param src 源图像
 * @param saturation 饱合度 -100(灰度图像) 至 100(高彩色比图像)
 * @param brightness 亮度 -100(全黑) 至 100(全白)
 */
image_p create_saturation_brightness_image(image_p src, int saturation, int brightness)
{
	uint32 y,x;
	image_p pimg;
	uint16 *p16,*psrc16;
	uint32 *p32,*psrc32;

	if(saturation<-100 || saturation>100)
	{
		nge_print("create_gray_image arg 'saturation' must between -100 to 100!");
		return NULL;
	}

	if(brightness<-100 || brightness>100)
	{
		nge_print("create_gray_image arg 'brightness' must between -100 to 100!");
		return NULL;
	}
	CHECK_AND_UNSWIZZLE(src);
	pimg = image_create(src->w, src->h, src->dtype);

	if(src->dtype == DISPLAY_PIXEL_FORMAT_8888)
	{
		p32 = (uint32 *)pimg->data;
		psrc32 = (uint32 *)src->data;

		for(y=0;y<src->h;y++)
		{
			p32 += src->texw;
			psrc32 += src->texw;

			for(x=0;x<src->w;x++)
			{
				*(p32 + x) = (uint32)get_saturation_brightness_color(pimg->dtype, *(psrc32 + x), saturation, brightness);
			}
		}
	}
	else if (src->dtype == DISPLAY_PIXEL_FORMAT_4444)
	{
		p16 = (uint16 *)pimg->data;
		psrc16 = (uint16 *)src->data;

		for(y=0;y<src->h;y++)
		{
			p16 += src->texw;
			psrc16 += src->texw;

			for(x=0;x<src->w;x++)
			{
				*(p16 + x) = (uint16)get_saturation_brightness_color(pimg->dtype, *(psrc16 + x), saturation, brightness);
			}
		}
	}

	CHECK_AND_SWIZZLE(src);

	return pimg;
}

//image conv
image_p image_conv(image_p src, int dtype)
{
	image_p dst;
	uint32 i,j;
	uint32 *src32, *dst32;
	uint16 *src16, *dst16;
	uint8 r,g,b,a;

	if(src->dtype == (uint32)dtype)
		return image_clone(src);

	CHECK_AND_UNSWIZZLE(src);
	dst = image_create(src->w, src->h, dtype);

	src32 = (uint32*)src->data;
	dst32 = (uint32*)dst->data;
	src16 = (uint16*)src->data;
	dst16 = (uint16*)dst->data;

	for(i = 0; i < src->h; i++)
	{
		for (j = 0; j<src->w; j++)
		{
			if(dtype == DISPLAY_PIXEL_FORMAT_8888){
				if(src->dtype == DISPLAY_PIXEL_FORMAT_4444){
					GET_RGBA_4444(src16[i*src->texw+j],r,g,b,a);
					dst32[i*dst->texw+j] = MAKE_RGBA_8888(r,g,b,a);
				}
				else if(src->dtype == DISPLAY_PIXEL_FORMAT_5551){
					GET_RGBA_5551(src16[i*src->texw+j],r,g,b,a);
					dst32[i*dst->texw+j] = MAKE_RGBA_8888(r,g,b,a);
				}
				else if(src->dtype == DISPLAY_PIXEL_FORMAT_565){
					GET_RGBA_565(src16[i*src->texw+j],r,g,b,a);
					dst32[i*dst->texw+j] = MAKE_RGBA_8888(r,g,b,a);
				}
			}
			else if(dtype == DISPLAY_PIXEL_FORMAT_4444){
				if(src->dtype == DISPLAY_PIXEL_FORMAT_8888){
					GET_RGBA_8888(src32[i*src->texw+j],r,g,b,a);
					dst16[i*dst->texw+j] = MAKE_RGBA_4444(r,g,b,a);
				}
				else if(src->dtype == DISPLAY_PIXEL_FORMAT_5551){
					GET_RGBA_5551(src16[i*src->texw+j],r,g,b,a);
					dst16[i*dst->texw+j] = MAKE_RGBA_4444(r,g,b,a);
				}
				else if(src->dtype == DISPLAY_PIXEL_FORMAT_565){
					GET_RGBA_565(src16[i*src->texw+j],r,g,b,a);
					dst16[i*dst->texw+j] = MAKE_RGBA_4444(r,g,b,a);
				}
			}
			else if(dtype == DISPLAY_PIXEL_FORMAT_5551){
				if(src->dtype == DISPLAY_PIXEL_FORMAT_8888){
					GET_RGBA_8888(src32[i*src->texw+j],r,g,b,a);
					dst16[i*dst->texw+j] = MAKE_RGBA_5551(r,g,b,a);
				}
				else if(src->dtype == DISPLAY_PIXEL_FORMAT_4444){
					GET_RGBA_4444(src16[i*src->texw+j],r,g,b,a);
					dst16[i*dst->texw+j] = MAKE_RGBA_5551(r,g,b,a);
				}
				else if(src->dtype == DISPLAY_PIXEL_FORMAT_565){
					GET_RGBA_565(src16[i*src->texw+j],r,g,b,a);
					dst16[i*dst->texw+j] = MAKE_RGBA_5551(r,g,b,a);
				}
			}
			else if(dtype == DISPLAY_PIXEL_FORMAT_565){
				if(src->dtype == DISPLAY_PIXEL_FORMAT_8888){
					GET_RGBA_8888(src32[i*src->texh+j],r,g,b,a);
					dst16[i*dst->texh+j] = MAKE_RGBA_565(r,g,b,a);
				}
				else if(src->dtype == DISPLAY_PIXEL_FORMAT_4444){
					GET_RGBA_4444(src16[i*src->texh+j],r,g,b,a);
					dst16[i*dst->texh+j] = MAKE_RGBA_565(r,g,b,a);
				}
				else if(src->dtype == DISPLAY_PIXEL_FORMAT_5551){
					GET_RGBA_5551(src16[i*src->texh+j],r,g,b,a);
					dst16[i*dst->texh+j] = MAKE_RGBA_565(r,g,b,a);
				}
			}
		}
	}
	dst->swizzle = 1;
	swizzle_swap(dst);
	CHECK_AND_SWIZZLE(src);
	return dst;
}

// for image_scale
#define FILTER_PI  (3.1415926535897932384626433832795)
#define FILTER_2PI (2.0 * 3.1415926535897932384626433832795)
#define FILTER_4PI (4.0 * 3.1415926535897932384626433832795)

enum{
	FILTER_BOX = 0,
	FILTER_BILINEAR,
	FILTER_GAUSSIAN,
	FILTER_HAMMING,
	FILTER_BLACKMAN,
};

enum{
	FILTER_SET_WIDTH = 0,
	FILTER_GET_WIDTH,
};

#define FILTER_SET_PARAM_IMP(type)                                      \
	int filter_##type##_set_param(void *this,double val,int flags) {    \
		assert(this != NULL);											\
		switch(flags) {													\
		case FILTER_SET_WIDTH:											\
			((Filter_##type *)this)->width = val;                       \
			return 1;													\
		default:														\
			break;														\
		}																\
		return 0;														\
	}

#define FILTER_GET_PARAM_IMP(type)                                      \
	int filter_##type##_get_param(void *this,double *pval,int flags) {  \
		assert((this != NULL) && (pval != NULL));						\
		switch(flags) {													\
		case FILTER_GET_WIDTH:											\
			*pval = ((Filter_##type *)this)->width;						\
			return 1;													\
		default:														\
			break;														\
		}																\
		return 0;														\
	}

#define FILTER_CREATE_IMP(type, val)                                    \
	void* filter_##type##_create()                                      \
	{																	\
		Filter_##type * type = (Filter_##type *)malloc(sizeof(Filter_##type)); \
		type->width  = val;												\
		return (void*)type;												\
	}

//////////////////////////////////////////////////////////////////////////
//BOX FILTER BEGIN
typedef struct _filter_box {
	double     width;
}Filter_box;

double filter_box_proc (void *this, double val)
{
	return (fabs(val) <= ((Filter_box*)this)->width ? 1.0 : 0.0);
}

FILTER_SET_PARAM_IMP(box)
FILTER_GET_PARAM_IMP(box)
FILTER_CREATE_IMP(box, 0.5)

//BOX FILTER END
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//Bilinear FILTER BEGIN
typedef struct _filter_bilinear{
	double     width;
}Filter_bilinear;

double filter_bilinear_proc (void *this,double val)
{
	double width = ((Filter_bilinear*)this)->width;
	val = fabs(val);
	return (val < width ? width - val : 0.0);
}

FILTER_SET_PARAM_IMP(bilinear)
FILTER_GET_PARAM_IMP(bilinear)
FILTER_CREATE_IMP(bilinear, 1.0)

//Bilinear FILTER END
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//Gaussian FILTER BEGIN
typedef struct _filter_gaussian{
	double     width;
}Filter_gaussian;

double filter_gaussian_proc (void *this,double val)
{
	return (fabs(val) > ((Filter_gaussian*)this)->width ? 0.0 : (exp(-val * val / 2.0)) / sqrt(FILTER_2PI));
}

FILTER_SET_PARAM_IMP(gaussian)
FILTER_GET_PARAM_IMP(gaussian)
FILTER_CREATE_IMP(gaussian, 3.0)

//Gaussian FILTER END
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//Hamming FILTER BEGIN
typedef struct _filter_hamming{
	double     width;
}Filter_hamming;

double filter_hamming_proc (void *this,double val)
{
	if (fabs (val) > ((Filter_hamming*)this)->width)
		return 0.0;
	{
		double window = 0.54 + 0.46 * cos (FILTER_2PI * val);
		double sinc = (val == 0) ? 1.0 : sin (FILTER_PI * val) / (FILTER_PI * val);
		return window * sinc;
	}
}

FILTER_SET_PARAM_IMP(hamming)
FILTER_GET_PARAM_IMP(hamming)
FILTER_CREATE_IMP(hamming, 0.5)

//Hamming FILTER END
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//Blackman FILTER BEGIN
typedef struct _filter_blackman{
	double     width;
}Filter_blackman;

double filter_blackman_proc (void *this,double val)
{
	double width = ((Filter_blackman*)this)->width;
	if (fabs (val) > width)
		return 0.0;
	{
		double d = 2.0 * width + 1.0;
		return (0.42 + 0.5 * cos (FILTER_2PI * val / ( d - 1.0 )) +
				0.08 * cos (FILTER_4PI * val / ( d - 1.0 )));
	}
}

FILTER_SET_PARAM_IMP(blackman)
FILTER_GET_PARAM_IMP(blackman)
FILTER_CREATE_IMP(blackman, 0.5)

//Blackman FILTER END
//////////////////////////////////////////////////////////////////////////

// we do not need it now
#undef FILTER_CREATE_IMP
#undef FILTER_GET_PARAM_IMP
#undef FILTER_SET_PARAM_IMP

typedef void* (*Filter_create)();
typedef double (*Filter_proc)(void *this,double val);
typedef int (*Filter_set_param)(void *this,double val,int flags);
typedef int (*Filter_get_param)(void *this,double* pval,int flags);

#define DEFINE_FILTER_FUNS(func)					\
	const Filter_##func filter_##func##_funs[] = {	\
		filter_box_##func,							\
		filter_bilinear_##func,						\
		filter_gaussian_##func,						\
		filter_hamming_##func,						\
		filter_blackman_##func,						\
	};

DEFINE_FILTER_FUNS(create)
DEFINE_FILTER_FUNS(proc)
DEFINE_FILTER_FUNS(get_param)
DEFINE_FILTER_FUNS(set_param)
void filter_destroy(void *this)
{
	free(this);
}

// we do not need it now
#undef DEFINE_FILTER_FUNS

// c10n == contribution
typedef struct
{
   double *weights;  // Normalized weights of neighboring pixels
   int left,right;   // Bounds of source pixels window
} C10n;  // Contirbution information for a single pixel

typedef struct
{
   C10n *row;  // Row (or column) of contribution weights
   uint32 win_size,              // Filter window size (of affecting source pixels)
		  line_len;              // Length of line (no. or rows / cols)
} Line_c10n;                // Contribution information for an entire line (row or column)

Line_c10n* alloc_contributions (uint32 line_length, uint32 window_size)
{
	uint32 i = 0;
	Line_c10n *res = (Line_c10n*)malloc(sizeof(Line_c10n));
	// Init structure header
	res->win_size = window_size;
	res->line_len  = line_length;
	// Allocate list of contributions
	res->row = (C10n*)malloc(sizeof(C10n)*line_length);//new ContributionType[uLineLength];
	for(; i < line_length ; i++)
	{
		// Allocate contributions for every pixel
		res->row[i].weights = (double*)malloc(sizeof(double)*window_size);//new double[uWindowSize];
		memset(res->row[i].weights,0,sizeof(double)*window_size);
	}
	return res;
}

void free_contributions (Line_c10n *p)
{
	uint32 i = 0;
	for (; i < p->line_len; i++)
	{
		// Free contribs for every pixel
		free(p->row[i].weights);
	}
	free(p->row);    // Free list of pixels contribs
	free(p);                   // Free contribs header
}

Line_c10n* calc_contributions (void* filter,int filter_type, uint32 line_size, uint32 src_size, double scale)
{
	//FilterClass CurFilter;
	uint32 u;
	int left, right , window_size, i;
	double width, center, total_weight;
	double fscale = 1.0;
	double fwidth;
	Line_c10n *res;
	filter_get_param_funs[filter_type](filter, &fwidth, FILTER_GET_WIDTH);
	if (scale < 1.0)
	{    // Minification
		width = fwidth / scale;
		fscale = scale;
	}
	else
	{    // Magnification
		width= fwidth;
	}

	// Window size is the number of sampled pixels
	window_size = 2 * (int)ceil(width) + 1;

	// Allocate a new line contributions strucutre
	res = alloc_contributions (line_size, window_size);

	for (u = 0; u < line_size; u++)
	{   // Scan through line of contributions
		center = (double)u / scale;   // Reverse mapping
		// Find the significant edge points that affect the pixel
		left  = _max (0, (int)floor (center - width));
		(*(res->row+u)).left = left;
		right = _min ((int)ceil (center + width), (int)src_size - 1);
		(*(res->row+u)).right = right;

		// Cut edge points to fit in filter window in case of spill-off
		if (right - left + 1 > window_size)
		{
			if (left < ((int)src_size - 1 / 2))
			{
				left++;
			}
			else
			{
				right--;
			}
		}
		total_weight = 0.0;  // Zero sum of weights
		for (i = left; i <= right; i++)
		{   // Calculate weights
			res->row[u].weights[i-left] =
				fscale * filter_proc_funs[filter_type](filter,fscale * (center - (double)i));
			total_weight += res->row[u].weights[i-left];
		}
		if (total_weight > 0.0)
		{   // Normalize weight of neighbouring points
			for (i = left; i <= right; i++)
			{   // Normalize point
				res->row[u].weights[i-left] /= total_weight;
			}
		}
   }
   return res;
}

#define __SCALE_LOOP(type, bit, dw_or_dh, _src_n, _dst_n)				\
	for (ii = 0; ii < dw_or_dh; ii++)									\
	{																	\
		left  = contrib->row[ii].left;									\
		right = contrib->row[ii].right;									\
		r = 0,g = 0,b = 0,a = 0;										\
		for (i = left; i <= right; i++)									\
		{																\
																		\
			GET_RGBA_##type(src##bit[ _src_n ],sr,sg,sb,sa);			\
			if(sa==0)													\
				continue;												\
			r += (uint8)(contrib->row[ii].weights[i-left] * (double)(sr)); \
			g += (uint8)(contrib->row[ii].weights[i-left] * (double)(sg)); \
			b += (uint8)(contrib->row[ii].weights[i-left] * (double)(sb)); \
			a += (uint8)(contrib->row[ii].weights[i-left] * (double)(sa)); \
		}																\
		dst##bit[ _dst_n ] = MAKE_RGBA_##type(r,g,b,a);					\
	}

#define SCALE_FUN_IMP(type, dw_or_dh, _sdata, _ddata, _src_n, _dst_n)	\
	void scale_##type (uint8 *sdata, uint32 sw, uint8 *ddata, uint32 dw_or_dh, uint32 u, Line_c10n *contrib,uint32 pitch,uint32 dtype) \
	{																	\
		uint32 ii, i;													\
		uint8 r,g,b,a,sr,sb,sg,sa;										\
		uint32 left,right;												\
		uint32 *src32,*dst32;											\
		uint16 *src16,*dst16;											\
		src32 = (uint32*) _sdata;										\
		dst32 = (uint32*) _ddata;										\
		src16 = (uint16*) _sdata;										\
		dst16 = (uint16*) _ddata;										\
		if(dtype == DISPLAY_PIXEL_FORMAT_8888){							\
			__SCALE_LOOP(8888, 32, dw_or_dh, _src_n, _dst_n);			\
		}																\
		else if(dtype == DISPLAY_PIXEL_FORMAT_4444){					\
			__SCALE_LOOP(4444, 16, dw_or_dh, _src_n, _dst_n);			\
		}																\
		else if(dtype == DISPLAY_PIXEL_FORMAT_5551){					\
			__SCALE_LOOP(5551, 16, dw_or_dh, _src_n, _dst_n);			\
		}																\
		else if(dtype == DISPLAY_PIXEL_FORMAT_565){						\
			__SCALE_LOOP(565, 16, dw_or_dh, _src_n, _dst_n);			\
		}																\
	}

SCALE_FUN_IMP(row, dw, sdata+u*pitch, ddata+u*dw, i, ii)
SCALE_FUN_IMP(col, dh, sdata, ddata, i*sw+u, ii*pitch+u)

#define DIRE_SCALE_FUN_IMP(type, stype, dire, dire2)                     \
	void type##_scale (void* filter, int filter_type, uint8 *sdata, uint32 sw, uint32 sh, uint8 *ddata, uint32 dw, uint32 dh,uint32 pitch,uint32 dtype) \
	{																	\
		uint32  u;														\
		Line_c10n * contrib;											\
		if (d##dire == s##dire)											\
		{    /* No scaling required, just copy */						\
			if(dtype == DISPLAY_PIXEL_FORMAT_8888)						\
				memcpy (ddata, sdata, sizeof (uint32) * sw * sh);		\
			else														\
				memcpy (ddata, sdata, sizeof (uint16) * sw * sh);		\
		}																\
		/* Allocate and calculate the contributions */					\
		contrib = calc_contributions (filter, filter_type, d##dire, s##dire, (double)d##dire / (double)s##dire); \
		for (u = 0; u < d##dire2; u++)									\
		{    /* Step through rows */									\
			scale_##stype (sdata, sw, ddata, d##dire, u, contrib, pitch, dtype); \
		}																\
		free_contributions (contrib);   /* Free contributions structure */ \
	}

DIRE_SCALE_FUN_IMP(horiz, row,  w, h)
DIRE_SCALE_FUN_IMP(vert, col, h, w)

//image scale
image_p image_scale(image_p src, int w, int h,int mode)
{
	image_p dst;
	uint8 * temp;
	uint32 spitch, dpitch;
	void* filter = filter_create_funs[mode]();
	dst = image_create(w,h,src->dtype);
	dst->dontswizzle = 1;
	CHECK_AND_UNSWIZZLE(src);
	temp = malloc(dst->w*src->h*src->bpb);
	memset(temp,0,dst->w*src->h*src->bpb);
	spitch = src->texw;
	dpitch = dst->texw;
	horiz_scale(filter, mode, src->data,src->w,src->h,temp,dst->w,src->h,spitch,src->dtype);
	vert_scale (filter, mode, temp,dst->w,src->h,dst->data,dst->w,dst->h,dpitch,src->dtype);
	free(temp);
	filter_destroy(filter);
	CHECK_AND_SWIZZLE(src);
	return dst;
}

#ifndef NGE_PSP

//for image_hue_rotate
//Thanks to Paul Haeberli
#define M_PI	(3.1415926535f)
#define RLUM    (0.3086f)
#define GLUM    (0.6094f)
#define BLUM    (0.0820f)

//multiply two matrixes
void matrixmult(float a[4][4], float b[4][4], float c[4][4])
{
    int x, y;
    float temp[4][4];

    for(y = 0; y < 4; y++)
        for(x = 0; x < 4; x++) {
            temp[y][x] = b[y][0] * a[0][x]
                       + b[y][1] * a[1][x]
                       + b[y][2] * a[2][x]
                       + b[y][3] * a[3][x];
        }
    for(y = 0; y < 4; y++)
        for(x = 0; x < 4; x++)
            c[y][x] = temp[y][x];
}

//transpose a matrix
void matrixtrans(float a[4][4])
{
    float temp;
	int y, x;
    for(y = 0; y < 4; y++)
        for(x = 0; x < y; x++) {
            temp = a[y][x];
            a[y][x] = a[x][y];
            a[x][y] = temp;
        }   
}

//generate an identity matrix
void identmat(float *matrix)
{
    *matrix++ = 1.0;    /* row 1        */
    *matrix++ = 0.0;
    *matrix++ = 0.0;
    *matrix++ = 0.0;
    *matrix++ = 0.0;    /* row 2        */
    *matrix++ = 1.0;
    *matrix++ = 0.0;
    *matrix++ = 0.0;
    *matrix++ = 0.0;    /* row 3        */
    *matrix++ = 0.0;
    *matrix++ = 1.0;
    *matrix++ = 0.0;
    *matrix++ = 0.0;    /* row 4        */
    *matrix++ = 0.0;
    *matrix++ = 0.0;
    *matrix++ = 1.0;
}

//transform a 3D point
void xformpnt(float matrix[4][4], float x, float y, float z, float *tx, float *ty, float *tz)
{
    *tx = x*matrix[0][0] + y*matrix[1][0] + z*matrix[2][0] + matrix[3][0];
    *ty = x*matrix[0][1] + y*matrix[1][1] + z*matrix[2][1] + matrix[3][1];
    *tz = x*matrix[0][2] + y*matrix[1][2] + z*matrix[2][2] + matrix[3][2];
}

//shear a matrix
void zshearmat(float mat[4][4], float dx, float dy)
{
    float mmat[4][4];

    mmat[0][0] = 1.0;
    mmat[0][1] = 0.0;
    mmat[0][2] = dx;
    mmat[0][3] = 0.0;

    mmat[1][0] = 0.0;
    mmat[1][1] = 1.0;
    mmat[1][2] = dy;
    mmat[1][3] = 0.0;

    mmat[2][0] = 0.0;
    mmat[2][1] = 0.0;
    mmat[2][2] = 1.0;
    mmat[2][3] = 0.0;

    mmat[3][0] = 0.0;
    mmat[3][1] = 0.0;
    mmat[3][2] = 0.0;
    mmat[3][3] = 1.0;
    matrixmult(mmat,mat,mat);
}

//z-rotate a matrix
void zrotatemat(float mat[4][4], float rs, float rc)
{
    float mmat[4][4];

    mmat[0][0] = rc;
    mmat[0][1] = rs;
    mmat[0][2] = 0.0;
    mmat[0][3] = 0.0;

    mmat[1][0] = -rs;
    mmat[1][1] = rc;
    mmat[1][2] = 0.0;
    mmat[1][3] = 0.0;

    mmat[2][0] = 0.0;
    mmat[2][1] = 0.0;
    mmat[2][2] = 1.0;
    mmat[2][3] = 0.0;

    mmat[3][0] = 0.0;
    mmat[3][1] = 0.0;
    mmat[3][2] = 0.0;
    mmat[3][3] = 1.0;
    matrixmult(mmat,mat,mat);
}

int image_hue_rotate(image_p pimage, float rot)
{
	float mat1[4][4], mmat[4][4];
	float lx, ly, lz;
	float mag;
	float zsx, zsy;
	float zrs, zrc;
	int n;
	uint16 *bmp16;
	uint32 *bmp32;
	uint16 ir, ig, ib;
	uint16 r, g, b, a;
	
	if(!pimage)	return 0;
	
	identmat(mat1);
	identmat(mmat);
	
	/* rotate the grey vector into positive Z */
	mag = sqrt(3.0f);
	mag = 1.0f / mag;
	mat1[0][0] = mat1[0][2] = mat1[1][2] = mat1[2][2] = mag;
	mat1[1][0] = -mag;
	mag = sqrt(2.0f);
	mat1[0][0] *= mag;
	mag = 1.0f / mag;
	mat1[1][0] *= mag;
	mat1[2][0] = mat1[1][0];
	mat1[1][1] = mag;
	mat1[2][1] = -mag;
	
	/* shear the space to make the luminance plane horizontal */
	xformpnt(mat1, RLUM, GLUM, BLUM, &lx, &ly, &lz);
	zsx = lx / lz;
	zsy = ly / lz;
	matrixmult(mmat, mat1, mmat);
	zshearmat(mmat,zsx,zsy);
	
	matrixtrans(mat1);
	
	/* rotate the hue */
	zrs = sin(rot*M_PI/180.0f);
	zrc = cos(rot*M_PI/180.0f);
	zrotatemat(mmat,zrs,zrc);
	
	/* unshear the space to put the luminance plane back */
	zshearmat(mmat,-zsx,-zsy);
	
	matrixmult(mat1, mmat, mmat);
	
	n = pimage->texh * pimage->texw;
	pimage->modified = 1;
	switch(pimage->dtype)	{
		case DISPLAY_PIXEL_FORMAT_565:
			bmp16 = (uint16*)pimage->data;
			while(n--)	{
				GET_RGBA_565(*bmp16, ir, ig, ib, a)
				r = ir*mmat[0][0] + ig*mmat[1][0] + ib*mmat[2][0];
				g = ir*mmat[0][1] + ig*mmat[1][1] + ib*mmat[2][1];
				b = ir*mmat[0][2] + ig*mmat[1][2] + ib*mmat[2][2];
				if(r<0) r = 0;
				if(r>255) r = 255;
				if(g<0) g = 0;
				if(g>255) g = 255;
				if(b<0) b = 0;
				if(b>255) b = 255;
				*(bmp16++) = MAKE_RGBA_565(r, g, b, a);
			}
			break;
		case DISPLAY_PIXEL_FORMAT_5551:
			bmp16 = (uint16*)pimage->data;
			while(n--)	{
				GET_RGBA_5551(*bmp16, ir, ig, ib, a)
				r = 1.0f*ir*mmat[0][0] + 1.0f*ig*mmat[1][0] + 1.0f*ib*mmat[2][0];
				g = 1.0f*ir*mmat[0][1] + 1.0f*ig*mmat[1][1] + 1.0f*ib*mmat[2][1];
				b = 1.0f*ir*mmat[0][2] + 1.0f*ig*mmat[1][2] + 1.0f*ib*mmat[2][2];
				if(r<0) r = 0;
				if(r>255) r = 255;
				if(g<0) g = 0;
				if(g>255) g = 255;
				if(b<0) b = 0;
				if(b>255) b = 255;
				*(bmp16++) = MAKE_RGBA_5551(r, g, b, a);
			}
			break;
		case DISPLAY_PIXEL_FORMAT_4444:
			bmp16 = (uint16*)pimage->data;
			while(n--)	{
				GET_RGBA_4444(*bmp16, ir, ig, ib, a)
				r = 1.0f*ir*mmat[0][0] + 1.0f*ig*mmat[1][0] + 1.0f*ib*mmat[2][0];
				g = 1.0f*ir*mmat[0][1] + 1.0f*ig*mmat[1][1] + 1.0f*ib*mmat[2][1];
				b = 1.0f*ir*mmat[0][2] + 1.0f*ig*mmat[1][2] + 1.0f*ib*mmat[2][2];
				if(r<0) r = 0;
				if(r>255) r = 255;
				if(g<0) g = 0;
				if(g>255) g = 255;
				if(b<0) b = 0;
				if(b>255) b = 255;
				*(bmp16++) = MAKE_RGBA_4444(r, g, b, a);
			}
			break;
		case DISPLAY_PIXEL_FORMAT_8888:
			bmp32 = (uint32*)pimage->data;
			while(n--)	{
				GET_RGBA_8888(*bmp32, ir, ig, ib, a)
				r = 1.0f*ir*mmat[0][0] + 1.0f*ig*mmat[1][0] + 1.0f*ib*mmat[2][0];
				g = 1.0f*ir*mmat[0][1] + 1.0f*ig*mmat[1][1] + 1.0f*ib*mmat[2][1];
				b = 1.0f*ir*mmat[0][2] + 1.0f*ig*mmat[1][2] + 1.0f*ib*mmat[2][2];
				if(r<0) r = 0;
				if(r>255) r = 255;
				if(g<0) g = 0;
				if(g>255) g = 255;
				if(b<0) b = 0;
				if(b>255) b = 255;
				*(bmp32++) = MAKE_RGBA_8888(r, g, b, a);
			}
			break;
	}
	return 1;
}

#else
//hue rotate for PSP
//VFPU accerleration
void GetHueMatrix(ScePspFMatrix4 *mat, float rot)	{
	__asm__ volatile (
		"vfim.s		S000, 0.3086\n"
		"vfim.s		S010, 0.6094\n"
		"vfim.s		S020, 0.0820\n"
		"vadd.s		S030, S030[0], S030[1]\n"
		"vcst.s		S001, VFPU_SQRT1_2\n"
		"vcst.s		S011, VFPU_SQRT3_2\n"
		"vrcp.s		S011, S011\n"
		"vmidt.q	M100\n"
		"vmul.s		S100, S011, S001\n"
		"vmul.s		S101, S100, S001\n"
		"vadd.t		C120, C101[0, 0, 0], C101[x, x, x]\n"
		"vmul.s		S101, S101, S001\n"
		"vneg.s		S101, S101\n"
		"vadd.s		S102, S101[0], S101[x]\n"
		"vadd.s		S111, S001[0], S001[x]\n"
		"vadd.s		S112, S001[0], S001[x]\n"
		"vneg.s		S112, S112\n"
		"vadd.s		S133, S133[0], S133[1]\n"
		"vtfm4.q	R001, M100, R000\n"
		"vrcp.s		S021, S021\n"
		"vmul.p		R001, R001[x, y], R021[x, x]\n"
		"vmidt.q	M200\n"
		"vadd.s		S220, S001[0], S001[x]\n"
		"vadd.s		S221, S011[0], S011[x]\n"
		"vmmul.q	M000, M100, M200\n"
		"mtv		%1, S500\n"
		"vfim.s		S510, 90.0\n"
		"vrcp.s		S510, S510\n"
		"vmul.s		S500, S500, S510\n"
		"vsin.s		S501, S500\n"
		"vcos.s		S502, S500\n"
		"vmidt.q	M300\n"
		//y - sin  z - cos
		"vadd.q		C300, C500[0, 0, 0, 0], C500[z, -y, 0, 0]\n"
		"vadd.q		C310, C500[0, 0, 0, 0], C500[y,  z, 0, 0]\n"
		"vmmul.q	M400, M000, M300\n"
		"vneg.s		S220, S220\n"
		"vneg.s		S221, S221\n"
		"vmmul.q	M000, M400, M200\n"
		"vmmul.q	M200, M000, E100\n"
		
		"sv.q		C200, 0(%0)\n"
		"sv.q		C210, 16(%0)\n"
		"sv.q		C220, 32(%0)\n"
		"sv.q		C230, 48(%0)\n"
		::"r"(mat), "r"(rot));
}

int image_hue_rotate(image_p pimage, float rot)	{
	ScePspFMatrix4 hue_mat;
	if(rot < 0.0001f && rot > -0.0001f)
		return 1;
	GetHueMatrix(&hue_mat, rot);
	if(!pimage)	return 0;
	pimage->modified = 1;
	switch(pimage->dtype)	{
		case DISPLAY_PIXEL_FORMAT_565:
			__asm__ volatile (
				"lv.q		C000, 0 + %1\n"
				"lv.q		C010, 16 + %1\n"
				"lv.q		C020, 32 + %1\n"
				"lv.q		C030, 48 + %1\n"
				"vfim.s		S102, 63.0\n"
				"U565:"
				"blez		%2, D565\n"
				"lhu		$t5, 0(%0)\n"
				"srl		$t6, $t5, 10\n"
				"andi		$t6, $t6, 0x3E\n"
				"mtv		$t6, S120\n"
				"srl		$t6, $t5, 5\n"
				"andi		$t6, $t6, 0x3F\n"
				"mtv		$t6, S110\n"
				"sll		$t6, $t5, 1\n"
				"andi		$t6, $t6, 0x3E\n"
				"mtv		$t6, S100\n"
				"vi2f.q		R100, R100, 0\n"
				"vtfm4.q	R101, M000, R100\n"
				"vmax.q		R101, R101[x, y, z, w], R101[0, 0, 0, 0]\n"
				"vmin.q		R101, R101[x, y, z, w], R102[x, x, x, x]\n"
				"vf2in.q	R101, R101, 0\n"
				"andi		$t5, $t5, 0x0\n"
				"mfv		$t6, S101\n"
				"srl		$t6, $t6, 1\n"
				"or			$t5, $t5, $t6\n"
				"mfv		$t6, S111\n"
				"sll		$t6, $t6, 5\n"
				"or			$t5, $t5, $t6\n"
				"mfv		$t6, S121\n"
				"srl		$t6, $t6, 1\n"
				"sll		$t6, $t6, 11\n"
				"or			$t5, $t5, $t6\n"
				"sh			$t5, 0(%0)\n"
				"addi		%0, %0, 2\n"
				"addi		%2, %2, -1\n"
				"j			U565\n"
				"D565:\n"
				::"r"(pimage->data), "m"(hue_mat), "r"(pimage->texw * pimage->texh):"memory");
			break;
		case DISPLAY_PIXEL_FORMAT_5551:
			__asm__ volatile (
				"lv.q		C000, 0 + %1\n"
				"lv.q		C010, 16 + %1\n"
				"lv.q		C020, 32 + %1\n"
				"lv.q		C030, 48 + %1\n"
				"vfim.s		S102, 31.0\n"
				"U5551:"
				"blez		%2, D5551\n"
				"lhu		$t5, 0(%0)\n"
				"srl		$t6, $t5, 10\n"
				"andi		$t6, $t6, 0x1F\n"
				"mtv		$t6, S120\n"
				"srl		$t6, $t5, 5\n"
				"andi		$t6, $t6, 0x1F\n"
				"mtv		$t6, S110\n"
				"andi		$t6, $t5, 0x1F\n"
				"mtv		$t6, S100\n"
				"vi2f.q		R100, R100, 0\n"
				"vtfm4.q	R101, M000, R100\n"
				"vmax.q		R101, R101[x, y, z, w], R101[0, 0, 0, 0]\n"
				"vmin.q		R101, R101[x, y, z, w], R102[x, x, x, x]\n"
				"vf2in.q	R101, R101, 0\n"
				"andi		$t5, 0x8000\n"
				"mfv		$t6, S101\n"
				"or			$t5, $t5, $t6\n"
				"mfv		$t6, S111\n"
				"sll		$t6, $t6, 5\n"
				"or			$t5, $t5, $t6\n"
				"mfv		$t6, S121\n"
				"sll		$t6, $t6, 10\n"
				"or			$t5, $t5, $t6\n"
				"sh			$t5, 0(%0)\n"
				"addi		%0, %0, 2\n"
				"addi		%2, %2, -1\n"
				"j			U5551\n"
				"D5551:\n"
				::"r"(pimage->data), "m"(hue_mat), "r"(pimage->texw * pimage->texh):"memory");
			break;
		case DISPLAY_PIXEL_FORMAT_4444:
			__asm__ volatile (
				"lv.q		C000, 0 + %1\n"
				"lv.q		C010, 16 + %1\n"
				"lv.q		C020, 32 + %1\n"
				"lv.q		C030, 48 + %1\n"
				"vfim.s		S102, 15.0\n"
				"U4444:"
				"blez		%2, D4444\n"
				"lhu		$t5, 0(%0)\n"
				"srl		$t6, $t5, 8\n"
				"andi		$t6, $t6, 0xF\n"
				"mtv		$t6, S120\n"
				"srl		$t6, $t5, 4\n"
				"andi		$t6, $t6, 0xF\n"
				"mtv		$t6, S110\n"
				"andi		$t6, $t5, 0xF\n"
				"mtv		$t6, S100\n"
				"vi2f.q		R100, R100, 0\n"
				"vtfm4.q	R101, M000, R100\n"
				"vmax.q		R101, R101[x, y, z, w], R101[0, 0, 0, 0]\n"
				"vmin.q		R101, R101[x, y, z, w], R102[x, x, x, x]\n"
				"vf2in.q	R101, R101, 0\n"
				"andi		$t5, 0xF000\n"
				"mfv		$t6, S101\n"
				"or			$t5, $t5, $t6\n"
				"mfv		$t6, S111\n"
				"sll		$t6, $t6, 4\n"
				"or			$t5, $t5, $t6\n"
				"mfv		$t6, S121\n"
				"sll		$t6, $t6, 8\n"
				"or			$t5, $t5, $t6\n"
				"sh			$t5, 0(%0)\n"
				"addi		%0, %0, 2\n"
				"addi		%2, %2, -1\n"
				"j			U4444\n"
				"D4444:\n"
				::"r"(pimage->data), "m"(hue_mat), "r"(pimage->texw * pimage->texh):"memory");
			break;
		case DISPLAY_PIXEL_FORMAT_8888:
			__asm__ volatile (
				"lv.q		C000, 0 + %1\n"
				"lv.q		C010, 16 + %1\n"
				"lv.q		C020, 32 + %1\n"
				"lv.q		C030, 48 + %1\n"
				"vfim.s		S102, 255.0\n"
				"U8888:"
				"blez		%2, D8888\n"
				"lv.s		S200, 0(%0)\n"
				".word		0xD0380000 | (8 << 8) | (36)\n"//"vuc2i.s	R100, S200\n"
				"vi2f.q		R100, R100, 23\n"
				"vtfm4.q	R101, M000, R100\n"
				"vmax.q		R101, R101[x, y, z, w], R101[0, 0, 0, 0]\n"
				"vmin.q		R101, R101[x, y, z, w], R102[x, x, x, x]\n"
				"vf2iz.q	R101, R101, 23\n"
				"vi2uc.q	S200, R101\n"
				"sv.s		S200, 0(%0)\n"
				"addi		%0, %0, 4\n"
				"addi		%2, %2, -1\n"
				"j			U8888\n"
				"D8888:\n"
				::"r"(pimage->data), "m"(hue_mat), "r"(pimage->texw * pimage->texh):"memory");
			break;
	}
	return 1;
}
#endif

void image_fillrect(image_p pimage, int x, int y, int w, int h, int color) {
	int i, j;
	uint16 *img16;
	uint32 *img32;
	if(w + x > pimage->texw)
		w = pimage->texw - x;
	if(h + y > pimage->texh)
		h = pimage->texh - y;
	pimage->modified = 1;
	if(pimage->bpb == 2) {
		img16 = ((uint16*)pimage->data) + y * pimage->texw + x;
		for(i = 0; i < h && i < 1; i++)
			for(j = 0; j < w; j++)
				*(img16 + j) = (uint16)(color & 0xffff);
		for(; i < h; i++)
			memcpy(img16 + i * pimage->texw, img16, w * pimage->bpb);
	}
	else {
		img32 = ((uint32*)pimage->data) + y * pimage->texw + x;
		for(i = 0; i < h && i < 1; i++)
			for(j = 0; j < w; j++)
				*(img32 + j) = color;
		for(; i < h; i++)
			memcpy(img32 + i * pimage->texw, img32, w * pimage->bpb);
	}
}
