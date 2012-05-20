#include "nge_debug_log.h"
#include "nge_font.h"
#include "nge_font_internal.h"
#include "nge_io_file.h"
#include "stdlib.h"
#include "string.h"
#include "nge_charsets.h"
#include "nge_font_bittype.h"

enum{
	HFONT_TYPE_ASCII,
	HFONT_TYPE_GBK,
	HFONT_TYPE_UNICODE,
	HFONT_TYPE_OTHER
};

//16 bytes header
typedef struct  {
	char magic[4];    //"NDOT"
	uint8 hdrlen;     //current version header is 16
	uint8 version;    //current version 1
	uint8 size;       //dot font size,eg 12,14,16...
	uint8 type;       //ASCII,GBK,UNICODE,OTHER
	uint8 alignsize;  //memalign size.
	uint16 gbkoffset; //gbkoffset
	char  reserved[5];//reserved
}NfontHeader;

typedef struct {
	void*	    procs;	/* font-specific rendering routines*/
	int		    size;	/* font height in pixels*/
	int		    rotation;	/* font rotation*/
	uint32		disp;	/* diplaymode*/
	int         flags;
	workbuf     encodingBuf;

	/* bit font spec */
	uint32      color_fg;
	uint32      color_bg;
	uint32      color_sh;
	char*       cfont_raw;
	char*       afont_raw;
	workbuf     bitbuf;

	/* NGE font spec */
	char*       p_rawbuffer;
	int 		alignsize;
	char        type;
	int         font_width;
	int         font_top;
	int         font_height;
	int         font_left;
}FontNfont,*PFontNfont;

//inner use
static BOOL nfont_getfontinfo(PFont pfont, PFontInfo pfontinfo);
static void nfont_gettextsize(PFont pfont, const void *text, int cc,int flags, int *pwidth, int *pheight,int *pbase);
static void nfont_destroyfont(PFont pfont);
static void nfont_drawtext(PFont pfont, image_p pimage, int x, int y,const void *str, int count, int flags);
static void nfont_drawtext_shadow(PFont pfont, image_p pimage, int x, int y,const void *str, int count, int flags);

/* handling routines for nFONT*/
static  FontProcs nfont_procs = {
		nfont_getfontinfo,
		nfont_gettextsize,
		NULL,				/* nfont_gettextbits */
		nfont_destroyfont,
		nfont_drawtext,
		nfont_drawtext_shadow,
		bit_setfontcolor,
		NULL,				/* setfontsize*/
		NULL, 				/* setfontrotation*/
		NULL,				/* setfontattr*/
		NULL,				/* duplicate not yet implemented */
		bit_setflags,
		bit_setcolorex
};

#define POP_PROPS_FROM_FONT(pf, font)			\
	pf->font_width  =  font[0];					\
	pf->font_height =  font[1];					\
	pf->font_left   =  (char)font[2];			\
	pf->font_top    =  (char)font[3];			\
	font += 4

#define MAKE_EXPANDCCHAR_FUNC(bits)										\
	void expandcchar_##bits(PFontNfont pf, int bg, int fg, unsigned char* c, uint##bits* bitmap) \
	{																	\
		int c1, c2, seq;												\
		int x,y, b = 0, i = 0;											\
		unsigned char *font;											\
																		\
		c1 = c[0];														\
		c2 = c[1];														\
		seq = ( 0xbf*(c1-0x81) + (c2-0x40) ) ;							\
																		\
		font = (unsigned char*)(pf->cfont_raw + seq*(pf->alignsize+4));	\
		POP_PROPS_FROM_FONT(pf, font);									\
																		\
		for (y = 0; y < pf->size; y++){									\
			FOR_LOOP_SET_FONT_COLOR(bits, x, b, font_width);			\
		}																\
	}

#define MAKE_EXPANDCHAR_FUNC(bits)										\
	void expandchar_##bits(PFontNfont pf, int bg, int fg, int c, uint##bits* bitmap) \
	{																	\
		int i=0,b = 0;													\
		int x,y;														\
		unsigned char *font;											\
																		\
		font = (unsigned char*)(pf->afont_raw + c * (pf->alignsize+4));	\
		POP_PROPS_FROM_FONT(pf, font);									\
																		\
		for (y = 0; y < pf->size; y++){									\
			FOR_LOOP_SET_FONT_COLOR(bits, x, b, font_width);			\
		}																\
	}

MAKE_EXPANDCCHAR_FUNC(16)
MAKE_EXPANDCCHAR_FUNC(32)

MAKE_EXPANDCHAR_FUNC(16)
MAKE_EXPANDCHAR_FUNC(32)

MAKE_COPY_RAWDATA_IMAGE_FUNC(16, w)
MAKE_COPY_RAWDATA_IMAGE_FUNC(32, w)

MAKE_PROCESS_SHADOW_FUNC(16, w)
MAKE_PROCESS_SHADOW_FUNC(32, w)

#define MAKE_DRAWTEXT_FUNC(bits)										\
	void nfont_drawtext_##bits(PFontNfont pf, image_p pimage, int ax, int ay,const void *text, int cc, int flags) \
	{																	\
		unsigned char c[2];												\
		uint##bits* bitmap = NULL;										\
		unsigned char s1[3];											\
		char *s,*sbegin;												\
		int size;														\
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
																		\
		size = 2*pf->font_width*pf->size*sizeof(uint##bits);			\
		if( size > pf->bitbuf.datalen){									\
			pf->bitbuf.datalen = size * 2;								\
			free(pf->bitbuf.data);										\
			pf->bitbuf.data = (char*)malloc(pf->bitbuf.datalen);		\
		}																\
		bitmap = (uint##bits*)pf->bitbuf.data;							\
		memset(bitmap,0,size);											\
																		\
		while( getnextchar(s, c) )										\
		{																\
			if( c[1] != '\0'){											\
				if(pf->type == HFONT_TYPE_GBK){							\
					expandcchar_##bits(pf, pf->color_bg,pf->color_fg,c, bitmap); \
					copy_rawdata_image_custom_##bits(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,pf->color_bg,pf->color_fg); \
					ax += pf->font_width+1;								\
				}														\
				else													\
					ax += pf->size+1;									\
				s += 2;													\
			}															\
			else{														\
				expandchar_##bits(pf, pf->color_bg,pf->color_fg,c[0], bitmap); \
				copy_rawdata_image_custom_##bits(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,pf->color_bg,pf->color_fg); \
				s += 1;													\
				ax += pf->font_width+1;									\
				/*printf("pf->font_left=%d\n",pf->font_left);*/			\
			}															\
																		\
			if(s>=sbegin+cc) break;										\
		}																\
																		\
		SAFE_FREE(bitmap);												\
	}

#define MAKE_DRAWTEXT_SHADOW_FUNC(bits)									\
	void nfont_drawtext_shadow_##bits(PFontNfont pf, image_p pimage, int ax, int ay,const void *text, int cc, int flags) \
	{																	\
		unsigned char c[2];												\
		uint##bits* bitmap = NULL;										\
		unsigned char s1[3];											\
		int size;														\
		char *s,*sbegin;												\
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
		size = 2*pf->font_width*pf->size*sizeof(uint##bits);			\
		if( size > pf->bitbuf.datalen){									\
			pf->bitbuf.datalen = size * 2;								\
			free(pf->bitbuf.data);										\
			pf->bitbuf.data = (char*)malloc(pf->bitbuf.datalen);		\
		}																\
		bitmap = (uint##bits*)pf->bitbuf.data;							\
		memset(bitmap,0,size);											\
																		\
		while( getnextchar(s, c) )										\
		{																\
			if( c[1] != '\0'){											\
				if(pf->type == HFONT_TYPE_GBK){							\
					expandcchar_##bits(pf, pf->color_bg,pf->color_fg,c, bitmap); \
					copy_rawdata_image_custom_##bits(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,pf->color_bg,pf->color_fg); \
					process_shadow_##bits(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,pf->color_bg,pf->color_fg,pf->color_sh); \
					ax += pf->font_width+1;								\
				}														\
				else													\
					ax += pf->size+1;									\
				s += 2;													\
			}															\
			else{														\
				expandchar_##bits(pf, pf->color_bg,pf->color_fg,c[0], bitmap); \
				copy_rawdata_image_custom_##bits(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,pf->color_bg,pf->color_fg); \
				process_shadow_##bits(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,pf->color_bg,pf->color_fg,pf->color_sh); \
				s += 1;													\
				ax += pf->font_width+1;									\
			}															\
																		\
			if(s>=sbegin+cc) break;										\
		}																\
		SAFE_FREE(bitmap);												\
	}

PFont create_font_nfont(const char* name,int disp)
{
	int handle  = 0;
	int nsize = 0 ;
	char* nfbuf= 0;
	PFont pf = NULL;

	GET_BUF_DATA_FROM_FILE(handle, name, nsize, nfbuf);

	pf = create_font_nfont_buf(nfbuf,nsize,disp);
	SAFE_FREE(nfbuf);

	return pf;
}

PFont create_font_nfont_buf(const char *nfbuf,int nsize,int disp)
{
	PFontNfont pf = NULL;
	NfontHeader* pheader = (NfontHeader*)nfbuf;

	if(nfbuf == NULL ||nsize < 16)
		return NULL;
	if(strncmp(pheader->magic,"NDOT",4)!=0){
		return NULL;
	}

	pf = (PFontNfont)malloc(sizeof(FontNfont));
	if (!pf)
		return NULL;
	memset(pf,0,sizeof(FontNfont));

	pf->type = pheader->type;
	pf->procs = &nfont_procs;
	pf->flags = FONT_TYPE_GBK;
	pf->size =  pheader->size;
	pf->alignsize = pheader->alignsize;

	pf->p_rawbuffer = (char*)malloc(nsize);
	memcpy(pf->p_rawbuffer,nfbuf,nsize);

	pf->afont_raw = pf->p_rawbuffer+pheader->hdrlen;
	if(pheader->type == HFONT_TYPE_ASCII)
		pf->cfont_raw = NULL;
	else
		pf->cfont_raw = pf->p_rawbuffer+pheader->gbkoffset;

	INIT_BITBUF(pf);

	INIT_DISP(pf, disp);

	return (PFont)pf;
}

BOOL nfont_getfontinfo(PFont pfont, PFontInfo pfontinfo)
{
	PFontNfont pf = (PFontNfont)pfont;

	pfontinfo->height = pf->size;
	pfontinfo->maxwidth = pf->alignsize/2;
	pfontinfo->baseline = pf->size - 2;
	/* FIXME: calculate these properly: */
	pfontinfo->linespacing = pfontinfo->height;
	pfontinfo->descent = pfontinfo->height - pfontinfo->baseline;
	pfontinfo->maxascent = pfontinfo->baseline;
	pfontinfo->maxdescent = pfontinfo->descent;

	pfontinfo->firstchar = 0;
	pfontinfo->lastchar = 0;
	pfontinfo->fixed = 1;

	return 1;
}

MAKE_BIT_GETTEXTSIZE(nfont, PFontNfont, size/2, size, size)

void nfont_destroyfont(PFont pfont)
{
	PFontNfont pf = (PFontNfont)pfont;
	SAFE_FREE(pf->encodingBuf.data);
	SAFE_FREE(pf->bitbuf.data);
	SAFE_FREE(pf->p_rawbuffer);
	SAFE_FREE(pf);
}

MAKE_DRAWTEXT_FUNC(16)
MAKE_DRAWTEXT_FUNC(32)

void nfont_drawtext(PFont pfont, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
{
	PFontNfont pf = (PFontNfont)pfont;
	text = (void*)_nge_ft_conv_encoding(pfont, text, &cc);
	if (cc <= 0)
		return;

	if(pimage->swizzle ==1){
		unswizzle_swap(pimage);
		pimage->dontswizzle = 1;
	}
	pimage->modified =1;
	if(pimage->dtype==pf->disp){
		if(pf->disp==DISPLAY_PIXEL_FORMAT_8888){
			nfont_drawtext_32(pf,pimage,ax,ay,text,cc, flags);
		}
		else{
			nfont_drawtext_16(pf,pimage,ax,ay,text,cc, flags);
		}
	}

}

MAKE_DRAWTEXT_SHADOW_FUNC(16)
MAKE_DRAWTEXT_SHADOW_FUNC(32)

void nfont_drawtext_shadow(PFont pfont, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
{
	PFontNfont pf = (PFontNfont)pfont;
	text = (void*)_nge_ft_conv_encoding(pfont, text, &cc);
	if (cc <= 0)
		return;

	if(pimage->dtype==pf->disp){
		if(pf->disp==DISPLAY_PIXEL_FORMAT_8888){
			nfont_drawtext_shadow_32(pf,pimage,ax,ay,text,cc, flags);
		}
		else{
			nfont_drawtext_shadow_16(pf,pimage,ax,ay,text,cc, flags);
		}
	}
}
