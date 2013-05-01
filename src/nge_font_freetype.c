#include "nge_debug_log.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRIGONOMETRY_H
#include FT_GLYPH_H
#include FT_STROKER_H
#include FT_SYNTHESIS_H
#include "nge_font.h"
#include "nge_font_internal.h"
#include "nge_charsets.h"

typedef struct
{
	void* procs;	/* font-specific rendering routines*/
	int	size;	/* font height in pixels*/
	int	rotation;	/* font rotation*/
	uint32_t disp;	/* diplaymode*/
	int flags;
	workbuf	encodingBuf;

/* freetype stuff */
	//color
	uint8_t  r;
	uint8_t  g;
	uint8_t  b;
	uint8_t  a;
	int   fix_width;
	FT_Face face;
	FT_Matrix matrix;
	FT_Library library;
	
/* custom stuff */
	uint8_t  alpha_table[256];
}FontFreetype,*PFontFreetype;

static BOOL freetype2_getfontinfo(PFont pfont, PFontInfo pfontinfo);
static void freetype2_gettextsize(PFont pfont, const void *text, int cc,
								  int flags, int *pwidth, int *pheight,
								  int *pbase);
static void freetype2_destroyfont(PFont pfont);
static void freetype2_drawtext(PFont pfont, image_p pimage, int x, int y,
							   const void *text, int cc, int flags);
static void freetype2_setfontsize(PFont pfont, int fontsize);
static void freetype2_setfontrotation(PFont pfont, int rot);
static void freetype2_setfontattr(PFont pfont, int setflags, int clrflags);
static PFont freetype2_duplicate(PFont psrcfont, int fontsize);
static uint32_t  freetype2_setfontcolor(PFont pfont, uint32_t color);
static void freetype2_calcalphatable(PFont pfont);
static FontProcs freetype2_procs = {
	freetype2_getfontinfo,
	freetype2_gettextsize,
	NULL,			/* gettextbits */
	freetype2_destroyfont,
	freetype2_drawtext,
	NULL,
	freetype2_setfontcolor,
	freetype2_setfontsize,
	freetype2_setfontrotation,
	freetype2_setfontattr,
	freetype2_duplicate,
	NULL,
	NULL
};

PFont create_font_freetype(const char* fname, int height,int disp)
{
	PFontFreetype pf;
	//FT_Error error;

	/* allocate font structure */
	pf = (PFontFreetype) malloc(sizeof(FontFreetype));
	memset(pf,0,sizeof(FontFreetype));
	if (!pf) {
		return NULL;
	}
	FT_Init_FreeType( &pf->library );
	FT_New_Face( pf->library, fname, 0, &pf->face );
	if(pf->face == NULL){
		FT_Done_FreeType( pf->library );
		free(pf);
		return NULL;
	}

	FT_Set_Char_Size( pf->face,height<< 6, height << 6, 96, 96);
	pf->procs = &freetype2_procs;
	pf->size = height;
	pf->disp = disp==0?DISPLAY_PIXEL_FORMAT_5551:disp;
	pf->r = 0;
	pf->g = 0;
	pf->a = 0;

	pf->encodingBuf.datalen = 2048;
	pf->encodingBuf.data = (char*)malloc(pf->encodingBuf.datalen);
	memset(pf->encodingBuf.data,0,pf->encodingBuf.datalen);
	memset(pf->alpha_table,0,256);
	return (PFont)pf;
}

PFont create_font_freetype_buf(const char* buf,int bsize, int height,int disp)
{
	PFontFreetype pf;
	//FT_Error error;

	/* allocate font structure */
	pf = (PFontFreetype) malloc(sizeof(FontFreetype));
	memset(pf,0,sizeof(FontFreetype));
	if (!pf) {
		return NULL;
	}
	FT_Init_FreeType( &pf->library );
	FT_New_Memory_Face( pf->library, (const unsigned char *)buf,bsize, 0, &pf->face );
	FT_Set_Char_Size( pf->face,height<< 6, height << 6, 96, 96);
	pf->procs = &freetype2_procs;
	pf->size = height;
	pf->disp = disp==0?DISPLAY_PIXEL_FORMAT_5551:disp;
	pf->r = 0;
	pf->g = 0;
	pf->a = 0;

	pf->encodingBuf.datalen = 2048;
	pf->encodingBuf.data = (char*)malloc(pf->encodingBuf.datalen);
	memset(pf->encodingBuf.data,0,pf->encodingBuf.datalen);
	memset(pf->alpha_table,0,256);
	return (PFont)pf;
}

static BOOL freetype2_getfontinfo(PFont pfont, PFontInfo pfontinfo)
{
	return 1;
}

#define ROUND_26_6_TO_INT(valuetoround) (((valuetoround) + 63) >> 6)
static FT_Error freetype2_get_glyph_size(PFontFreetype pf,
						 FT_Face face,
						 int glyph_index,
						 int *padvance,
						 int *pascent,
						 int *pdescent)
{
	FT_Error error;
		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		if (error)
			return error;

		if (padvance)
			*padvance = ROUND_26_6_TO_INT(face->glyph->advance.x);
		if (pascent || pdescent)
		{
			FT_Glyph glyph;
			FT_BBox bbox;

			error = FT_Get_Glyph(face->glyph, &glyph);
			if (error)
				return error;

			FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_pixels, &bbox);

			FT_Done_Glyph(glyph);

			if (pascent)
				*pascent = bbox.yMax;
			if (pdescent)
				*pdescent = -bbox.yMin;
		}

		return 0;
}

inline static uint16_t* _nge_ft_conv_encoding(PFont pf, const void *text, int * pCC) {
	uint16_t *value;
	int len = *pCC;

	if( len > pf->encodingBuf.datalen){
		pf->encodingBuf.datalen = len*2;
		free(pf->encodingBuf.data);
		pf->encodingBuf.data = (char*)malloc(pf->encodingBuf.datalen);
	}
	value = (uint16_t*)pf->encodingBuf.data;

	if(nge_font_encoding == NGE_ENCODING_GBK){
		*pCC = nge_charset_gbk_to_ucs2((uint8_t*)text, value, len, pf->encodingBuf.datalen);
	}
	else if (nge_font_encoding == NGE_ENCODING_UTF_8) {
		*pCC = nge_charset_utf8_to_ucs2((uint8_t*)text, value, len, pf->encodingBuf.datalen);
	}
	return value;
}

static void freetype2_gettextsize(PFont pfont, const void *text, int cc,
								  int flags, int *pwidth, int *pheight,
								  int *pbase)
{
	FT_Face face;
	uint16_t* value;
	int char_index;
	int total_advance;
	int max_ascent;
	int max_descent;
	int advance;
	int ascent;
	int descent;
	FT_Error error;
	int cur_glyph_code;
	PFontFreetype pf = (PFontFreetype) pfont;

	value = _nge_ft_conv_encoding(pfont, text, &cc);
	if (cc <= 0)
		return;

	face = pf->face;
	/*
	* Starting point
	*/
	total_advance = 0;
	max_ascent  = 0;
	max_descent = 0;

	for (char_index = 0; char_index < cc; char_index++) {
		cur_glyph_code = FT_Get_Char_Index( pf->face, value[char_index] );//LOOKUP_CHAR(pf, face, str[char_index]);

		error = freetype2_get_glyph_size(pf, face, cur_glyph_code, &advance, &ascent, &descent);
		if (error)
			continue;

		total_advance += advance;
		if (max_ascent < ascent)
			max_ascent = ascent;
		if (max_descent < descent)
			max_descent = descent;
	}

	if(pwidth) *pwidth = total_advance;
	if(pheight) *pheight = max_ascent + max_descent;
	if(pbase) *pbase = max_ascent;
}

static void freetype2_destroyfont(PFont pfont)
{
	PFontFreetype pf = (PFontFreetype) pfont;
	FT_Done_Face(pf->face);
	FT_Done_FreeType(pf->library);
	SAFE_FREE(pf->encodingBuf.data);
	SAFE_FREE(pf);
}


static void draw_one_word(PFontFreetype pf,FT_Bitmap* bitmap,image_p pimage,int x,int y)
{
	int height = bitmap->rows;
	int width = bitmap->width;
	int i,j;
	uint32_t* cpbegin32;
	uint16_t* cpbegin16;
	unsigned char *buf = bitmap->buffer;
	if(y + height > (int)pimage->h)
		height = pimage->h - y;
	if(x + width > (int)pimage->w)
		width = pimage->w - x;

	if(height <= 0 || width <= 0)
		return;
		
	switch(pimage->dtype) {
		case DISPLAY_PIXEL_FORMAT_8888:
			cpbegin32 = (uint32_t*)pimage->data + y * pimage->texw + x;
			if(y < 0) {
				cpbegin32 -= y * pimage->texw;
				buf -= y * bitmap->width;
				j = -y;
			}
			else
				j = 0;
			for(; j < height; j++){
				if(x < 0) {
					cpbegin32 -= x;
					buf -= x;
					i = -x;
				}
				else
					i = 0;
				while(i < width) {
					for(; !(*buf) && i < width; i++, cpbegin32++, buf++) {
					}
					for(; (*buf) && i < width; i++, buf++, cpbegin32++)
						*cpbegin32 = MAKE_RGBA_8888(pf->r, pf->g, pf->b, pf->alpha_table[*buf]);
				}
				cpbegin32 += pimage->texw - width;
				buf += bitmap->width - width;
			}
			break;
		case DISPLAY_PIXEL_FORMAT_4444:
			cpbegin16 = (uint16_t*)pimage->data + y * pimage->texw + x;
			if(y < 0) {
				cpbegin16 -= y * pimage->texw;
				buf -= y * bitmap->width;
				j = -y;
			}
			else
				j = 0;
			for(; j < height; j++){
				if(x < 0) {
					cpbegin16 -= x;
					buf -= x;
					i = -x;
				}
				else
					i = 0;
				while(i < width) {
					for(; !(*buf) && i < width; i++, cpbegin16++, buf++) {
					}
					for(; (*buf) && i < width; i++, buf++, cpbegin16++)
						*cpbegin16 = MAKE_RGBA_4444(pf->r, pf->g, pf->b, pf->alpha_table[*buf]);
				}
				cpbegin16 += pimage->texw - width;
				buf += bitmap->width - width;
			}
			break;
		case DISPLAY_PIXEL_FORMAT_5551:
			cpbegin16 = (uint16_t*)pimage->data + y * pimage->texw + x;
			if(y < 0) {
				cpbegin16 -= y * pimage->texw;
				buf -= y * bitmap->width;
				j = -y;
			}
			else
				j = 0;
			for(; j < height; j++){
				if(x < 0) {
					cpbegin16 -= x;
					buf -= x;
					i = -x;
				}
				else
					i = 0;
				while(i < width) {
					for(; !(*buf) && i < width; i++, cpbegin16++, buf++) {
					}
					for(; (*buf) && i < width; i++, buf++, cpbegin16++)
						*cpbegin16 = MAKE_RGBA_4444(pf->r, pf->g, pf->b, (pf->a&(*(buf++)))?255:0);
				}
				cpbegin16 += pimage->texw - width;
				buf += bitmap->width - width;
			}
			break;
		case DISPLAY_PIXEL_FORMAT_565:
			cpbegin16 = (uint16_t*)pimage->data + y * pimage->texw + x;
			if(y < 0) {
				cpbegin16 -= y * pimage->texw;
				buf -= y * bitmap->width;
				j = -y;
			}
			else
				j = 0;
			for(; j < height; j++){
				if(x < 0) {
					cpbegin16 -= x;
					buf -= x;
					i = -x;
				}
				else
					i = 0;
				while(i < width) {
					for(; !(*buf) && i < width; i++, cpbegin16++, buf++) {
					}
					for(; (*buf) && i < width; i++, buf++, cpbegin16++)
						*cpbegin16 = MAKE_RGBA_565(pf->r, pf->g, pf->b, 0);
				}
				cpbegin16 += pimage->texw - width;
				buf += bitmap->width - width;
			}
			break;
		default:
			break;
	}
}


static void freetype2_drawtext(PFont pfont, image_p pimage, int x, int y,
							   const void *text, int cc, int flags)
{
	PFontFreetype pf = (PFontFreetype) pfont;
	uint16_t* value;
	FT_Glyph glyph;
	int pen_x = x;
	int pen_y = y + (pf->face->size->metrics.ascender >> 6);
	int i;
	FT_BitmapGlyph bitmap_glyph;
	FT_Bitmap* bitmap;

	value = _nge_ft_conv_encoding(pfont, text, &cc);
	if (cc <= 0)
		return;

	if(pimage->swizzle ==1){
		unswizzle_swap(pimage);
		pimage->dontswizzle = 1;
	}
	pimage->modified =1;
	for (i =0;i<cc;i++) {
		FT_Load_Glyph( pf->face, FT_Get_Char_Index( pf->face, value[i] ), FT_LOAD_DEFAULT );
		if(pf->flags & FLAGS_FREETYPE_BOLD)
			FT_GlyphSlot_Embolden(pf->face->glyph);
		if(pf->flags & FLAGS_FREETYPE_ITALICS)
			FT_GlyphSlot_Oblique(pf->face->glyph);
		FT_Get_Glyph( pf->face->glyph, &glyph );
		FT_Render_Glyph( pf->face->glyph, ft_render_mode_normal );
		FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
		bitmap_glyph = (FT_BitmapGlyph)glyph;
		bitmap=&bitmap_glyph->bitmap;
		draw_one_word(pf,bitmap,pimage,pen_x + pf->face->glyph->bitmap_left,pen_y - pf->face->glyph->bitmap_top );
		pen_x  +=(pf->face->glyph->advance.x+pf->fix_width*72) >> 6  ;
		FT_Done_Glyph( glyph );
	}
}

static void freetype2_setfontsize(PFont pfont, int fontsize)
{
	PFontFreetype pf = (PFontFreetype) pfont;
	pf->size = fontsize;
	FT_Set_Char_Size( pf->face,pf->size<< 6, pf->size << 6, 96, 96);
}

static void freetype2_setfontrotation(PFont pfont, int rot)
{
}

static void freetype2_setfontattr(PFont pfont, int attr, int setflags)
{
	PFontFreetype pf = (PFontFreetype) pfont;
	switch(setflags)
	{
	case SET_ATTR_FIXWIDTH:
		pf->fix_width = attr;
		break;
	case SET_ATTR_BOLD:
		pf->flags |= FLAGS_FREETYPE_BOLD;
		break;
	case SET_ATTR_NOBOLD:
		pf->flags &= ~FLAGS_FREETYPE_BOLD;
		break;
	case SET_ATTR_ITALICS:
		pf->flags |= FLAGS_FREETYPE_ITALICS;
		break;
	case SET_ATTR_NOITALICS:
		pf->flags &= ~FLAGS_FREETYPE_ITALICS;
		break;
	case SET_ATTR_MARGIN:
		break;
	case SET_ATTR_SIZE:
		freetype2_setfontsize(pfont,attr);
		break;
	default:
		break;
	}
}


static PFont freetype2_duplicate(PFont pfont, int fontsize)
{
	return pfont;
}

uint32_t  freetype2_setfontcolor(PFont pfont, uint32_t color)
{
	PFontFreetype pf = (PFontFreetype) pfont;
	uint32_t last_color;
	switch(pf->disp)
	{
		case DISPLAY_PIXEL_FORMAT_5551:
			last_color = MAKE_RGBA_5551(pf->r,pf->g,pf->b,pf->a);
			GET_RGBA_5551(color,pf->r,pf->g,pf->b,pf->a);
			break;
		case DISPLAY_PIXEL_FORMAT_4444:
			last_color = MAKE_RGBA_4444(pf->r,pf->g,pf->b,pf->a);
			GET_RGBA_4444(color,pf->r,pf->g,pf->b,pf->a);
			break;
		case DISPLAY_PIXEL_FORMAT_565:
			last_color = MAKE_RGBA_565(pf->r,pf->g,pf->b,pf->a);
			GET_RGBA_565(color,pf->r,pf->g,pf->b,pf->a);
			break;
//		case DISPLAY_PIXEL_FORMAT_8888:
	default:
			last_color = MAKE_RGBA_8888(pf->r,pf->g,pf->b,pf->a);
			GET_RGBA_8888(color,pf->r,pf->g,pf->b,pf->a);
			break;
	}
	freetype2_calcalphatable(pfont);
	return last_color;
}

void freetype2_calcalphatable(PFont pfont)	{
	int i;
	PFontFreetype pf = (PFontFreetype)pfont;
	for(i = 0; i < 256; i++)
		pf->alpha_table[i] = (i * pf->a) >> 8;
}