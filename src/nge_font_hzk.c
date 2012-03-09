#include "nge_debug_log.h"
#include "nge_font.h"
#include "nge_font_internal.h"
#include "nge_io_file.h"
#include "stdlib.h"
#include "string.h"
#include "nge_charsets.h"
#include "nge_font_bittype.h"

typedef struct {
	void*		procs;	/* font-specific rendering routines*/
	int			size;	/* font height in pixels*/
	int			rotation;	/* font rotation*/
	uint32		disp;	/* diplaymode*/
	int         flags;
	workbuf     encodingBuf;

	uint32      color_fg;
	uint32      color_bg;
	uint32      color_sh;
	uint8*      cfont_raw;
	uint8* 		afont_raw;
	workbuf		bitbuf;

//hzk special
	int 		afont_width;
	int 		cfont_width;
	int 		font_height;
}FontHzk,*PFontHzk;

//inner use
static BOOL hzk_getfontinfo(PFont pfont, PFontInfo pfontinfo);
static void hzk_gettextsize(PFont pfont, const void *text, int cc,int flags, int *pwidth, int *pheight,int *pbase);
static void hzk_destroyfont(PFont pfont);
static void hzk_drawtext(PFont pfont, image_p pimage, int x, int y,const void *str, int count, int flags);
static void hzk_drawtext_shadow(PFont pfont, image_p pimage, int x, int y,const void *str, int count, int flags);

/* handling routines for HZKFONT*/
static  FontProcs hzk_procs = {
		hzk_getfontinfo,
		hzk_gettextsize,
		NULL,				/* hzk_gettextbits */
		hzk_destroyfont,
		hzk_drawtext,
		hzk_drawtext_shadow,
		bit_setfontcolor,
		NULL,				/* setfontsize*/
		NULL, 				/* setfontrotation*/
		NULL,				/* setfontattr*/
		NULL,				/* duplicate not yet implemented */
		bit_setflags,
		bit_setcolorex
};

static int use_big5 = 0;

#define MAKE_EXPANDCCHAR_FUNC(bits)										\
	static inline void expandcchar_##bits (PFontHzk pf, int bg, int fg, unsigned char* c, uint##bits * bitmap) \
	{																	\
		int x,y, i=0, b = 0;											\
		int c1, c2, seq;												\
		unsigned char *font;											\
																		\
		c1 = c[0];														\
		c2 = c[1];														\
		if (use_big5)													\
		{																\
			seq=0;														\
			/* ladd=loby-(if(loby<127)?64:98)*/							\
			c2-=(c2<127?64:98);											\
																		\
			/* hadd=(hiby-164)*157*/									\
			if (c1>=0xa4)	/* standard font*/							\
			{															\
				seq=(((c1-164)*157)+c2);								\
				if (seq>=5809) seq-=408;								\
			}															\
																		\
			/* hadd=(hiby-161)*157*/									\
			if (c1<=0xa3)	/* special font*/							\
				seq=(((c1-161)*157)+c2)+13094;							\
		}																\
		else{															\
			if(pf->flags == FONT_TYPE_HZK)								\
				seq=((c1 - 161)*94 + c2 - 161);							\
			else														\
				seq = ( 0xbf*(c1-0x81) + (c2-0x40) ) ;					\
		}																\
		font = pf->cfont_raw + ((seq) *									\
								(pf->font_height * ((pf->cfont_width + 7) / 8))); \
																		\
		for (y = 0; y < pf->font_height; y++){							\
			FOR_LOOP_SET_FONT_COLOR(bits, x, b, cfont_width);			\
		}																\
	}


#define MAKE_EXPANDCHAR_FUNC(bits)										\
	static inline void expandchar_##bits(PFontHzk pf, int bg, int fg, int c, uint##bits* bitmap) \
	{																	\
		int x,y, i=0, b = 0;											\
		unsigned char *font;											\
		font = pf->afont_raw + c * (pf->font_height * ((pf->afont_width + 7) / 8)); \
		for (y = 0; y < pf->font_height; y++){							\
			FOR_LOOP_SET_FONT_COLOR(bits, x, b, afont_width);			\
			i+= pf->afont_width;										\
		}																\
	}

MAKE_EXPANDCCHAR_FUNC(16)
MAKE_EXPANDCCHAR_FUNC(32)

MAKE_EXPANDCHAR_FUNC(16)
MAKE_EXPANDCHAR_FUNC(32)

MAKE_COPY_RAWDATA_IMAGE_FUNC(16, h)
MAKE_COPY_RAWDATA_IMAGE_FUNC(32, h)

MAKE_PROCESS_SHADOW_FUNC(16, h)
MAKE_PROCESS_SHADOW_FUNC(32, h)

#define MAKE_DRAWTEXT_FUNC(bits)										\
	void hzk_drawtext_##bits(PFontHzk pf, image_p pimage, int ax, int ay,const void *text, int cc, int flags) \
	{																	\
		unsigned char c[2];												\
		uint##bits* bitmap = NULL;										\
		unsigned char s1[3];											\
		char *s,*sbegin;												\
		int size;														\
																		\
		s=(char *)text;													\
		if(cc==1)														\
		{																\
			s1[0]=*((unsigned char*)text);								\
			s1[1]=0x0;													\
			s1[2]=0x0;													\
			s=(char*)s1;												\
		}																\
																		\
		sbegin=s;														\
		size = pf->cfont_width * pf->font_height *sizeof(uint##bits);	\
		EXPAND_WORKBUF(pf, size, bitbuf);								\
		bitmap = (uint##bits*)pf->bitbuf.data;							\
		memset(bitmap,0,size);											\
																		\
		while( getnextchar(s, c) )										\
		{																\
			if( c[1] != '\0'){											\
				expandcchar_##bits(pf, pf->color_bg,pf->color_fg,c, bitmap); \
				copy_rawdata_image_custom_##bits(bitmap,pimage,ax,ay,pf->cfont_width,pf->font_height,(uint##bits)pf->color_bg,(uint##bits)pf->color_fg); \
				s += 2;													\
				ax += pf->cfont_width;									\
			}															\
			else{														\
				expandchar_##bits(pf, pf->color_bg,pf->color_fg,c[0], bitmap); \
				copy_rawdata_image_custom_##bits(bitmap,pimage,ax,ay,pf->afont_width,pf->font_height,(uint##bits)pf->color_bg,(uint##bits)pf->color_fg); \
				s += 1;													\
				ax += pf->afont_width;									\
			}															\
																		\
			if(s>=sbegin+cc) break;										\
		}																\
	}

#define MAKE_DRAWTEXT_SHADOW_FUNC(bits)									\
	void hzk_drawtext_shadow_##bits(PFontHzk pf, image_p pimage, int ax, int ay,const void *text, int cc, int flags) \
	{																	\
		unsigned char c[2];												\
		uint##bits* bitmap = NULL;										\
		unsigned char s1[3];											\
		int size;														\
		char *s,*sbegin;												\
																		\
		s=(char *)text;													\
		if(cc==1)														\
		{																\
			s1[0]=*((unsigned char*)text);								\
			s1[1]=0x0;													\
			s1[2]=0x0;													\
			s=(char*)s1;												\
		}																\
																		\
		sbegin=s;														\
		size = pf->cfont_width * pf->font_height *sizeof(uint##bits);	\
		EXPAND_WORKBUF(pf, size, bitbuf);								\
		bitmap = (uint##bits*)pf->bitbuf.data;							\
		memset(bitmap,0,size);											\
																		\
		while( getnextchar(s, c) )										\
		{																\
			if( c[1] != '\0'){											\
				expandcchar_##bits(pf, pf->color_bg,pf->color_fg,c, bitmap); \
				copy_rawdata_image_custom_##bits(bitmap,pimage,ax,ay,pf->cfont_width,pf->font_height,(uint##bits)pf->color_bg,(uint##bits)pf->color_fg); \
				process_shadow_##bits(bitmap,pimage,ax,ay,pf->cfont_width,pf->font_height,(uint##bits)pf->color_bg,(uint##bits)pf->color_fg,(uint##bits)pf->color_sh); \
				s += 2;													\
				ax += pf->cfont_width;									\
			}															\
			else{														\
				expandchar_##bits(pf, pf->color_bg,pf->color_fg,c[0], bitmap); \
				copy_rawdata_image_custom_##bits(bitmap,pimage,ax,ay,pf->afont_width,pf->font_height,(uint##bits)pf->color_bg,(uint##bits)pf->color_fg); \
				process_shadow_##bits(bitmap,pimage,ax,ay,pf->cfont_width,pf->font_height,(uint##bits)pf->color_bg,(uint##bits)pf->color_fg,(uint##bits)pf->color_sh); \
				s += 1;													\
				ax += pf->afont_width;									\
			}															\
																		\
			if(s>=sbegin+cc) break;										\
		}																\
	}

PFont create_font_hzk(const char* cname,const char* aname, int height,int disp)
{
	int handle  = 0;
	int asize = 0 ;
	int csize = 0;
	char* afbuf= 0;
	char* cfbuf = 0;
	PFont pf;
	GET_BUF_DATA_FROM_FILE(handle, cname, csize, cfbuf);
	GET_BUF_DATA_FROM_FILE(handle, aname, asize, afbuf);
	pf = create_font_hzk_buf(cfbuf,csize,afbuf,asize,height,disp);
	SAFE_FREE(cfbuf);
	SAFE_FREE(afbuf);
	return pf;
}

PFont create_font_hzk_buf(const char *cfbuf,int csize,const char* afbuf,int asize, int height,int disp)
{
	PFontHzk pf;

	if (asize <= 0 || csize <=0)
		return NULL;

	pf = (PFontHzk)malloc(sizeof(FontHzk));
	if (!pf)
		return NULL;
	memset(pf,0,sizeof(FontHzk));
	pf->procs = &hzk_procs;
	pf->size = height;
	pf->rotation = 0;
	pf->flags = FONT_TYPE_GBK;

	INIT_BITBUF(pf);
	INIT_DISP(pf, disp);

	pf->afont_width = height/2;
	pf->cfont_width = height;
	pf->font_height = height;

	/* Load the font library to the system memory.*/
	pf->cfont_raw = (uint8*)malloc(csize);
	memcpy(pf->cfont_raw,cfbuf,csize);

	pf->afont_raw = (uint8*)malloc(asize);
	memcpy(pf->afont_raw,afbuf,asize);

	return (PFont)pf;
}

BOOL hzk_getfontinfo(PFont pfont, PFontInfo pfontinfo)
{
	PFontHzk pf = (PFontHzk)pfont;
	int i;
	pfontinfo->height = pf->font_height;
	pfontinfo->maxwidth = pf->cfont_width;
	pfontinfo->baseline = pf->font_height - 2;
	/* FIXME: calculate these properly: */
	pfontinfo->linespacing = pfontinfo->height;
	pfontinfo->descent = pfontinfo->height - pfontinfo->baseline;
	pfontinfo->maxascent = pfontinfo->baseline;
	pfontinfo->maxdescent = pfontinfo->descent;

	pfontinfo->firstchar = 0;
	pfontinfo->lastchar = 0;
	pfontinfo->fixed = 1;

	for(i=0; i<=256; i++)
		pfontinfo->widths[i] = pf->afont_width;

	return 1;
}

MAKE_BIT_GETTEXTSIZE(hzk, PFontHzk, afont_width, cfont_width, font_height)

void hzk_destroyfont(PFont pfont)
{
	PFontHzk pf = (PFontHzk)pfont;
	SAFE_FREE(pf->encodingBuf.data);
	SAFE_FREE(pf->bitbuf.data);
	SAFE_FREE(pf->afont_raw);
	SAFE_FREE(pf->cfont_raw);
	SAFE_FREE(pf);
}

MAKE_DRAWTEXT_FUNC(16)
MAKE_DRAWTEXT_FUNC(32)

void hzk_drawtext(PFont pfont, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
{
	PFontHzk pf = (PFontHzk)pfont;

	text = (void*)_nge_ft_conv_encoding(pf, text, &cc);
	if (cc <= 0)
		return;

	if(pimage->swizzle ==1){
		unswizzle_swap(pimage);
		pimage->dontswizzle = 1;
	}
	pimage->modified =1;
	if(pimage->dtype==pf->disp){
		if(pf->disp==DISPLAY_PIXEL_FORMAT_8888){
			hzk_drawtext_32(pf,pimage,ax,ay,text,cc, flags);
		}
		else{
			hzk_drawtext_16(pf,pimage,ax,ay,text,cc, flags);
		}
	}
}

MAKE_DRAWTEXT_SHADOW_FUNC(16)
MAKE_DRAWTEXT_SHADOW_FUNC(32)

void hzk_drawtext_shadow(PFont pfont, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
{
	PFontHzk pf = (PFontHzk)pfont;

	text = (void*)_nge_ft_conv_encoding(pf, text, &cc);
	if (cc <= 0)
		return;

	if(pimage->dtype==pf->disp){
		if(pf->disp==DISPLAY_PIXEL_FORMAT_8888){
			hzk_drawtext_shadow_32(pf,pimage,ax,ay,text,cc, flags);
		}
		else{
			hzk_drawtext_shadow_16(pf,pimage,ax,ay,text,cc, flags);
		}
	}
}
