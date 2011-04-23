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
	uint8 recover = 0;
	image_p pimg;
	uint16 *p16 = NULL,*psrc16 = NULL;
	uint32 *p32 = NULL,*psrc32 = NULL;

	if(gray<0 || gray>100)
	{
		nge_print("create_gray_image arg 'gray' must between 0 to 100！");
		return NULL;
	}
	if(src->swizzle ==1){
		unswizzle_swap(src);
		recover = 1;
	}
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
	if(recover)
		swizzle_swap(src);

	return pimg;
}

int get_saturation_brightness_color(int dtype, int scol,int saturation, int brightness)
{
	int gcol, r, g, b, a, mincol, maxcol;
	float D;
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
	// 处理饱和度
	if(saturation!=0)
	{
		gcol = ((r + g + b) / 3) & 0xff;
		maxcol = r>g ? (r>b?r:b):g;
		mincol = r>g ? (g>b?b:g):r;
		//gcol = maxcol - mincol;
		D = (maxcol - mincol)*1.0f;
		if(maxcol==mincol)
		{

		}
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
	uint8 recover = 0;
	image_p pimg;
	uint16 *p16,*psrc16;
	uint32 *p32,*psrc32;

	if(saturation<-100 || saturation>100)
	{
		nge_print("create_gray_image arg 'saturation' must between -100 to 100！");
		return NULL;
	}

	if(brightness<-100 || brightness>100)
	{
		nge_print("create_gray_image arg 'brightness' must between -100 to 100！");
		return NULL;
	}
	if(src->swizzle ==1){
		unswizzle_swap(src);
		recover = 1;
	}
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
				*(p32 + x) = (uint32)get_saturation_brightness_color(pimg->dtype, *(psrc32 + x), saturation, brightness);
			}
			else
			{
				*(p16 + x) = (uint16)get_saturation_brightness_color(pimg->dtype, *(psrc16 + x), saturation, brightness);
			}
		}
	}
	if(recover)
		swizzle_swap(src);

	return pimg;
}

//image conv
image_p image_conv(image_p src, int dtype)
{
	image_p dst;
	uint8 recover = 0;
	uint32 i,j;
	uint32 *src32, *dst32;
	uint16 *src16, *dst16;
	uint8 r,g,b,a;

	if(src->dtype == dtype)
		return image_clone(src);

	if(src->swizzle ==1){
		unswizzle_swap(src);
		recover = 1;
	}
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
	swizzle_swap(dst);
	if(recover)
		swizzle_swap(src);
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
	uint32 u, i;
	int left, right , window_size;
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
		for (ii = 0; ii < dw_or_dh; ii++)								\
		{																\
			left  = contrib->row[ii].left;								\
			right = contrib->row[ii].right;								\
			r = 0,g = 0,b = 0,a = 0;									\
			for (i = left; i <= right; i++)								\
			{															\
																		\
				if(dtype == DISPLAY_PIXEL_FORMAT_8888){					\
					GET_RGBA_8888(src32[ _src_n ],sr,sg,sb,sa);			\
				}														\
				else if(dtype == DISPLAY_PIXEL_FORMAT_4444){			\
					GET_RGBA_4444(src16[ _src_n ],sr,sg,sb,sa);			\
				}														\
				else if(dtype == DISPLAY_PIXEL_FORMAT_5551){			\
					GET_RGBA_5551(src16[ _src_n ],sr,sg,sb,sa);			\
				}														\
				else if(dtype == DISPLAY_PIXEL_FORMAT_565){				\
					GET_RGBA_565(src16[ _src_n ],sr,sg,sb,sa);			\
				}														\
				if(sa==0)												\
					continue;											\
				r += (uint8)(contrib->row[ii].weights[i-left] * (double)(sr)); \
				g += (uint8)(contrib->row[ii].weights[i-left] * (double)(sg)); \
				b += (uint8)(contrib->row[ii].weights[i-left] * (double)(sb)); \
				a += (uint8)(contrib->row[ii].weights[i-left] * (double)(sa)); \
			}															\
			if(dtype == DISPLAY_PIXEL_FORMAT_8888)						\
				dst32[ _dst_n ] = MAKE_RGBA_8888(r,g,b,a);			\
			else if(dtype == DISPLAY_PIXEL_FORMAT_4444)					\
				dst16[ _dst_n ] = MAKE_RGBA_4444(r,g,b,a);			\
			else if(dtype == DISPLAY_PIXEL_FORMAT_5551)					\
				dst16[ _dst_n ] = MAKE_RGBA_5551(r,g,b,a);			\
			else if(dtype == DISPLAY_PIXEL_FORMAT_565)					\
				dst16[ _dst_n ] = MAKE_RGBA_565(r,g,b,a);				\
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
	uint8 recover = 0;
	uint32 spitch, dpitch;
	void* filter = filter_create_funs[mode]();
	dst = image_create(w,h,src->dtype);
	dst->dontswizzle = 1;
	if(src->swizzle ==1){
		unswizzle_swap(src);
		recover = 1;
	}
	temp = malloc(dst->w*src->h*src->bpb);
	memset(temp,0,dst->w*src->h*src->bpb);
	spitch = src->texw;
	dpitch = dst->texw;
	horiz_scale(filter, mode, src->data,src->w,src->h,temp,dst->w,src->h,spitch,src->dtype);
	vert_scale (filter, mode, temp,dst->w,src->h,dst->data,dst->w,dst->h,dpitch,src->dtype);
	free(temp);
	filter_destroy(filter);
	if(recover)
		swizzle_swap(src);
	return dst;
}
