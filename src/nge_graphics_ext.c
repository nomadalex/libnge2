#include "nge_graphics_ext.h"
#include "nge_image_load.h"

int get_gray_color(int dtype, int scol, int gray)
{
	int gcol, r, g, b, a;
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
	uint16 *p16,*psrc16;
	uint32 *p32,*psrc32;

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