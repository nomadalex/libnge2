#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRIGONOMETRY_H
#include FT_GLYPH_H
#include FT_STROKER_H   
#include FT_SYNTHESIS_H   
#include "nge_font.h"
#include "nge_image_load.h"
typedef struct 
{
	void*	    procs;	/* font-specific rendering routines*/
	int		    size;	/* font height in pixels*/
	int		    rotation;	/* font rotation*/
	uint32		disp;	/* diplaymode*/
	/* freetype stuff */
	//color 
	uint8  r;
	uint8  g;
	uint8  b;
	uint8  a;
	int   fix_width;
	FT_Face face;
	FT_Matrix matrix;
	FT_Library library;
	int flags;
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
static void freetype2_setflags(PFont pfont,int flags);
static uint32  freetype2_setfontcolor(PFont pfont, uint32 color);
static FontProcs freetype2_procs = {
	ENCODING_GBK,		/* routines expect unicode 16 */
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
	FT_Set_Char_Size( pf->face,height<< 6, height << 6, 96, 96); 
	pf->procs = &freetype2_procs;
	pf->size = height;
	pf->disp = disp==0?DISPLAY_PIXEL_FORMAT_5551:disp;
	pf->r = 0;
	pf->g = 0;
	pf->a = 0;
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
	return (PFont)pf;
}


static void freetype2_setflags(PFont pfont,int flags)
{
	PFontFreetype pf = (PFontFreetype) pfont;
	pf->flags = flags;
}
static BOOL freetype2_getfontinfo(PFont pfont, PFontInfo pfontinfo)
{
	PFontFreetype pf = (PFontFreetype) pfont;
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


static void freetype2_gettextsize(PFont pfont, const void *text, int cc,
								  int flags, int *pwidth, int *pheight,
								  int *pbase)
{
	FT_Face face;
	FT_Size size;
	//const unsigned short *str = text;
	int need_free = 0,len;
	uint16* value;
	int char_index;
	int total_advance;
	int max_ascent;
	int max_descent;
	int advance;
	int ascent;
	int descent;
	FT_Error error;
	//FT_Vector kerning_delta;
	//int use_kerning;
	int cur_glyph_code;
	int last_glyph_code = 0;	/* Used for kerning */
	PFontFreetype pf = (PFontFreetype) pfont;
	if(((PFontProcs)(pf->procs))->encoding== ENCODING_GBK){
		len = strlen((char*)text);
		value = (uint16*)malloc(len);
		cc = gbk_to_unicode(value,(char*)text,len);
		need_free = 1;
	}
	else{
		value = (uint16*)text;
	}
	face = pf->face;
	size = face->size;
	/*
	* Starting point
	*/
	total_advance = 0;
	max_ascent  = 0;
	max_descent = 0;
	
	for (char_index = 0; char_index < cc; char_index++) {
		cur_glyph_code = FT_Get_Char_Index( pf->face, value[char_index] );//LOOKUP_CHAR(pf, face, str[char_index]);
		
		last_glyph_code = cur_glyph_code;
		
		error = freetype2_get_glyph_size(pf, face, cur_glyph_code, &advance, &ascent, &descent);
		if (error)
			continue;
		
		total_advance += advance;
		if (max_ascent < ascent)
			max_ascent = ascent;
		if (max_descent < descent)
			max_descent = descent;
	}
	
	*pwidth = total_advance;
	*pheight = max_ascent + max_descent;
	*pbase = max_ascent;
	if(need_free){
		SAFE_FREE(value);
	}
}
static void freetype2_destroyfont(PFont pfont)
{
	PFontFreetype pf = (PFontFreetype) pfont;
	FT_Done_Face(pf->face);
	FT_Done_FreeType( pf->library );
	SAFE_FREE(pf);
}


static void draw_one_word(PFontFreetype pf,FT_Bitmap* bitmap,image_p pimage,int x,int y)
{
	uint8 a,dgree;
	uint32 height = bitmap->rows;
	uint32 width = bitmap->width;
	uint32 i,j;
	uint32* cpbegin32;
	uint16* cpbegin16;

	if(pimage->dtype == DISPLAY_PIXEL_FORMAT_8888){
		cpbegin32 = (uint32*)pimage->data+y*pimage->texw+x;
		for(j=0; j < height ; j++){ 
			for(i=0; i < width; i++){
				if(x+i>pimage->texw||y+j>pimage->texh)
					continue;
				if(i>=width || j>=height){
					a = 0;
				}
				dgree = bitmap->buffer[i + bitmap->width*j];
				a = dgree;
				//pimage->data[(i+(height-j-1)*width)]=MAKE_RGBA_5551(r,g,b,a);
				cpbegin32[i]=MAKE_RGBA_8888(pf->r,pf->g,pf->b,a);
			}
			cpbegin32 += pimage->texw;
		}
	}
	else{
		cpbegin16 = (uint16*)pimage->data+y*pimage->texw+x;
		for(j=0; j < height ; j++){ 
			for(i=0; i < width; i++){
				if(x+i>pimage->texw||y+j>pimage->texh)
					continue;
				if(i>=width || j>=height){
					a = 0;
				}
				dgree = bitmap->buffer[i + bitmap->width*j];
				a = dgree;
				//pimage->data[(i+(height-j-1)*width)]=MAKE_RGBA_5551(r,g,b,a);
				if(pimage->dtype = DISPLAY_PIXEL_FORMAT_4444)
					cpbegin16[i]=MAKE_RGBA_4444(pf->r,pf->g,pf->b,a);
				else if(pimage->dtype = DISPLAY_PIXEL_FORMAT_5551){
					cpbegin16[i]=MAKE_RGBA_5551(pf->r,pf->g,pf->b,a);
				}
				else{
					cpbegin16[i]=MAKE_RGBA_565(pf->r,pf->g,pf->b,a);
				}
			}
			cpbegin16 += pimage->texw;
		}
	}
}

//FT_Load_Glyph( pf->face, FT_Get_Char_Index( pf->face, value[0] ), FT_LOAD_RENDER | FT_LOAD_MONOCHROME );

static void freetype2_drawtext(PFont pfont, image_p pimage, int x, int y,
							   const void *text, int cc, int flags)
{
	PFontFreetype pf = (PFontFreetype) pfont;
	int need_free = 0,len;
	uint16* value;
	FT_Glyph glyph; 
	int pen_x = x;
	int pen_y = y + pf->size;
	int i;
	FT_BitmapGlyph bitmap_glyph;
	FT_Bitmap* bitmap;

	if(((PFontProcs)(pf->procs))->encoding== ENCODING_GBK){
		len = strlen((char*)text);
		value = (uint16*)malloc(len*sizeof(uint16));
		memset(value,0,len*sizeof(uint16));
		cc = gbk_to_unicode(value,(char*)text,len);
		need_free = 1;
	}
	else{
		value = (uint16*)text;
	}
	if(pimage->swizzle ==1){
		unswizzle_swap(pimage);
		pimage->dontswizzle = 1;
	}
	pimage->modified =1;
	for (i =0;i<cc;i++) {
		FT_Load_Glyph( pf->face, FT_Get_Char_Index( pf->face, value[i] ), FT_LOAD_DEFAULT );
		if(pf->flags == FLAGS_FREETYPE_BOLDER)
			FT_GlyphSlot_Embolden(pf->face->glyph);
		FT_Get_Glyph( pf->face->glyph, &glyph );
		FT_Render_Glyph( pf->face->glyph, ft_render_mode_normal ); 
		FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 ); 
		bitmap_glyph = (FT_BitmapGlyph)glyph; 
		bitmap=&bitmap_glyph->bitmap;
		draw_one_word(pf,bitmap,pimage,pen_x + pf->face->glyph->bitmap_left,pen_y - pf->face->glyph->bitmap_top );
		pen_x  +=(pf->face->glyph->advance.x+pf->fix_width*72) >> 6  ;
		FT_Done_Glyph( glyph );
	}
	//if (buffer[j * pitch + i / 8] & (0x80 >> (i % 8))) 
	if(need_free){
		SAFE_FREE(value);
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
	PFontFreetype pf = (PFontFreetype) pfont;
}
static void freetype2_setfontattr(PFont pfont, int attr, int setflags)
{
	PFontFreetype pf = (PFontFreetype) pfont;
	pf->fix_width = attr;
}
static PFont freetype2_duplicate(PFont pfont, int fontsize)
{
	PFontFreetype pf = (PFontFreetype) pfont;
	return pfont;
}

uint32  freetype2_setfontcolor(PFont pfont, uint32 color)
{
	PFontFreetype pf = (PFontFreetype) pfont;
	uint32 last_color;
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
		case DISPLAY_PIXEL_FORMAT_8888:
			last_color = MAKE_RGBA_8888(pf->r,pf->g,pf->b,pf->a);
			GET_RGBA_8888(color,pf->r,pf->g,pf->b,pf->a);
			break;
	}
	return last_color;
}