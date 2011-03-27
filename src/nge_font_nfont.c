#include "nge_font.h"

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

typedef struct{
	char*       data;
	int         datalen;
}workbuf;

typedef struct {
	void*	    procs;	/* font-specific rendering routines*/
	int		    size;	/* font height in pixels*/
	int		    rotation;	/* font rotation*/
	uint32		disp;	/* diplaymode*/
	//NGE font special
	uint32      color_fg;
	uint32      color_bg;
	uint32      color_sh;
	char*       p_gbkraw;		/* nfont stuff */
	char*       p_ascraw;
	char*       p_rawbuffer;
	int 		alignsize;
	char        type;
	int         flags;
	int         font_width;
	int         font_top;
	int         font_height;
	int         font_left;
	workbuf     bitbuf;
}FontNfont,*PFontNfont;


//inner use
static BOOL nfont_getfontinfo(PFont pfont, PFontInfo pfontinfo);
static void nfont_gettextsize(PFont pfont, const void *text, int cc,int flags, int *pwidth, int *pheight,int *pbase);
static void nfont_destroyfont(PFont pfont);
static void nfont_drawtext(PFont pfont, image_p pimage, int x, int y,const void *str, int count, int flags);
static void nfont_drawtext_shadow(PFont pfont, image_p pimage, int x, int y,const void *str, int count, int flags);
static void nfont_setflags(PFont pfont,int flags);
static void nfont_setcolorex(PFont pfont, uint32 color_fg,uint32 color_bg ,uint32 color_sh );
static uint32  nfont_setfontcolor(PFont pfont, uint32 color);
/* handling routines for nFONT*/
static  FontProcs nfont_procs = {
		ENCODING_ASCII,			/* routines expect ASCII*/
		nfont_getfontinfo,
		nfont_gettextsize,
		NULL,				/* nfont_gettextbits */
		nfont_destroyfont,
		nfont_drawtext,
		nfont_drawtext_shadow,
		nfont_setfontcolor,
		NULL,				/* setfontsize*/
		NULL, 				/* setfontrotation*/
		NULL,				/* setfontattr*/
		NULL,				/* duplicate not yet implemented */
		nfont_setflags,
		nfont_setcolorex
};


static void expandchar(PFontNfont pf, int bg, int fg, int c, uint16* bitmap);
static int  getnextchar(char* s, unsigned char* cc);
static void expandcchar(PFontNfont pf, int bg, int fg, unsigned char* c, uint16* bitmap);
static void copy_rawdata_image_custom_16(void* data,const image_p des,int x,int y,uint32 w,uint32 h,uint16 color_bg,uint16 color_fg,uint16 color_sh);
static void copy_rawdata_image_custom_32(void* data,const image_p des,int x,int y,uint32 w,uint32 h,uint32 color_bg,uint32 color_fg,uint32 color_sh);
static void expandcchar_16(PFontNfont pf, int bg, int fg, unsigned char* c, uint16* bitmap);
static void expandchar_16(PFontNfont pf, int bg, int fg, int c, uint16* bitmap);
static void expandcchar_32(PFontNfont pf, int bg, int fg, unsigned char* c, uint32* bitmap);
static void expandchar_32(PFontNfont pf, int bg, int fg, int c, uint32* bitmap);


PFont create_font_nfont(const char* name,int disp)
{
	int handle  = 0;
	int nsize = 0 ;
	char* nfbuf= 0;
	PFont pf = NULL;
	handle	= io_fopen(name,IO_RDONLY);
	if(handle != 0){
		nsize = io_fsize(handle);
		nfbuf = (char*)malloc(nsize);
		io_fread(nfbuf,1,nsize,handle);
		io_fclose(handle);
		pf = create_font_nfont_buf(nfbuf,nsize,disp);
		SAFE_FREE(nfbuf);
	}
	
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
	pf->size =  pheader->size;
	pf->alignsize = pheader->alignsize;
	pf->p_rawbuffer = (char*)malloc(nsize);
	memcpy(pf->p_rawbuffer,nfbuf,nsize);
	pf->p_ascraw = pf->p_rawbuffer+pheader->hdrlen;
	if(pheader->type == HFONT_TYPE_ASCII)
		pf->p_gbkraw = 0;
	else
		pf->p_gbkraw = pf->p_rawbuffer+pheader->gbkoffset;
	pf->type = pheader->type;
	pf->procs = &nfont_procs;
	pf->flags = FONT_TYPE_GBK;
	pf->bitbuf.datalen = 2048;
	pf->bitbuf.data = (char*)malloc(pf->bitbuf.datalen);
	memset(pf->bitbuf.data,0,pf->bitbuf.datalen);
	
	switch(disp)
	{
		case DISPLAY_PIXEL_FORMAT_4444:
			pf->disp = disp;
			pf->color_bg = FONT_BG_4444;
			pf->color_fg = FONT_FG_4444;
			pf->color_sh = FONT_SH_4444;
			break;
		case DISPLAY_PIXEL_FORMAT_565:
			pf->disp = disp;
			pf->color_bg = FONT_BG_565;
			pf->color_fg = FONT_FG_565;
			pf->color_sh = FONT_SH_565;
			break;
		case DISPLAY_PIXEL_FORMAT_5551:
			pf->disp = disp;
			pf->color_bg = FONT_BG_5551;
			pf->color_fg = FONT_FG_5551;
			pf->color_sh = FONT_SH_5551;
			break;
		case DISPLAY_PIXEL_FORMAT_8888:
			pf->disp = disp;
			pf->color_bg = FONT_BG_8888;
			pf->color_fg = FONT_FG_8888;
			pf->color_sh = FONT_SH_8888;
			break;
		default:
			pf->disp = DISPLAY_PIXEL_FORMAT_5551;
			pf->color_bg = FONT_BG_5551;
			pf->color_fg = FONT_FG_5551;
			pf->color_sh = FONT_SH_5551;
			break;
	}
	return (PFont)pf;
}


uint32  nfont_setfontcolor(PFont pfont, uint32 color)
{
	PFontNfont pf = (PFontNfont)pfont;
	uint32 last_color = pf->color_fg;
	pf->color_fg = color;
	return last_color;
}
void nfont_setflags(PFont pfont,int flags)
{
	PFontNfont pf = (PFontNfont)pfont;
	pf->flags = flags;
}

void nfont_setcolorex(PFont pfont, uint32 color_fg,uint32 color_bg ,uint32 color_sh )
{
	PFontNfont pf = (PFontNfont)pfont;
	pf->color_fg = color_fg;
	pf->color_bg = color_bg;
	pf->color_sh = color_sh;
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


int getnextchar(char* s, unsigned char* cc)
{
	if( s[0] == '\0') return 0;
	
	cc[0] = (unsigned char)(*s);
	cc[1] = (unsigned char)(*(s + 1));
	
	if( ((unsigned char)cc[0]>0x80) )
			return 1;
	cc[1] = '\0';
	return 1;
}

void nfont_gettextsize(PFont pfont, const void *text, int cc,int flags, int *pwidth, int *pheight,int *pbase)
{
	PFontNfont pf = (PFontNfont)pfont;
   	unsigned char c[2];
	char *s,*sbegin;
	unsigned char s1[3];
	
	int ax=0;
	s=(char *)text;
	if(cc==0)
	{
		*pwidth = 0;
		*pheight = pf->size;
		*pbase = pf->size-2;
		
	}
	if(cc==1)
	{
		s1[0]=*((unsigned char*)text);
		s1[1]=0x0;
		s1[2]=0x0;
		s=(char*)s1;
	}
	sbegin=s;
	while( getnextchar(s, c) )
	{
		if( c[1] != '\0') 
		{
			s += 2;
			ax += pf->size;
		}
		else 
		{
			s += 1;
			ax += pf->size/2;
		}
		if(s>=sbegin+cc) {
			break;
		}
		
	}
	*pwidth = ax;
	*pheight = pf->size;
	*pbase = pf->size-2;
}


void nfont_destroyfont(PFont pfont)
{
	PFontNfont pf = (PFontNfont)pfont;
	SAFE_FREE(pf->bitbuf.data);
	SAFE_FREE(pf->p_rawbuffer);
	SAFE_FREE(pf);
}


static void copy_rawdata_image_custom_16(void* data,const image_p des,int x,int y,uint32 w,uint32 h,uint16 color_bg,uint16 color_fg,uint16 color_sh)
{
	uint16* cpbegin16 = NULL;
	uint16* bmp16 = NULL;
	uint32 i = 0,j = 0;
	uint32 bufsize = des->texw*des->texh;
	cpbegin16 = (uint16*)des->data+y*des->texw+x;
	bmp16 = (uint16*)data;
	for(i =0;i<h;i++){
		for(j =0;j<w;j++){
				if(j+x>des->texw||i+y>des->texh)
					continue;
				cpbegin16[j] = bmp16[i*w+j];
				
		}
		cpbegin16 += des->texw;
	}
	
	if(color_sh!=0xffff){
		bmp16 = (uint16*)data;
		cpbegin16 = (uint16*)des->data+y*des->texw+x;
		for(i = 1;i<h+1;i++){
			for(j = 1;j<w+1;j++){
				if(j+x+1>des->texw||i+y+1>des->texh)
					continue;
				if(bmp16[(i-1)*w+(j-1)]==color_fg&&cpbegin16[j]!=color_fg)
					cpbegin16[j] = color_sh;
			}
			cpbegin16 += des->texw;
		}
	}

}

static void copy_rawdata_image_custom_32(void* data,const image_p des,int x,int y,uint32 w,uint32 h,uint32 color_bg,uint32 color_fg,uint32 color_sh)
{
	uint32* cpbegin32 = NULL;
	uint32* bmp32 = NULL;
	uint32 i = 0,j = 0;
	uint32 size = 0;
	uint32 bufsize = des->texw*des->texh;
	cpbegin32 = (uint32*)des->data+y*des->texw+x;
	bmp32 = (uint32*)data;
	for(i =0;i<h;i++){
		for(j =0;j<w;j++){
			if(j+x>des->texw||i+y>des->texh)
				continue;
			cpbegin32[j] = bmp32[i*w+j];
		}
		cpbegin32 += des->texw;
	}
	size = 0;
	if(color_sh!=0xffffffff){
		bmp32 = (uint32*)data;
		cpbegin32 = (uint32*)des->data+y*des->texw+x;
		for(i = 1;i<h+1;i++){
			for(j = 1;j<w+1;j++){
				if(j+x+1>des->texw||i+y+1>des->texh)
					continue;
				if(bmp32[(i-1)*w+(j-1)]==color_fg&&cpbegin32[j]!=color_fg)
					cpbegin32[j] = color_sh;
			}
			cpbegin32 += des->texw;
		}
	}
	
}


void expandcchar_16(PFontNfont pf, int bg, int fg, unsigned char* c, uint16* bitmap)
{
	int i = 0,j = 0,k = 0,m = 0,b =0;
	int c1, c2, seq;
	int x,y,pixelsize;
	unsigned char width = 0;
	unsigned char *font;
	
	pixelsize=sizeof(uint16);
   	c1 = c[0];
	c2 = c[1];
	seq = ( 0xbf*(c1-0x81) + (c2-0x40) ) ;

	font = pf->p_gbkraw + seq*(pf->alignsize+4);
	pf->font_width  =  font[0];
	pf->font_height =  font[1];
	pf->font_left   =  (char)font[2];
	pf->font_top    =  (char)font[3];
	font += 4;
	
	for (y = 0; y < pf->size; y++){
		for (x = 0; x < pf->font_width; x++) 
		{
			if (x % 8 == 0)
				b = *font++;
			
			if (b & (128 >> (x % 8))){  
				bitmap[i++]=(uint16)fg;
			}
			else{
				bitmap[i++]=(uint16)bg;
			}
		}
	}
}

void expandchar_16(PFontNfont pf, int bg, int fg, int c, uint16* bitmap)
{
	int i=0,b = 0;
	int x,y;
	unsigned char *font;
	
	font = pf->p_ascraw + c * (pf->alignsize+4);
	pf->font_width  =  font[0];
	pf->font_height =  font[1];
	pf->font_left   =  (char)font[2];
	pf->font_top    =  (char)font[3];
	font += 4;
	for (y = 0; y < pf->size; y++){
		for (x = 0; x < pf->font_width; x++) 
		{
			if (x % 8 == 0)
				b = *font++;
			
			if (b & (128 >> (x % 8))){  
				bitmap[i++]=(uint16)fg;
			}
			else{
				bitmap[i++]=(uint16)bg;
			}
		}
	}
}


void nfont_drawtext_16(PFontNfont pf, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
{
	unsigned char c[2];
	uint16* bitmap = NULL;
	unsigned char s1[3];
	char *s,*sbegin;
	int size;
	s=(char *)text;
	if(cc==1)
	{
		s1[0]=*((unsigned char*)text);
		s1[1]=0x0;
		s1[2]=0x0;
		s=(char*)s1;
	}
	
	sbegin=s;

	size = 2*pf->font_width*pf->size*sizeof(uint16);
	if( size > pf->bitbuf.datalen){
		pf->bitbuf.datalen = size * 2;
		free(pf->bitbuf.data);
		pf->bitbuf.data = (char*)malloc(pf->bitbuf.datalen);
		memset(pf->bitbuf.data,0,pf->bitbuf.datalen);
	}
	bitmap = (uint16*)pf->bitbuf.data;
	memset(bitmap,0,size);

	while( getnextchar(s, c) )
	{
		if( c[1] != '\0'){
			if(pf->type == HFONT_TYPE_GBK){
				expandcchar_16(pf, pf->color_bg,pf->color_fg,c, bitmap);
				copy_rawdata_image_custom_16(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,(uint16)pf->color_bg,(uint16)pf->color_fg,0xffff);
				ax += pf->font_width+1;
			}
			else
				ax += pf->size+1;
			s += 2;
		}
		else{
			expandchar_16(pf, pf->color_bg,pf->color_fg,c[0], bitmap);
			copy_rawdata_image_custom_16(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,(uint16)pf->color_bg,(uint16)pf->color_fg,0xffff);
			s += 1;
			ax += pf->font_width+1;
			printf("pf->font_left=%d\n",pf->font_left);
		}
		
		if(s>=sbegin+cc) break;
	}
	
	SAFE_FREE(bitmap);
}

void expandcchar_32(PFontNfont pf, int bg, int fg, unsigned char* c, uint32* bitmap)
{
	int i=0;
	int c1, c2, seq;
	int x,y;
	uint8 width = 0;
	unsigned char *font;
	int b = 0;	
	
	int pixelsize;
	pixelsize=sizeof(uint32);
	
   	c1 = c[0];
	c2 = c[1];
	
	seq = ( 0xbf*(c1-0x81) + (c2-0x40) ) ;
	
	font = pf->p_gbkraw + seq*(pf->alignsize+4);
	pf->font_width  =  font[0];
	pf->font_height =  font[1];
	pf->font_left   =  (char)font[2];
	pf->font_top    =  (char)font[3];
	font += 4;
	
	for (y = 0; y < pf->size; y++){
		for (x = 0; x < pf->font_width; x++) 
		{
			if (x % 8 == 0)
				b = *font++;
			
			if (b & (128 >> (x % 8)))  
				bitmap[i++]=(uint32)fg;
			else
				bitmap[i++]=(uint32)bg;
		}
	}
}

void expandchar_32(PFontNfont pf, int bg, int fg, int c, uint32* bitmap)
{
	int i=0;
	int x,y;
	unsigned char *font;
	int b = 0;		
	font = pf->p_ascraw + c * (pf->alignsize+4);
	pf->font_width  =  font[0];
	pf->font_height =  font[1];
	pf->font_left   =  (char)font[2];
	pf->font_top    =  (char)font[3];
	font += 4;

	for (y = 0; y < pf->size; y++){
		for (x = 0; x < pf->font_width; x++) 
		{
			if (x % 8 == 0)
				b = *font++;
			if (b & (128 >> (x % 8)))
				bitmap[i++]=(uint32)fg;
			else
				bitmap[i++]=(uint32)bg;
		}
		//i+= pf->afont_width;
	}
}
void nfont_drawtext_32(PFontNfont pf, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
{
	unsigned char c[2];
	uint32* bitmap = NULL;
	unsigned char s1[3];
	char *s,*sbegin;
	int size;
	s=(char *)text;
	if(cc==1)
	{
		s1[0]=*((unsigned char*)text);
		s1[1]=0x0;
		s1[2]=0x0;
		s=(char*)s1;
	}
	sbegin=s;

	size = 2*pf->font_width*pf->size*sizeof(uint32);
	if( size > pf->bitbuf.datalen){
		pf->bitbuf.datalen = size * 2;
		free(pf->bitbuf.data);
		pf->bitbuf.data = (char*)malloc(pf->bitbuf.datalen);
		memset(pf->bitbuf.data,0,pf->bitbuf.datalen);
	}
	bitmap = (uint32*)pf->bitbuf.data;
	memset(bitmap,0,size);
	
	while( getnextchar(s, c) )
	{
		if( c[1] != '\0'){
			if(pf->type == HFONT_TYPE_GBK){
				expandcchar_32(pf, pf->color_bg,pf->color_fg,c, bitmap);
				copy_rawdata_image_custom_32(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,(uint32)pf->color_bg,(uint32)pf->color_fg,0xffffffff);
				ax += pf->font_width + 1;
			}
			else
				ax += pf->size+1;
			s += 2;
		}
		else{
			expandchar_32(pf, pf->color_bg,pf->color_fg,c[0], bitmap);
			copy_rawdata_image_custom_32(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,(uint32)pf->color_bg,(uint32)pf->color_fg,0xffffffff);
			s += 1;
			ax += pf->font_width +1;
		}
		
		if(s>=sbegin+cc) break;
	}
	
	SAFE_FREE(bitmap);
}

void nfont_drawtext(PFont pfont, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
{
	PFontNfont pf = (PFontNfont)pfont;
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


void nfont_drawtext_shadow_16(PFontNfont pf, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
{
	unsigned char c[2];
	uint16* bitmap = NULL;
	unsigned char s1[3];
	int size;
	char *s,*sbegin;
	s=(char *)text;
	if(cc==1)
	{
		s1[0]=*((unsigned char*)text);
		s1[1]=0x0;
		s1[2]=0x0;
		s=(char*)s1;
	}
	
	sbegin=s;
	size = 2*pf->font_width*pf->size*sizeof(uint16);
	if( size > pf->bitbuf.datalen){
		pf->bitbuf.datalen = size * 2;
		free(pf->bitbuf.data);
		pf->bitbuf.data = (char*)malloc(pf->bitbuf.datalen);
		memset(pf->bitbuf.data,0,pf->bitbuf.datalen);
	}
	bitmap = (uint16*)pf->bitbuf.data;
	memset(bitmap,0,size);

	while( getnextchar(s, c) )
	{
		if( c[1] != '\0'){
			if(pf->type == HFONT_TYPE_GBK){
				expandcchar_16(pf, pf->color_bg,pf->color_fg,c, bitmap);
				copy_rawdata_image_custom_16(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,(uint16)pf->color_bg,(uint16)pf->color_fg,(uint16)pf->color_sh);
				ax += pf->font_width+1;
			}
			else
				ax += pf->size+1;
			s += 2;
		}
		else{
			expandchar_16(pf, pf->color_bg,pf->color_fg,c[0], bitmap);
			copy_rawdata_image_custom_16(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,(uint16)pf->color_bg,(uint16)pf->color_fg,(uint16)pf->color_sh);
			s += 1;
			ax += pf->font_width+1;
		}
		
		if(s>=sbegin+cc) break;
	}
	SAFE_FREE(bitmap);
}


void nfont_drawtext_shadow_32(PFontNfont pf, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
{
	unsigned char c[2];
	uint32* bitmap = NULL;
	unsigned char s1[3];
	char *s,*sbegin;
	int size;
	s=(char *)text;
	if(cc==1)
	{
		s1[0]=*((unsigned char*)text);
		s1[1]=0x0;
		s1[2]=0x0;
		s=(char*)s1;
	}
	sbegin=s;
	size = 2*pf->font_width*pf->size*sizeof(uint32);
	if( size > pf->bitbuf.datalen){
		pf->bitbuf.datalen = size * 2;
		free(pf->bitbuf.data);
		pf->bitbuf.data = (char*)malloc(pf->bitbuf.datalen);
		memset(pf->bitbuf.data,0,pf->bitbuf.datalen);
	}
	bitmap = (uint32*)pf->bitbuf.data;
	memset(bitmap,0,size);
	
	while( getnextchar(s, c) )
	{
		if( c[1] != '\0'){
			if(pf->type == HFONT_TYPE_GBK){
				expandcchar_32(pf, pf->color_bg,pf->color_fg,c, bitmap);
				copy_rawdata_image_custom_32(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,pf->color_bg,pf->color_fg,pf->color_sh);
				ax += pf->font_width + 1;
			}
			else
				ax += pf->size + 1;
			s += 2;
		}
		else{
			expandchar_32(pf, pf->color_bg,pf->color_fg,c[0], bitmap);
			copy_rawdata_image_custom_32(bitmap,pimage,ax,ay+(pf->size-pf->font_top),pf->font_width,pf->font_height,pf->color_bg,pf->color_fg,pf->color_sh);
			s += 1;
			ax += pf->font_width +1;
		}
		
		if(s>=sbegin+cc) break;
	}
	SAFE_FREE(bitmap);
}


void nfont_drawtext_shadow(PFont pfont, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
{
	PFontNfont pf = (PFontNfont)pfont;
	if(pimage->dtype==pf->disp){
		if(pf->disp==DISPLAY_PIXEL_FORMAT_8888){
			nfont_drawtext_shadow_32(pf,pimage,ax,ay,text,cc, flags);
		}
		else{
			nfont_drawtext_shadow_16(pf,pimage,ax,ay,text,cc, flags);
		}
	}
}
