#include "nge_debug_log.h"
#include "nge_font.h"
#include "nge_font_internal.h"

uint8_t nge_font_encoding = NGE_ENCODING_GBK;

void NGE_SetFontEncoding(uint8_t encoding) {
	nge_font_encoding = encoding;
}

void font_drawtext(PFont pf,const char* text,int len,image_p textarea,int dx,int dy,int flags)
{
	PFontProcs procs;

	if(pf == NULL || text == NULL || textarea ==NULL || len == 0)
		return;
	procs = pf->procs;
	if(flags == FONT_SHOW_SHADOW){
		if(procs->DrawTextShadow!=NULL){
			procs->DrawTextShadow(pf,textarea,dx,dy,text,len,0);
		}
		else{
			procs->DrawText(pf,textarea,dx,dy,text,len,0);
		}
	}
	else{
		procs->DrawText(pf,textarea,dx,dy,text,len,0);
	}
}

void font_setattr(PFont pf,int attr,int setflag)
{
	PFontProcs procs;
	if(pf == NULL)
		return;
	procs = pf->procs;
	if(procs->SetFontAttr!=NULL)
		procs->SetFontAttr(pf,attr,setflag);
}

void font_destory(PFont pf)
{
	PFontProcs procs;
	if(pf == NULL)
		return;
	procs = pf->procs;
	if(procs->DestroyFont != NULL)
		procs->DestroyFont(pf);
}

int font_setcolor(PFont pf,uint32_t color)
{
	PFontProcs procs;
	if(pf == NULL)
		return 0;
	procs = pf->procs;
	if(procs->SetFontColor != NULL)
		return procs->SetFontColor(pf,color);
	return 0;
}

void font_setcolor_ex(PFont pf, uint32_t color_fg,uint32_t color_bg ,uint32_t color_sh )
{
	PFontProcs procs;
	if(pf == NULL)
		return;
	procs = pf->procs;
	if(procs->SetShadowColor!=NULL)
		procs->SetShadowColor(pf,color_fg,color_bg,color_sh);
	else
		procs->SetFontColor(pf,color_fg);
}

void font_textsize(PFont pf, const void *text, int cc,int *pwidth, int *pheight,int *pbase)
{
	PFontProcs procs;
	if(pf == NULL)
		return ;
	procs = pf->procs;
	if(procs->GetTextSize != NULL)
		procs->GetTextSize(pf,text,cc,0/*flag*/,pwidth,pheight,pbase);
}
