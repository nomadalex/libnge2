/**
 * @file  nge_font_bittype.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/07 02:56:58
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _NGE_FONT_BITTYPE_H
#define _NGE_FONT_BITTYPE_H

typedef struct {
	void*		procs;	/* font-specific rendering routines*/
	int			size;	/* font height in pixels*/
	int			rotation;	/* font rotation*/
	uint32_t		disp;	/* diplaymode*/
	int         flags;
	workbuf     encodingBuf;

/* bit spec */
	uint32_t      color_fg;
	uint32_t      color_bg;
	uint32_t      color_sh;
	uint8_t*      cfont_raw;
	uint8_t* 		afont_raw;
	workbuf		bitbuf;
} FontBit,*PFontBit;

/* utils */
inline static int IsBig5(int i)
{
	if ((i>=0xa140 && i<=0xa3bf) || /* a140-a3bf(!a3e0) */
		(i>=0xa440 && i<=0xc67e) || /* a440-c67e        */
		(i>=0xc6a1 && i<=0xc8d3) || /* c6a1-c8d3(!c8fe) */
		(i>=0xc940 && i<=0xf9fe))   /* c940-f9fe        */
		return 1;
	else
		return 0;
}

/* get next gbk character */
inline static int getnextchar(char* s, unsigned char* cc)
{
	if( s[0] == '\0') return 0;

	cc[0] = (unsigned char)(*s);

	if( ((unsigned char)cc[0]<0x80) ) { /* ascii */
		cc[1] = '\0';
		return 1;
	}

	cc[1] = (unsigned char)(*(s + 1));
	return 1;
}
/* utils end */

#define EXPAND_WORKBUF(pf, size, buf)					\
	if( size > pf->buf.datalen){						\
		pf->buf.datalen = size * 2;						\
		free(pf->buf.data);								\
		pf->buf.data = (char*)malloc(pf->buf.datalen);	\
	}

inline static uint8_t* _nge_ft_conv_encoding(PFont pf, const void *text, int * pCC) {
	uint8_t *value;

	if (nge_font_encoding == NGE_ENCODING_UTF_8) {
		int len = *pCC;

		EXPAND_WORKBUF(pf, len, encodingBuf);
		value = (uint8_t*)pf->encodingBuf.data;

		*pCC = nge_charsets_utf8_to_gbk((const uint8_t*)text, value, len, pf->encodingBuf.datalen);

		return value;
	}
	else
		return (uint8_t*)text;
}

#define INIT_BITBUF(pf)										\
	pf->bitbuf.datalen = 2048;								\
	pf->bitbuf.data = (char*)malloc(pf->bitbuf.datalen);	\
	memset(pf->bitbuf.data,0,pf->bitbuf.datalen)

#define INIT_DISP(pf, disp)						\
	switch(disp)								\
	{											\
	case DISPLAY_PIXEL_FORMAT_4444:				\
		pf->disp = disp;						\
		pf->color_bg = FONT_BG_4444;			\
		pf->color_fg = FONT_FG_4444;			\
		pf->color_sh = FONT_SH_4444;			\
		break;									\
	case DISPLAY_PIXEL_FORMAT_565:				\
		pf->disp = disp;						\
		pf->color_bg = FONT_BG_565;				\
		pf->color_fg = FONT_FG_565;				\
		pf->color_sh = FONT_SH_565;				\
		break;									\
	case DISPLAY_PIXEL_FORMAT_5551:				\
		pf->disp = disp;						\
		pf->color_bg = FONT_BG_5551;			\
		pf->color_fg = FONT_FG_5551;			\
		pf->color_sh = FONT_SH_5551;			\
		break;									\
	case DISPLAY_PIXEL_FORMAT_8888:				\
		pf->disp = disp;						\
		pf->color_bg = FONT_BG_8888;			\
		pf->color_fg = FONT_FG_8888;			\
		pf->color_sh = FONT_SH_8888;			\
		break;									\
	default:									\
		pf->disp = DISPLAY_PIXEL_FORMAT_5551;	\
		pf->color_bg = FONT_BG_5551;			\
		pf->color_fg = FONT_FG_5551;			\
		pf->color_sh = FONT_SH_5551;			\
		break;									\
	}

#define GET_BUF_DATA_FROM_FILE(handle, fname, size, buf)	\
	handle	= io_fopen(fname,IO_RDONLY);					\
	if(handle != 0){										\
		size = io_fsize(handle);							\
		buf = (char*)malloc(size);							\
		io_fread(buf,size,1,handle);						\
		io_fclose(handle);									\
	}														\
	else													\
		return NULL

#define MAKE_BIT_GETTEXTSIZE(prefix, type, afont_width, cfont_width, font_height) \
	void prefix##_gettextsize(PFont pfont, const void *text, int cc,int flags, int *pwidth, int *pheight,int *pbase) \
	{																	\
		type pf = (type)pfont;											\
		unsigned char c[2];												\
		char *s,*sbegin;												\
		unsigned char s1[3];											\
		int ax=0;														\
		uint8_t* value;													\
																		\
		value = _nge_ft_conv_encoding(pfont, text, &cc);				\
		if(cc > 0)														\
		{																\
			s = (char*)value;											\
																		\
			if(cc==1)													\
			{															\
				s1[0]=*((unsigned char*)value);							\
				s1[1]=0x0;												\
				s1[2]=0x0;												\
				s=(char*)s1;											\
			}															\
			sbegin=s;													\
			while(getnextchar(s, c))									\
			{															\
				if( c[1] == '\0') /* ascii */							\
				{														\
					s += 1;												\
					ax += pf->afont_width;								\
				}														\
				else													\
				{														\
					s += 2;												\
					ax += pf->cfont_width;								\
				}														\
																		\
				if(s>=sbegin+cc) {										\
					break;												\
				}														\
			}															\
		}																\
																		\
		if(pwidth) *pwidth = ax;										\
		if(pheight) *pheight = pf->font_height;							\
		if(pbase) *pbase = pf->font_height-2;							\
	}

static uint32_t bit_setfontcolor(PFont pfont, uint32_t color)
{
	PFontBit pf = (PFontBit)pfont;
	uint32_t last_color = pf->color_fg;
	pf->color_fg = color;
	return last_color;
}

static void bit_setflags(PFont pfont,int flags)
{
	PFontBit pf = (PFontBit)pfont;
	pf->flags = flags;
}

static void bit_setcolorex(PFont pfont, uint32_t color_fg,uint32_t color_bg ,uint32_t color_sh )
{
	PFontBit pf = (PFontBit)pfont;
	pf->color_fg = color_fg;
	pf->color_bg = color_bg;
	pf->color_sh = color_sh;
}

#define FOR_LOOP_SET_FONT_COLOR(bits, x, b, font_width)	\
	for (x = 0; x < pf->font_width; x++)				\
	{													\
		if (x % 8 == 0)									\
			b = *font++;								\
														\
		if (b & (128 >> (x % 8)))						\
			bitmap[i++]=(uint##bits##_t)fg;					\
		else											\
			bitmap[i++]=(uint##bits##_t)bg;					\
	}

#define MAKE_COPY_RAWDATA_IMAGE_FUNC(bits, var)							\
	inline static void copy_rawdata_image_custom_##bits(void* data,const image_p des,int x,int y,uint32_t w,uint32_t h,uint##bits##_t color_bg,uint##bits##_t color_fg) \
	{																	\
		uint##bits##_t *bmp, *cpbegin;										\
		uint32_t i,j;														\
																		\
		cpbegin = (uint##bits##_t*)des->data+y*des->texw+x;					\
		bmp = (uint##bits##_t*)data;										\
		for(i =0;i<h;i++){												\
			for(j =0;j<w;j++){											\
				if(j+x>des->texw||i+y>des->texh)						\
					continue;											\
				cpbegin[j] = bmp[i*var+j];								\
			}															\
			cpbegin += des->texw;										\
		}																\
	}

#define MAKE_PROCESS_SHADOW_FUNC(bits, var)								\
	inline static void process_shadow_##bits(void* data, const image_p des,int x,int y,uint32_t w,uint32_t h,uint##bits##_t color_bg,uint##bits##_t color_fg,uint##bits##_t color_sh) \
	{																	\
		uint##bits##_t *bmp, *cpbegin;										\
		uint32_t i,j;														\
		bmp = (uint##bits##_t*)data;										\
		cpbegin = (uint##bits##_t*)des->data+y*des->texw+x;					\
		for(i = 1;i<h+1;i++){											\
			for(j = 1;j<w+1;j++){										\
				if(j+x+1>des->texw||i+y+1>des->texh)					\
					continue;											\
				if(bmp[(i-1)*var+(j-1)]==color_fg&&cpbegin[j]!=color_fg) \
					cpbegin[j] = color_sh;								\
			}															\
			cpbegin += des->texw;										\
		}																\
	}

#endif /* _NGE_FONT_BITTYPE_H */
