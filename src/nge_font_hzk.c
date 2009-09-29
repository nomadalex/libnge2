#include "nge_font.h"
#include "nge_image_load.h"

typedef struct {
	void*	    procs;	/* font-specific rendering routines*/
	int		    size;	/* font height in pixels*/
	int		    rotation;	/* font rotation*/
	uint32		disp;	/* diplaymode*/
//hzk special
	uint32      color_fg;
	uint32      color_bg;
	uint32      color_sh;
	uint8*          cfont_raw;		/* hzkfont stuff */
	uint8* 	        afont_raw;
	int 			afont_width;
	int 			cfont_width;
	int 			font_height;
	int             flags;
}FontHzk,*PFontHzk;

//inner use
static BOOL hzk_getfontinfo(PFont pfont, PFontInfo pfontinfo);
static void hzk_gettextsize(PFont pfont, const void *text, int cc,int flags, int *pwidth, int *pheight,int *pbase);
static void hzk_destroyfont(PFont pfont);
static void hzk_drawtext(PFont pfont, image_p pimage, int x, int y,const void *str, int count, int flags);
static void hzk_drawtext_shadow(PFont pfont, image_p pimage, int x, int y,const void *str, int count, int flags);
static void hzk_setflags(PFont pfont,int flags);
static void hzk_setcolorex(PFont pfont, uint32 color_fg,uint32 color_bg ,uint32 color_sh );
static uint32  hzk_setfontcolor(PFont pfont, uint32 color);
/* handling routines for HZKFONT*/
static  FontProcs hzk_procs = {
	    ENCODING_ASCII,			/* routines expect ASCII*/
		hzk_getfontinfo,
		hzk_gettextsize,
		NULL,				/* hzk_gettextbits */
		hzk_destroyfont,
		hzk_drawtext,
		hzk_drawtext_shadow,
		hzk_setfontcolor,
		NULL,				/* setfontsize*/
		NULL, 				/* setfontrotation*/
		NULL,				/* setfontattr*/
		NULL,				/* duplicate not yet implemented */
		hzk_setflags,
		hzk_setcolorex
};


static int use_big5 = 0;

static void expandchar(PFontHzk pf, int bg, int fg, int c, uint16* bitmap);
static int getnextchar(char* s, unsigned char* cc);
static void expandcchar(PFontHzk pf, int bg, int fg, unsigned char* c, uint16* bitmap);
static void copy_rawdata_image_custom_16(void* data,const image_p des,int x,int y,uint32 w,uint32 h,uint16 color_bg,uint16 color_fg,uint16 color_sh);
static void copy_rawdata_image_custom_32(void* data,const image_p des,int x,int y,uint32 w,uint32 h,uint32 color_bg,uint32 color_fg,uint32 color_sh);
static void expandcchar_16(PFontHzk pf, int bg, int fg, unsigned char* c, uint16* bitmap);
static void expandchar_16(PFontHzk pf, int bg, int fg, int c, uint16* bitmap);
static void expandcchar_32(PFontHzk pf, int bg, int fg, unsigned char* c, uint32* bitmap);
static void expandchar_32(PFontHzk pf, int bg, int fg, int c, uint32* bitmap);


PFont create_font_hzk(const char* cname,const char* aname, int height,int disp)
{
	int handle  = 0;
	int asize = 0 ;
	int csize = 0;
	char* afbuf= 0;
	char* cfbuf = 0;
	PFont pf;
	handle	= io_fopen(cname,IO_RDONLY);
	if(handle != 0){
		csize = io_fsize(handle);
		cfbuf = (char*)malloc(csize);
		io_fread(cfbuf,1,csize,handle);
		io_fclose(handle);
	}
	else
		return NULL;
	handle = io_fopen(aname,IO_RDONLY);
	if(handle != 0){
		asize = io_fsize(handle);
		afbuf = (char*)malloc(asize);
		io_fread(afbuf,1,asize,handle);
		io_fclose(handle);
	}
	else
		return NULL;
	pf = create_font_hzk_buf(cfbuf,csize,afbuf,asize,height,disp);
	SAFE_FREE(cfbuf);
	SAFE_FREE(afbuf);
	return pf;
}

PFont create_font_hzk_buf(const char *cfbuf,int csize,const char* afbuf,int asize, int height,int disp)
{
	PFontHzk pf;
	pf = (PFontHzk)malloc(sizeof(FontHzk));
	if (!pf)
		return NULL;
	memset(pf,0,sizeof(FontHzk));
	pf->procs = &hzk_procs;
	pf->size = height;
	pf->rotation = 0;
	pf->flags = FONT_TYPE_GBK;
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
	if(height==12){		
		pf->afont_width = 6;
		pf->cfont_width = 12;
		pf->font_height = 12;
	}
	else if(height==16){		
		pf->afont_width = 8;
		pf->cfont_width = 16;
		pf->font_height = 16;
	}
	else {
		pf->afont_width = height/2;
		pf->cfont_width = height;
		pf->font_height = height;
	}

	/* Load the font library to the system memory.*/
	if(csize > 0){
		pf->cfont_raw = (uint8*)malloc(csize);
		memcpy(pf->cfont_raw,cfbuf,csize);
	}
	if(asize > 0){
		pf->afont_raw = (uint8*)malloc(asize);
		memcpy(pf->afont_raw,afbuf,asize);
	}

	return (PFont)pf;
}


uint32  hzk_setfontcolor(PFont pfont, uint32 color)
{
	PFontHzk pf = (PFontHzk)pfont;
	uint32 last_color = pf->color_fg;
	pf->color_fg = color;
	return last_color;
}
void hzk_setflags(PFont pfont,int flags)
{
	PFontHzk pf = (PFontHzk)pfont;
	pf->flags = flags;
}

void hzk_setcolorex(PFont pfont, uint32 color_fg,uint32 color_bg ,uint32 color_sh )
{
	PFontHzk pf = (PFontHzk)pfont;
	pf->color_fg = color_fg;
	pf->color_bg = color_bg;
	pf->color_sh = color_sh;
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

static int IsBig5(int i)
{
	if ((i>=0xa140 && i<=0xa3bf) || /* a140-a3bf(!a3e0) */
		(i>=0xa440 && i<=0xc67e) || /* a440-c67e        */
		(i>=0xc6a1 && i<=0xc8d3) || /* c6a1-c8d3(!c8fe) */
		(i>=0xc940 && i<=0xf9fe))   /* c940-f9fe        */
		return 1;
	else
		return 0;
}
int getnextchar(char* s, unsigned char* cc)
{
	if( s[0] == '\0') return 0;
	
	cc[0] = (unsigned char)(*s);
	cc[1] = (unsigned char)(*(s + 1));
	
	if (use_big5)
	{
		if( IsBig5( (int) ( (cc[0] << 8) + cc[1]) ) )
			return 1;
	}
	else
	{
		if( ((unsigned char)cc[0] > 0xa0) &&
			((unsigned char)cc[1] > 0xa0) )
			return 1;
	}
	
	cc[1] = '\0';
	
	return 1;
}

void hzk_gettextsize(PFont pfont, const void *text, int cc,int flags, int *pwidth, int *pheight,int *pbase)
{
	PFontHzk pf = (PFontHzk)pfont;
   	unsigned char c[2];
	char *s,*sbegin;
	unsigned char s1[3];
	
	int ax=0;
	s=(char *)text;
	if(cc==0)
	{
		*pwidth = 0;
		*pheight = pf->font_height;
		*pbase = pf->font_height-2;
		
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
			ax += pf->cfont_width;
		}
		else 
		{
			s += 1;
			ax += pf->afont_width;
		}
		if(s>=sbegin+cc) {
			/*printf("s=%x,sbegin=%x,cc=%x\n",s,sbegin,cc);*/
			break;
		}
		
	}
	/*printf("ax=%d,\n",ax);*/
	*pwidth = ax;
	*pheight = pf->font_height;
	*pbase = pf->font_height-2;
}


void hzk_destroyfont(PFont pfont)
{
	PFontHzk pf = (PFontHzk)pfont;
	SAFE_FREE(pf->afont_raw);
	SAFE_FREE(pf->cfont_raw);
	SAFE_FREE(pf);
}

void expandcchar_16(PFontHzk pf, int bg, int fg, unsigned char* c, uint16* bitmap)
{
	int i=0;
	int c1, c2, seq;
	int x,y;
	unsigned char *font;
	int b = 0;		/* keep gcc happy with b = 0 - MW */
	
	int pixelsize;
	pixelsize=sizeof(uint16);
	
   	c1 = c[0];
	c2 = c[1];
	if (use_big5)
	{
		seq=0;
		/* ladd=loby-(if(loby<127)?64:98)*/
		c2-=(c2<127?64:98);   
		
		/* hadd=(hiby-164)*157*/
		if (c1>=0xa4)	/* standard font*/
		{
			seq=(((c1-164)*157)+c2);
			if (seq>=5809) seq-=408;
		}
		
		/* hadd=(hiby-161)*157*/
		if (c1<=0xa3)	/* special font*/
			seq=(((c1-161)*157)+c2)+13094;
	}
	else{
		if(pf->flags == FONT_TYPE_HZK)
			seq=((c1 - 161)*94 + c2 - 161);
		else
			seq = ( 0xbf*(c1-0x81) + (c2-0x40) ) ;
	}
	font = pf->cfont_raw + ((seq) *
		(pf->font_height * ((pf->cfont_width + 7) / 8)));
	
	for (y = 0; y < pf->font_height; y++){
		for (x = 0; x < pf->cfont_width; x++) 
		{
			if (x % 8 == 0)
				b = *font++;
			
			if (b & (128 >> (x % 8)))  
				bitmap[i++]=(uint16)fg;
			else
				bitmap[i++]=(uint16)bg;
		}
	}
}

void expandchar_16(PFontHzk pf, int bg, int fg, int c, uint16* bitmap)
{
	int i=0;
	int x,y;
	unsigned char *font;
	int b = 0;		
	font = pf->afont_raw + c * (pf->font_height * ((pf->afont_width + 7) / 8));
	for (y = 0; y < pf->font_height; y++){
		for (x = 0; x < pf->afont_width; x++) 
		{
			if (x % 8 == 0)
				b = *font++;
			if (b & (128 >> (x % 8)))
				bitmap[i++]=fg;
			else
				bitmap[i++]=bg;
		}
		i+= pf->afont_width;
	}
}


void hzk_drawtext_16(PFontHzk pf, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
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
	size = pf->cfont_width * pf->font_height *sizeof(uint16);
	bitmap = (uint16 *)malloc(size);
	memset(bitmap,0,size);
	while( getnextchar(s, c) )
	{
		if( c[1] != '\0'){
			expandcchar_16(pf, pf->color_bg,pf->color_fg,c, bitmap);
			//rawdata_to_image(bitmap,pimage,ax,ay,pf->cfont_width,pf->font_height);
			copy_rawdata_image_custom_16(bitmap,pimage,ax,ay,pf->cfont_width,pf->font_height,(uint16)pf->color_bg,(uint16)pf->color_fg,0xffff);
			s += 2;
			ax += pf->cfont_width;
		}
		else{
			expandchar_16(pf, pf->color_bg,pf->color_fg,c[0], bitmap);
			//rawdata_to_image(bitmap,pimage,ax,ay,pf->afont_width,pf->font_height);
			copy_rawdata_image_custom_16(bitmap,pimage,ax,ay,pf->afont_width,pf->font_height,(uint16)pf->color_bg,(uint16)pf->color_fg,0xffff);
			s += 1;
			ax += pf->afont_width;
		}
		
		if(s>=sbegin+cc) break;
	}
	
	SAFE_FREE(bitmap);
}
void expandcchar_32(PFontHzk pf, int bg, int fg, unsigned char* c, uint32* bitmap)
{
	int i=0;
	int c1, c2, seq;
	int x,y;
	unsigned char *font;
	int b = 0;		/* keep gcc happy with b = 0 - MW */
	
	int pixelsize;
	pixelsize=sizeof(uint32);
	
   	c1 = c[0];
	c2 = c[1];
	if (use_big5)
	{
		seq=0;
		/* ladd=loby-(if(loby<127)?64:98)*/
		c2-=(c2<127?64:98);   
		
		/* hadd=(hiby-164)*157*/
		if (c1>=0xa4)	/* standard font*/
		{
			seq=(((c1-164)*157)+c2);
			if (seq>=5809) seq-=408;
		}
		
		/* hadd=(hiby-161)*157*/
		if (c1<=0xa3)	/* special font*/
			seq=(((c1-161)*157)+c2)+13094;
	}
	else{
		if(pf->flags == FONT_TYPE_HZK)
			seq=((c1 - 161)*94 + c2 - 161);
		else
			seq = ( 0xbf*(c1-0x81) + (c2-0x40) ) ;
	}
	font = pf->cfont_raw + ((seq) *
		(pf->font_height * ((pf->cfont_width + 7) / 8)));
	
	for (y = 0; y < pf->font_height; y++){
		for (x = 0; x < pf->cfont_width; x++) 
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

void expandchar_32(PFontHzk pf, int bg, int fg, int c, uint32* bitmap)
{
	int i=0;
	int x,y;
	unsigned char *font;
	int b = 0;		
	font = pf->afont_raw + c * (pf->font_height * ((pf->afont_width + 7) / 8));
	for (y = 0; y < pf->font_height; y++){
		for (x = 0; x < pf->afont_width; x++) 
		{
			if (x % 8 == 0)
				b = *font++;
			if (b & (128 >> (x % 8)))
				bitmap[i++]=fg;
			else
				bitmap[i++]=bg;
		}
		i+= pf->afont_width;
	}
}
void hzk_drawtext_32(PFontHzk pf, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
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
	size = pf->cfont_width * pf->font_height *sizeof(uint32);
	bitmap = (uint32*)malloc(size);
	memset(bitmap,0,size);
	while( getnextchar(s, c) )
	{
		if( c[1] != '\0'){
			expandcchar_32(pf, pf->color_bg,pf->color_fg,c, bitmap);
			//rawdata_to_image(bitmap,pimage,ax,ay,pf->cfont_width,pf->font_height);
			copy_rawdata_image_custom_32(bitmap,pimage,ax,ay,pf->cfont_width,pf->font_height,(uint32)pf->color_bg,(uint32)pf->color_fg,0xffffffff);
			s += 2;
			ax += pf->cfont_width;
		}
		else{
			expandchar_32(pf, pf->color_bg,pf->color_fg,c[0], bitmap);
			//rawdata_to_image(bitmap,pimage,ax,ay,pf->afont_width,pf->font_height);
			copy_rawdata_image_custom_32(bitmap,pimage,ax,ay,pf->afont_width,pf->font_height,(uint32)pf->color_bg,(uint32)pf->color_fg,0xffffffff);
			s += 1;
			ax += pf->afont_width;
		}
		
		if(s>=sbegin+cc) break;
	}
	
	SAFE_FREE(bitmap);
}

void hzk_drawtext(PFont pfont, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
{
	PFontHzk pf = (PFontHzk)pfont;
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
				cpbegin16[j] = bmp16[i*h+j];
				
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
				if(bmp16[(i-1)*h+(j-1)]==color_fg&&cpbegin16[j]!=color_fg)
					cpbegin16[j] = color_sh;
			}
			cpbegin16 += des->texw;
		}
	}

}
void hzk_drawtext_shadow_16(PFontHzk pf, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
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
	size = pf->cfont_width * pf->font_height *sizeof(uint16);
	bitmap = (uint16 *)malloc(size);
	memset(bitmap,0,size);
	while( getnextchar(s, c) )
	{
		if( c[1] != '\0'){
			expandcchar_16(pf, pf->color_bg,pf->color_fg,c, bitmap);
			copy_rawdata_image_custom_16(bitmap,pimage,ax,ay,pf->cfont_width,pf->font_height,(uint16)pf->color_bg,(uint16)pf->color_fg,(uint16)pf->color_sh);
			s += 2;
			ax += pf->cfont_width;
		}
		else{
			expandchar_16(pf, pf->color_bg,pf->color_fg,c[0], bitmap);
			copy_rawdata_image_custom_16(bitmap,pimage,ax,ay,pf->afont_width,pf->font_height,(uint16)pf->color_bg,(uint16)pf->color_fg,(uint16)pf->color_sh);
			s += 1;
			ax += pf->afont_width;
		}
		
		if(s>=sbegin+cc) break;
	}
	SAFE_FREE(bitmap);
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
				cpbegin32[j] = bmp32[i*h+j];
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
				if(bmp32[(i-1)*h+(j-1)]==color_fg&&cpbegin32[j]!=color_fg)
					cpbegin32[j] = color_sh;
			}
			cpbegin32 += des->texw;
		}
	}

}
void hzk_drawtext_shadow_32(PFontHzk pf, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
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
	size = pf->cfont_width * pf->font_height *sizeof(uint32);
	bitmap = (uint32 *)malloc(size);
	memset(bitmap,0,size);
	while( getnextchar(s, c) )
	{
		if( c[1] != '\0'){
			expandcchar_32(pf, pf->color_bg,pf->color_fg,c, bitmap);
			copy_rawdata_image_custom_32(bitmap,pimage,ax,ay,pf->cfont_width,pf->font_height,pf->color_bg,pf->color_fg,pf->color_sh);
			s += 2;
			ax += pf->cfont_width;
		}
		else{
			expandchar_32(pf, pf->color_bg,pf->color_fg,c[0], bitmap);
			copy_rawdata_image_custom_32(bitmap,pimage,ax,ay,pf->afont_width,pf->font_height,pf->color_bg,pf->color_fg,pf->color_sh);
			s += 1;
			ax += pf->afont_width;
		}
		
		if(s>=sbegin+cc) break;
	}
	SAFE_FREE(bitmap);
}


void hzk_drawtext_shadow(PFont pfont, image_p pimage, int ax, int ay,const void *text, int cc, int flags)
{
	PFontHzk pf = (PFontHzk)pfont;
	if(pimage->dtype==pf->disp){
		if(pf->disp==DISPLAY_PIXEL_FORMAT_8888){
			hzk_drawtext_shadow_32(pf,pimage,ax,ay,text,cc, flags);
		}
		else{
			hzk_drawtext_shadow_16(pf,pimage,ax,ay,text,cc, flags);
		}
	}
}