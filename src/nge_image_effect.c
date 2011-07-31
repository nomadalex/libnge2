#include "nge_debug_log.h"
#include "nge_image_effect.h"
#include "nge_graphics.h"
#include "nge_misc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined WIN32 || defined __linux__

#if defined(WIN32) // on WIN32, gl need it
#define WINGDIAPI
#define APIENTRY WINAPI
#define WINAPI __stdcall
#endif

#include <GL/gl.h>

#if defined __linux__
#include <X11/Xlib.h>
#include <GL/glx.h>

#else
#include <SDL.h>
#endif

#elif defined IPHONEOS || defined ANDROID

#ifdef IPHONEOS
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#else
#include <GLES/gl.h>
#endif

#define glOrtho glOrthof
#endif

#define ___Max(x, y) (x>y?x:y)

//////////////////////////////////////////////////////////////////////////
//渐入开始
//////////////////////////////////////////////////////////////////////////
typedef struct {
	int m_type;
	int m_status;
	int m_effect_fps;
	int m_timeticks;
	nge_timer* m_ptimer;
	//公共函数
	effect_draw draw;
	effect_setparam set_param;
	effect_getparam get_param;
	effect_destroy  destroy;
	effect_getstatus status;
	//fade in/out privated
	int m_src_alpha;
	int m_des_alpha;
	float m_delta;
	float m_mins;
	int m_lastticks;
	int m_currentticks;
	int m_color;
}image_effect_fade_t,*image_effect_fade_p;

static float effect_setparam_fadein(image_effect_p effector,float param,int flags)
{
	image_effect_fade_p pfadein = (image_effect_fade_p)effector;
	float ret = 0;
	switch(flags)
	{
	case SET_EFFECT_FPS:
		if(param>0){
			ret = (float)pfadein->m_effect_fps;
			pfadein->m_effect_fps = (int)param;
		}
		break;
	case SET_EFFECT_TIMETICKS:
		if(param>0){
			ret = (float)pfadein->m_timeticks;
			pfadein->m_timeticks = (int)param;
		}
		break;
	case SET_EFFECT_FADE_SRC:
		if(param>=0&&param<=255){
			pfadein->m_src_alpha = (int)param;
			ret = (float)pfadein->m_src_alpha;
		}
		break;
	case SET_EFFECT_FADE_DES:
		if(param>=0&&param<=255){
			pfadein->m_des_alpha = (int)param;
			ret = (float)pfadein->m_des_alpha;
		}
		break;
	default:
		break;
	}
	return ret;
}

static float effect_getparam_fadein(image_effect_p effector,int flags)
{
	image_effect_fade_p pfadein = (image_effect_fade_p)effector;
	float ret = 0;
	switch(flags)
	{
	case SET_EFFECT_FPS:
		ret = (float)pfadein->m_effect_fps;
		break;
	case SET_EFFECT_TIMETICKS:
		ret = (float)pfadein->m_timeticks;
		break;
	case SET_EFFECT_FADE_SRC:
		ret = (float)pfadein->m_src_alpha;
		break;
	case SET_EFFECT_FADE_DES:
		ret = (float)pfadein->m_des_alpha;
		break;
	default:
		break;
	}
	return ret;
}

static void effect_draw_fadein(image_effect_p effector,image_p pimg,float dx,float dy)
{
	image_effect_fade_p pfadein = (image_effect_fade_p)effector;
	if(pimg == NULL || pfadein == NULL )
		return;

	switch(pfadein->m_status)
	{
	case EFFECT_INIT:
		pfadein->m_ptimer->start(pfadein->m_ptimer);
		pfadein->m_lastticks = pfadein->m_ptimer->get_ticks(pfadein->m_ptimer);
		pfadein->m_status = EFFECT_PLAY;
		pfadein->m_color = CreateColor(255,255,255,(uint8)pfadein->m_delta,pimg->dtype);
		break;
	case EFFECT_PLAY:
		pfadein->m_currentticks = pfadein->m_ptimer->get_ticks(pfadein->m_ptimer);
		if(pfadein->m_currentticks-pfadein->m_lastticks >= 1000/pfadein->m_effect_fps){
			pfadein->m_lastticks = pfadein->m_currentticks;
			pfadein->m_delta +=  pfadein->m_mins;
			if(pfadein->m_delta < pfadein->m_des_alpha){
				pfadein->m_color = CreateColor(255,255,255,(uint8)pfadein->m_delta,pimg->dtype);
			}
			else{
				pfadein->m_color = CreateColor(255,255,255,(uint8)pfadein->m_des_alpha,pimg->dtype);
				pfadein->m_status = EFFECT_STOP;
			}
		}
		break;
	case EFFECT_STOP:
		break;
	}
	RenderQuad(pimg,0,0,0,0,dx,dy,1.0,1.0,0,pfadein->m_color);
}

static void effect_destroy_fadein(image_effect_p effector)
{
	image_effect_fade_p pfadein = (image_effect_fade_p)effector;
	if(pfadein){
		if(pfadein->m_ptimer)
			timer_free(pfadein->m_ptimer);
	}
	SAFE_FREE(pfadein);
}

static int effect_status_fadein(image_effect_p effector)
{
	image_effect_fade_p pfadein = (image_effect_fade_p)effector;
	return pfadein->m_status;
}

image_effect_p effect_create_fadein(int src_alpha,int des_alpha,int timeticks)
{
	int tmp = 0;
	image_effect_fade_p pfadein = (image_effect_fade_p)malloc(sizeof(image_effect_fade_t));
	memset(pfadein,0,sizeof(image_effect_fade_t));
	if(src_alpha > des_alpha){
		//渐入应该是用小->大,如果用户参数输入错误给它交换下
		tmp = src_alpha;
		src_alpha = des_alpha;
		des_alpha = tmp;
	}
	if(src_alpha>=0&&src_alpha<=255)
		pfadein->m_src_alpha = src_alpha;
	if(des_alpha>=0&&des_alpha<=255)
		pfadein->m_des_alpha = des_alpha;
	if(timeticks>=0)
		pfadein->m_timeticks = timeticks;
	pfadein->get_param = effect_getparam_fadein;
	pfadein->set_param = effect_setparam_fadein;
	pfadein->draw      = effect_draw_fadein;
	pfadein->destroy   = effect_destroy_fadein;
	pfadein->status  = effect_status_fadein;
	pfadein->m_status = EFFECT_INIT;
	pfadein->m_type = IMAGE_EFFECT_FADEIN;
	pfadein->m_effect_fps = DEFAULT_FPS;
	pfadein->m_ptimer = timer_create();

	//计算一些参数
	pfadein->m_mins = (float)((pfadein->m_des_alpha - pfadein->m_src_alpha)*1000.0/(pfadein->m_timeticks*pfadein->m_effect_fps));

	return (image_effect_p)pfadein;
}
//////////////////////////////////////////////////////////////////////////
//渐入结束
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//渐出开始
//////////////////////////////////////////////////////////////////////////
static float effect_setparam_fadeout(image_effect_p effector,float param,int flags)
{
	image_effect_fade_p pfadeout = (image_effect_fade_p)effector;
	float ret = 0;
	switch(flags)
	{
	case SET_EFFECT_FPS:
		if(param>0){
			ret = (float)pfadeout->m_effect_fps;
			pfadeout->m_effect_fps = (int)param;
		}
		break;
	case SET_EFFECT_TIMETICKS:
		if(param>0){
			ret = (float)pfadeout->m_timeticks;
			pfadeout->m_timeticks = (int)param;
		}
		break;
	case SET_EFFECT_FADE_SRC:
		if(param>=0&&param<=255){
			ret = (float)pfadeout->m_src_alpha;
			pfadeout->m_src_alpha = (int)param;
		}
		break;
	case SET_EFFECT_FADE_DES:
		if(param>=0&&param<=255){
			ret = (float)pfadeout->m_des_alpha;
			pfadeout->m_des_alpha = (int)param;

		}
		break;
	default:
		break;
	}
	return ret;
}

static float effect_getparam_fadeout(image_effect_p effector,int flags)
{
	image_effect_fade_p pfadeout = (image_effect_fade_p)effector;
	float ret = 0;
	switch(flags)
	{
	case SET_EFFECT_FPS:
		ret = (float)pfadeout->m_effect_fps;
		break;
	case SET_EFFECT_TIMETICKS:
		ret = (float)pfadeout->m_timeticks;
		break;
	case SET_EFFECT_FADE_SRC:
		ret = (float)pfadeout->m_src_alpha;
		break;
	case SET_EFFECT_FADE_DES:
		ret = (float)pfadeout->m_des_alpha;
		break;
	default:
		break;
	}
	return ret;
}

static void effect_draw_fadeout(image_effect_p effector,image_p pimg,float dx,float dy)
{
	image_effect_fade_p pfadeout = (image_effect_fade_p)effector;
	if(pimg == NULL || pfadeout == NULL )
		return;

	switch(pfadeout->m_status)
	{
	case EFFECT_INIT:
		pfadeout->m_ptimer->start(pfadeout->m_ptimer);
		pfadeout->m_lastticks = pfadeout->m_ptimer->get_ticks(pfadeout->m_ptimer);
		pfadeout->m_delta = (float)pfadeout->m_src_alpha;
		pfadeout->m_status = EFFECT_PLAY;
		pfadeout->m_color = CreateColor(255,255,255,(uint8)pfadeout->m_delta,pimg->dtype);
		break;
	case EFFECT_PLAY:
		pfadeout->m_currentticks = pfadeout->m_ptimer->get_ticks(pfadeout->m_ptimer);
		if(pfadeout->m_currentticks - pfadeout->m_lastticks >= 1000/pfadeout->m_effect_fps){
			pfadeout->m_lastticks = pfadeout->m_currentticks;
			pfadeout->m_delta -=  pfadeout->m_mins;
			if(pfadeout->m_delta > pfadeout->m_des_alpha){
				pfadeout->m_color = CreateColor(255,255,255,(uint8)pfadeout->m_delta,pimg->dtype);
			}
			else{
				pfadeout->m_color = CreateColor(255,255,255,(uint8)pfadeout->m_des_alpha,pimg->dtype);
				pfadeout->m_status = EFFECT_STOP;
			}
		}
		break;
	case EFFECT_STOP:
		break;
	}

	RenderQuad(pimg,0,0,0,0,dx,dy,1.0,1.0,0,pfadeout->m_color);
}

static void effect_destroy_fadeout(image_effect_p effector)
{
	image_effect_fade_p pfadeout = (image_effect_fade_p)effector;
	if(pfadeout){
		if(pfadeout->m_ptimer)
			timer_free(pfadeout->m_ptimer);
	}
	SAFE_FREE(pfadeout);
}

static int effect_status_fadeout(image_effect_p effector)
{
	image_effect_fade_p pfadeout = (image_effect_fade_p)effector;
	return pfadeout->m_status;
}

image_effect_p effect_create_fadeout(int src_alpha,int des_alpha,int timeticks)
{
	int tmp = 0;
	image_effect_fade_p pfadeout = (image_effect_fade_p)malloc(sizeof(image_effect_fade_t));
	memset(pfadeout,0,sizeof(image_effect_fade_t));
	if(src_alpha < des_alpha){
		//渐出应该是用大->小,如果用户参数输入错误给它交换下
		tmp = src_alpha;
		src_alpha = des_alpha;
		des_alpha = tmp;
	}
	if(src_alpha>=0&&src_alpha<=255)
		pfadeout->m_src_alpha = src_alpha;
	if(des_alpha>=0&&des_alpha<=255)
		pfadeout->m_des_alpha = des_alpha;
	if(timeticks>=0)
		pfadeout->m_timeticks = timeticks;
	pfadeout->get_param = effect_getparam_fadeout;
	pfadeout->set_param = effect_setparam_fadeout;
	pfadeout->draw      = effect_draw_fadeout;
	pfadeout->destroy   = effect_destroy_fadeout;
	pfadeout->status  = effect_status_fadeout;
	pfadeout->m_status = EFFECT_INIT;
	pfadeout->m_type = IMAGE_EFFECT_FADEOUT;
	pfadeout->m_effect_fps = DEFAULT_FPS;
	pfadeout->m_ptimer = timer_create();

	//计算一些参数
	pfadeout->m_mins = (float)((pfadeout->m_src_alpha - pfadeout->m_des_alpha)*1000.0/(pfadeout->m_timeticks*pfadeout->m_effect_fps));

	return (image_effect_p)pfadeout;
}
//////////////////////////////////////////////////////////////////////////
//渐出结束
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//抖动效果开始
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//抖动效果结束
//////////////////////////////////////////////////////////////////////////
typedef struct {
	int m_type;
	int m_status;
	int m_effect_fps;
	int m_timeticks;
	nge_timer* m_ptimer;
	//公共函数
	effect_draw draw;
	effect_setparam set_param;
	effect_getparam get_param;
	effect_destroy  destroy;
	effect_getstatus status;
	//shake privated
	float m_shake_x;
	float m_shake_y;
	float m_dfx;
	float m_dfy;
	int   m_passtime;
	int m_lastticks;
	int m_currentticks;
}image_effect_shake_t,*image_effect_shake_p;


static float effect_setparam_shake(image_effect_p effector,float param,int flags)
{
	image_effect_shake_p pshake = (image_effect_shake_p)effector;
	float ret = 0;
	switch(flags)
	{
	case SET_EFFECT_FPS:
		if(param>0){
			ret = (float)pshake->m_effect_fps;
			pshake->m_effect_fps = (int)param;
		}
		break;
	case SET_EFFECT_TIMETICKS:
		if(param>0){
			ret = (float)pshake->m_timeticks;
			pshake->m_timeticks = (int)param;
		}
		break;
	case SET_EFFECT_SHAKE_X:
		if(param>=0){
			pshake->m_shake_x = param;
			ret = (float)pshake->m_shake_x;
		}
		break;
	case SET_EFFECT_SHAKE_Y:
		if(param>=0){
			pshake->m_shake_y = param;
			ret = (float)pshake->m_shake_y;
		}
		break;
	default:
		break;
	}
	return ret;
}

static float effect_getparam_shake(image_effect_p effector,int flags)
{
	image_effect_shake_p pshake = (image_effect_shake_p)effector;
	float ret = 0;
	switch(flags)
	{
	case SET_EFFECT_FPS:
		ret = (float)pshake->m_effect_fps;
		break;
	case SET_EFFECT_TIMETICKS:
		ret = (float)pshake->m_timeticks;
		break;
	case SET_EFFECT_SHAKE_X:
		ret = (float)pshake->m_shake_x;
		break;
	case SET_EFFECT_SHAKE_Y:
		ret = (float)pshake->m_shake_y;
		break;
	default:
		break;
	}
	return ret;
}

static void effect_draw_shake(image_effect_p effector,image_p pimg,float dx,float dy)
{
	int flag = 0;
	image_effect_shake_p pshake = (image_effect_shake_p)effector;
	if(pimg == NULL || pshake == NULL )
		return;

	switch(pshake->m_status)
	{
	case EFFECT_INIT:
		pshake->m_ptimer->start(pshake->m_ptimer);
		pshake->m_lastticks = pshake->m_ptimer->get_ticks(pshake->m_ptimer);
		pshake->m_status = EFFECT_PLAY;
		pshake->m_passtime = pshake->m_lastticks;
		pshake->m_dfx = dx;
		pshake->m_dfy = dy;
		break;
	case EFFECT_PLAY:
		pshake->m_currentticks = pshake->m_ptimer->get_ticks(pshake->m_ptimer);
		if(pshake->m_currentticks-pshake->m_lastticks >= 1000/pshake->m_effect_fps){
			pshake->m_passtime +=  (pshake->m_currentticks-pshake->m_lastticks);
			pshake->m_lastticks = pshake->m_currentticks;
			if(pshake->m_passtime <= pshake->m_timeticks){
				flag = rand_int(0, 1)==0?-1:1;
				pshake->m_dfx = dx + rand_float(0, pshake->m_shake_x)*flag;
				pshake->m_dfy = dy + rand_float(0, pshake->m_shake_y)*flag;
			}
			else{
				pshake->m_dfx = dx;
				pshake->m_dfy = dy;
				pshake->m_status = EFFECT_STOP;
			}
		}
		break;
	case EFFECT_STOP:
		break;
	}
	RenderQuad(pimg,0,0,0,0,pshake->m_dfx,pshake->m_dfy,1.0,1.0,0,pimg->mask);
}

static void effect_destroy_shake(image_effect_p effector)
{
	image_effect_shake_p pshake = (image_effect_shake_p)effector;
	if(pshake){
		if(pshake->m_ptimer)
			timer_free(pshake->m_ptimer);
	}
	SAFE_FREE(pshake);
}

static int effect_status_shake(image_effect_p effector)
{
	image_effect_shake_p pshake = (image_effect_shake_p)effector;
	return pshake->m_status;
}

image_effect_p effect_create_shake(float shake_x,float shake_y,int timeticks)
{
	//int tmp = 0;
	image_effect_shake_p pshake = (image_effect_shake_p)malloc(sizeof(image_effect_shake_t));
	memset(pshake,0,sizeof(image_effect_shake_t));

	if(shake_x>=0)
		pshake->m_shake_x = shake_x;
	if(shake_y>=0)
		pshake->m_shake_y = shake_y;
	if(timeticks>=0)
		pshake->m_timeticks = timeticks;
	pshake->get_param = effect_getparam_shake;
	pshake->set_param = effect_setparam_shake;
	pshake->draw      = effect_draw_shake;
	pshake->destroy   = effect_destroy_shake;
	pshake->status  = effect_status_shake;
	pshake->m_status = EFFECT_INIT;
	pshake->m_type = IMAGE_EFFECT_SHAKE;
	pshake->m_effect_fps = DEFAULT_FPS;
	pshake->m_ptimer = timer_create();
	//计算一些参数
	return (image_effect_p)pshake;
}

//////////////////////////////////////////////////////////////////////////
//模糊效果
//////////////////////////////////////////////////////////////////////////
typedef struct {
	int m_type;
	int m_status;
	int m_effect_fps;
	int m_timeticks;
	nge_timer* m_ptimer;
	//公共函数
	effect_draw draw;
	effect_setparam set_param;
	effect_getparam get_param;
	effect_destroy  destroy;
	effect_getstatus status;
	//blur privated
	int m_src_blur;
	int m_des_blur;
	float m_delta;
	float m_mins;
	int m_lastticks;
	int m_currentticks;
	int m_passtime;
	int m_mask;
	int m_lasts;
	int m_optimization;
	image_p m_image;
}image_effect_blur_t,*image_effect_blur_p;
static float effect_setparam_blur(image_effect_p effector,float param,int flags)
{
	image_effect_blur_p pfblur = (image_effect_blur_p)effector;
	float ret = 0;
	switch(flags)
	{
	case SET_EFFECT_FPS:
		if(param>0){
			ret = (float)pfblur->m_effect_fps;
			pfblur->m_effect_fps = (int)param;
		}
		break;
	case SET_EFFECT_TIMETICKS:
		if(param>0){
			ret = (float)pfblur->m_timeticks;
			pfblur->m_timeticks = (int)param;
		}
		break;
	case SET_EFFECT_BLUR_SRC:
		if(param>=0&&param<=255){
			pfblur->m_src_blur = (int)param;
			ret = (float)pfblur->m_src_blur;
		}
		break;
	case SET_EFFECT_BLUR_DES:
		if(param>=0&&param<=255){
			pfblur->m_des_blur = (int)param;
			ret = (float)pfblur->m_des_blur;
		}
		break;
	case SET_EFFECT_BLUR_OPTIMIZATION:
		if(param>=0&&param<=3)
		{
			pfblur->m_optimization = (int)param;
			ret = (float)pfblur->m_optimization;
		}
	default:
		break;
	}
	return ret;
}
static float effect_getparam_blur(image_effect_p effector,int flags)
{
	image_effect_blur_p pfblur = (image_effect_blur_p)effector;
	float ret = 0;
	switch(flags)
	{
	case SET_EFFECT_FPS:
		ret = (float)pfblur->m_effect_fps;
		break;
	case SET_EFFECT_TIMETICKS:
		ret = (float)pfblur->m_timeticks;
		break;
	case SET_EFFECT_BLUR_SRC:
		ret = (float)pfblur->m_src_blur;
		break;
	case SET_EFFECT_BLUR_DES:
		ret = (float)pfblur->m_des_blur;
		break;
	case SET_EFFECT_BLUR_OPTIMIZATION:
		ret = (float)pfblur->m_optimization;
		break;
	default:
		break;
	}
	return ret;
}

void set_image_pixel(image_p image, int x, int y, uint8 r,uint8 g,uint8 b ,uint8 a)
{
	static uint16* p16 = 0;
	static uint32* p32 = 0;
	   if(x<(int)image->texw && y<(int)image->texh && x >= 0 && y >= 0)
	{
		switch(image->dtype)
		{
		case DISPLAY_PIXEL_FORMAT_4444:
			p16 = (uint16*)image->data;
			p16[y*image->texw+x] = MAKE_RGBA_4444(r,g,b,a);
			break;
		case DISPLAY_PIXEL_FORMAT_8888:
			p32 = (uint32*)image->data;
			p32[y*image->texw+x] = MAKE_RGBA_8888(r,g,b,a);
			break;
		case DISPLAY_PIXEL_FORMAT_5551:
			p16 = (uint16*)image->data;
			p16[y*image->texw+x] = MAKE_RGBA_5551(r,g,b,a);
			break;
		case DISPLAY_PIXEL_FORMAT_565:
			p16 = (uint16*)image->data;
			p16[y*image->texw+x] = MAKE_RGBA_565(r,g,b,a);
			break;
		}
	}
}

#define CAL_AND_SET_BLUR(type, bit, has_alpha, step)                    \
	for(bx=(((int)(ss+x))<0 ? 0 :ss);bx<=se &&                          \
			(bx + x) < pimg->w;bx+=step)                                \
	{                                                                   \
		for(by=(((int)(ss+y))<0 ? 0 :ss);by<=se &&                      \
				(by + y) < pimg->h;by+=step)                            \
		{                                                               \
			GET_RGBA_##type ((*(p##bit + by * pimg->texw + bx)),sr,sb,sg,sa); \
			tn ++;                                                      \
			a += sa;                                                    \
			b += sb;                                                    \
			r += sr;                                                    \
			g += sg;                                                    \
		}                                                               \
	}                                                                   \
	if(tn==0)                                                           \
		break;                                                          \
	*(pdes##bit + x) = CreateColor((uint8)(r/tn), (uint8)(g/tn), (uint8)(b/tn), has_alpha ? (uint8)(a/tn) :(uint8)(0xff/tn), pfblur->m_image->dtype)

static void effect_draw_blur_op0(image_effect_blur_p pfblur,int s,uint32* pdes32,uint16* pdes16,uint32* p32,uint16* p16, image_p pimg)
{
	// 不优化速度方式绘制模糊效果
	int tn,a,b,r,g,bx,by,ss,se;
	static char sr,sb,sg,sa;
	uint32 x,y;
	ss = -s/2;
	se = s/2;
	if(se==0)
		se = 1;
	for(y=0;y<pimg->h;y++)
	{
		if(pimg->dtype == DISPLAY_PIXEL_FORMAT_8888)
		{
			p32 = ((uint32*)pimg->data) + pimg->texw * y;
			pdes32 = ((uint32*)pfblur->m_image->data) + pfblur->m_image->texw * y;
		}
		else
		{
			p16 = ((uint16*)pimg->data) + pimg->texw * y;
			pdes16 = ((uint16*)pfblur->m_image->data) + pimg->texw * y;
		}

		for(x=0;x<pimg->w;x++)
		{
			switch(pimg->dtype)
			{
			case DISPLAY_PIXEL_FORMAT_8888:
				a = 0;b = 0;r = 0;g = 0; tn = 0;
				CAL_AND_SET_BLUR(8888, 32, 1, 1);
				break;
			case DISPLAY_PIXEL_FORMAT_4444:
				a = 0;b = 0;r = 0;g = 0; tn = 0;
				CAL_AND_SET_BLUR(4444, 16, 1, 1);
				break;
			case DISPLAY_PIXEL_FORMAT_5551:
				a = 1;b = 0;r = 0;g = 0; tn = 0;
				CAL_AND_SET_BLUR(5551, 16, 0, 1);
				break;
			case DISPLAY_PIXEL_FORMAT_565:
				a = 0;b = 0;r = 0;g = 0; tn = 0;
				CAL_AND_SET_BLUR(565, 16, 0, 1);
				break;
			default:
				printf("pixel no supper\n");
				break;
			}
			if(pimg->dtype == DISPLAY_PIXEL_FORMAT_8888)
			{
				p32++;
			}
			else
			{
				p16++;
			}
		}
	}
}
static void effect_draw_blur_op1(image_effect_blur_p pfblur,int s,uint32* pdes32,uint16* pdes16,uint32* p32,uint16* p16, image_p pimg)
{
	// 以四关键点方式绘制模糊效果
	int tn,a,b,r,g,bx,by,step,ss,se;
	static char sr,sb,sg,sa;
	uint32 x,y;
	step =((s/2)>=1 ? s/2 : 1);
	ss = -s/2;
	se = s/2;
	if(se==0)
		se = 1;
	for(y=0;y<pimg->h;y++)
	{
		if(pimg->dtype == DISPLAY_PIXEL_FORMAT_8888)
		{
			p32 = ((uint32*)pimg->data) + pimg->texw * y;
			pdes32 = ((uint32*)pfblur->m_image->data) + pfblur->m_image->texw * y;
		}
		else
		{
			p16 = ((uint16*)pimg->data) + pimg->texw * y;
			pdes16 = ((uint16*)pfblur->m_image->data) + pimg->texw * y;
		}
		//pfblur->m_image->texid = image_tid++;
		for(x=0;x<pimg->w;x++)
		{
			switch(pimg->dtype)
			{
			case DISPLAY_PIXEL_FORMAT_8888:
				a = 0;b = 0;r = 0;g = 0; tn = 0;
				CAL_AND_SET_BLUR(8888, 32, 1, step);
				break;
			case DISPLAY_PIXEL_FORMAT_4444:
				a = 0;b = 0;r = 0;g = 0; tn = 0;
				CAL_AND_SET_BLUR(4444, 16, 1, step);
				break;
			case DISPLAY_PIXEL_FORMAT_5551:
				a = 1;b = 0;r = 0;g = 0; tn = 0;
				CAL_AND_SET_BLUR(5551, 16, 0, step);
				break;
			case DISPLAY_PIXEL_FORMAT_565:
				a = 0;b = 0;r = 0;g = 0; tn = 0;
				CAL_AND_SET_BLUR(565, 16, 0, step);
				break;
			default:
				printf("pixel no supper\n");
				break;
			}
			if(pimg->dtype == DISPLAY_PIXEL_FORMAT_8888)
			{
				p32++;
			}
			else
			{
				p16++;
			}
		}
	}
}
static uint32 effect_draw_blur_op2_transition(int dtype, int color1,int color2,int bs)
{
	// 参数 b  0 - 255, 0 返回 color1, 255 返回 color2;
	int a,b,r,g;
	static char sa,sb,sr,sg;
	if(bs==0)
		return color1;
	if(bs==255)
		return color2;
	switch(dtype)
	{
	case DISPLAY_PIXEL_FORMAT_8888:
		a = GET_RGBA_A(color1) * (255-bs) / 255;
		b = GET_RGBA_B(color1) * (255-bs) / 255;
		r = GET_RGBA_R(color1) * (255-bs) / 255;
		g = GET_RGBA_G(color1) * (255-bs) / 255;
		a += GET_RGBA_A(color2) * bs / 255;
		b += GET_RGBA_B(color2) * bs / 255;
		r += GET_RGBA_R(color2) * bs / 255;
		g += GET_RGBA_G(color2) * bs / 255;
		return MAKE_RGBA_8888(r,g,b,a);
		break;
	case DISPLAY_PIXEL_FORMAT_4444:
		GET_RGBA_4444(color1,sr,sb,sg,sa);
		a = (int)(sa * (255-bs) / 255.0 + 0.5);
		b = (int)(sb * (255-bs) / 255.0 + 0.5);
		g = (int)(sg * (255-bs) / 255.0 + 0.5);
		r = (int)(sr * (255-bs) / 255.0 + 0.5);
		GET_RGBA_4444(color2,sr,sb,sg,sa);
		a += (int)(sa * bs / 255.0 + 0.5);
		b += (int)(sb * bs / 255.0 + 0.5);
		g += (int)(sg * bs / 255.0 + 0.5);
		r += (int)(sr * bs / 255.0 + 0.5);
		if(a!=15)
		{
			printf("a!=15\n");
		}
		return MAKE_RGBA_4444(r,g,b,a);
		break;
	case DISPLAY_PIXEL_FORMAT_5551:
		GET_RGBA_5551(color1,sr,sb,sg,sa);
		b=(int)(sb* (255-bs) / 255.0 + 0.5);
		g=(int)(sg * (255-bs) / 255.0 + 0.5);
		r=(int)(sr * (255-bs) / 255.0 + 0.5);
		GET_RGBA_5551(color2,sr,sb,sg,sa);
		b+=(int)(sb * bs / 255.0 + 0.5);
		g+=(int)(sg * bs / 255.0 + 0.5);
		r+=(int)(sr * bs / 255.0 + 0.5);
		return MAKE_RGBA_5551(r,g,b,0xff);
		break;
	case DISPLAY_PIXEL_FORMAT_565:
		a = 0;
		GET_RGBA_565(color1,sr,sb,sg,sa);
		b = (int)(sb * (255-bs) / 255.0 + 0.5);
		g = (int)(sg * (255-bs) / 255.0 + 0.5);
		r = (int)(sr * (255-bs) / 255.0 + 0.5);
		GET_RGBA_565(color1,sr,sb,sg,sa);
		b += (int)(sg * bs / 255.0 + 0.5);
		g += (int)(sg * bs / 255.0 + 0.5);
		r += (int)(sr * bs / 255.0 + 0.5);
		return MAKE_RGBA_565(r,g,b,0xff);
		break;
	}
	return 0;
}
static void effect_draw_blur_op2(image_effect_blur_p pfblur,int s,uint32* pdes32,uint16* pdes16,uint32* p32,uint16* p16, image_p pimg)
{
	// 以单关键点差补方式绘制模糊效果
	int by,col,coll,colt,collt;
	uint32 x,y,bx;
	for(y=0;y<pimg->h;y+=s)
	{
		if(pimg->dtype == DISPLAY_PIXEL_FORMAT_8888)
		{
			p32 = ((uint32*)pimg->data) + pimg->texw * y;
			pdes32 = ((uint32*)pfblur->m_image->data) + pfblur->m_image->texw * y;
		}
		else
		{
			p16 = ((uint16*)pimg->data) + pimg->texw * y;
			pdes16 = ((uint16*)pfblur->m_image->data) + pfblur->m_image->texw * y;
		}
		for(x=0;x<pimg->w;x+=s)
		{
			switch(pimg->dtype)
			{
			case DISPLAY_PIXEL_FORMAT_8888:
				// 复制当前点颜色
				col = *(p32 + x);
				*(pdes32 + x) = col;
				if(x>0 && y>0)
				{
					// 产生补色
					coll = *(p32 + x - s);// 左补色
					colt = *(p32 + x - s * pimg->texw);// 上补色
					collt = *(p32 - s * pimg->texw + x - s);// 左上补色
					for(bx=0;bx<s;bx++)
					{
						for(by=0;by<s;by++)
						{
							*(pdes32 - (s-by)*pimg->texw + x - (s - bx)) = //collt;
								effect_draw_blur_op2_transition(pimg->dtype,
									effect_draw_blur_op2_transition(pimg->dtype,collt,colt,bx*255/s),
									effect_draw_blur_op2_transition(pimg->dtype, coll, col,bx*255/s),
								by*255/s);
						}
					}
				}
				break;
			case DISPLAY_PIXEL_FORMAT_4444:
			case DISPLAY_PIXEL_FORMAT_5551:
			case DISPLAY_PIXEL_FORMAT_565:
				// 复制当前点颜色
				col = *(p16 + x);
				*(pdes16 + x) = col;
				if(x>0 && y>0)
				{
					// 产生补色
					coll = *(p16 + x - s);// 左补色
					colt = *(p16 + x - s * pimg->texw);// 上补色
					collt = *(p16 - s * pimg->texw + x - s);// 左上补色
					for(bx=0;bx<s;bx++)
					{
						for(by=0;by<s;by++)
						{
							*(pdes16 - (s-by)*pimg->texw + x - (s - bx)) = //collt;
								effect_draw_blur_op2_transition(pimg->dtype,
									effect_draw_blur_op2_transition(pimg->dtype,collt,colt,bx*255/s),
									effect_draw_blur_op2_transition(pimg->dtype, coll, col,bx*255/s),
								by*255/s);
						}
					}
				}
				break;
			default:
				printf("pixel no supper\n");
				break;
			}
		}
		// 绘制超出的 x
		if(y>0)
		{
			for(bx=0;bx<(pimg->w-(x-s));bx++)
			{
				for(by=0;by<s;by++)
				{
					switch(pimg->dtype)
					{
					case DISPLAY_PIXEL_FORMAT_8888:
						*(pdes32 - (s-by)*pimg->texw + (x-s) + bx)=
							effect_draw_blur_op2_transition(pimg->dtype, colt, col, bx*255/s);
						break;
					case DISPLAY_PIXEL_FORMAT_4444:
					case DISPLAY_PIXEL_FORMAT_5551:
					case DISPLAY_PIXEL_FORMAT_565:
						*(pdes16 - (s-by)*pimg->texw + (x-s) + bx)=
							effect_draw_blur_op2_transition(pimg->dtype, colt, col, bx*255/s);
						break;
					}
				}
			}
		}
	}
	// 绘制超出的 y
	if(pimg->dtype==DISPLAY_PIXEL_FORMAT_8888)
		p32 = ((uint32*)pfblur->m_image->data) + pfblur->m_image->texw * (y-s-1);
	else
		p16 = ((uint16*)pfblur->m_image->data) + pfblur->m_image->texw * (y-s-1);
	for(y-=s;y<pimg->h;y++)
	{
		if(pimg->dtype == DISPLAY_PIXEL_FORMAT_8888)
		{
			pdes32 = ((uint32*)pfblur->m_image->data) + pfblur->m_image->texw * y;
		}
		else
		{
			pdes16 = ((uint16*)pfblur->m_image->data) + pimg->texw * y;
		}
		for(x=0;x<pimg->w;x++)
		{
			switch(pimg->dtype)
			{
			case DISPLAY_PIXEL_FORMAT_8888:
				*(pdes32 + x)= *(p32 + x);
				break;
			case DISPLAY_PIXEL_FORMAT_4444:
			case DISPLAY_PIXEL_FORMAT_5551:
			case DISPLAY_PIXEL_FORMAT_565:
				*(pdes16 + x)= *(p16 + x);
				break;
			}
		}
	}
}
static void effect_draw_blur_op3(image_effect_blur_p pfblur,int s,uint32* pdes32,uint16* pdes16,uint32* p32,uint16* p16, image_p pimg)
{
	// 以单关键点方式绘制模糊效果
	uint32 x,y,bx,by,col;

	for(y=0;y<pimg->h;y+=s)
	{
		if(pimg->dtype == DISPLAY_PIXEL_FORMAT_8888)
		{
			p32 = ((uint32*)pimg->data) + pimg->texw * y;
			pdes32 = ((uint32*)pfblur->m_image->data) + pfblur->m_image->texw * y;
		}
		else
		{
			p16 = ((uint16*)pimg->data) + pimg->texw * y;
			pdes16 = ((uint16*)pfblur->m_image->data) + pimg->texw * y;
		}
		for(x=0;x<pimg->w;x+=s)
		{
			switch(pimg->dtype)
			{
			case DISPLAY_PIXEL_FORMAT_8888:
				// 复制当前点颜色
				col = *(p32 + x);
				*(pdes32 + x) = col;
				if(x>0 && y>0)
				{
					for(bx=0;bx<s;bx++)
					{
						for(by=0;by<s;by++)
						{
							*(pdes32 - (s-by)*pimg->texw + x - (s - bx)) = col;
						}
					}
				}
				break;
			case DISPLAY_PIXEL_FORMAT_4444:
			case DISPLAY_PIXEL_FORMAT_5551:
			case DISPLAY_PIXEL_FORMAT_565:
				// 复制当前点颜色
				col = *(p16 + x);
				*(pdes16 + x) = col;
				if(x>0 && y>0)
				{
					for(bx=0;bx<s;bx++)
					{
						for(by=0;by<s;by++)
						{
							*(pdes16 - (s-by)*pimg->texw + x - (s - bx)) = col;
						}
					}
				}
				break;
			default:
				printf("pixel no supper\n");
				break;
			}
		}
		// 绘制超出的 x
		if(y>0)
		{
			for(bx=0;bx<(pimg->w-(x-s));bx++)
			{
				for(by=0;by<s;by++)
				{
					switch(pimg->dtype)
					{
					case DISPLAY_PIXEL_FORMAT_8888:
						*(pdes32 - (s-by)*pimg->texw + (x-s) + bx)= col;
						break;
					case DISPLAY_PIXEL_FORMAT_4444:
					case DISPLAY_PIXEL_FORMAT_5551:
					case DISPLAY_PIXEL_FORMAT_565:
						*(pdes16 - (s-by)*pimg->texw + (x-s) + bx)= col;
						break;
					}
				}
			}
		}
	}
	// 绘制超出的 y
	if(pimg->dtype == DISPLAY_PIXEL_FORMAT_8888)
		p32 = ((uint32*)pfblur->m_image->data) + pfblur->m_image->texw * (y-s);
	else
		p16 = ((uint16*)pfblur->m_image->data) + pfblur->m_image->texw * (y-s);
	for(y-=s;y<pimg->h;y++)
	{
		if(pimg->dtype == DISPLAY_PIXEL_FORMAT_8888)
		{
			pdes32 = ((uint32*)pfblur->m_image->data) + pimg->texw * y;
		}
		else
		{
			pdes16 = ((uint16*)pfblur->m_image->data) + pimg->texw * y;
		}
		for(x=0;x<pimg->w;x++)
		{
			switch(pimg->dtype)
			{
			case DISPLAY_PIXEL_FORMAT_8888:
				*(pdes32 + x)= *(p32 + x);
				break;
			case DISPLAY_PIXEL_FORMAT_4444:
			case DISPLAY_PIXEL_FORMAT_5551:
			case DISPLAY_PIXEL_FORMAT_565:
				*(pdes16 - (s-by)*pimg->texw + (x-s) + bx)=col;
				break;
			}
		}
	}
}
static void effect_draw_blur(image_effect_p effector,image_p pimg,float dx,float dy)
{
	//int s,x,y,bx,by,r,g,b,a,tn;
	int s = 0;
	static uint16* p16 = 0;
	static uint32* p32 = 0;
	static uint16* pdes16 = 0;
	static uint32* pdes32 = 0;
	image_effect_blur_p pfblur = (image_effect_blur_p)effector;
	if(pimg == NULL || pfblur == NULL )
		return;

	switch(pfblur->m_status)
	{
	case EFFECT_INIT:
		pfblur->m_ptimer->start(pfblur->m_ptimer);
		pfblur->m_lastticks = pfblur->m_ptimer->get_ticks(pfblur->m_ptimer);
		pfblur->m_passtime = pfblur->m_lastticks;
		pfblur->m_currentticks = pfblur->m_lastticks;
		pfblur->m_image = image_create(pimg->w,pimg->h,pimg->dtype);
		pfblur->m_image->dontswizzle = 1;
		if(pimg->swizzle == 1){
				unswizzle_swap(pimg);
				pimg->dontswizzle = 1;
		}
		pfblur->m_status = EFFECT_PLAY;
		break;
	case EFFECT_PLAY:
		if(pfblur->m_lastticks!=pfblur->m_currentticks && pfblur->m_currentticks + 1000/pfblur->m_effect_fps > pfblur->m_ptimer->get_ticks(pfblur->m_ptimer))
		{
			break;
		}
		pfblur->m_currentticks = pfblur->m_ptimer->get_ticks(pfblur->m_ptimer);
		if(pfblur->m_ptimer->get_ticks(pfblur->m_ptimer) - pfblur->m_lastticks>pfblur->m_timeticks)
		{
			pfblur->m_status = EFFECT_STOP;
			s = pfblur->m_des_blur;
		}
		else
		{
			if(pfblur->m_des_blur>pfblur->m_src_blur)
			{
				s = (((pfblur->m_ptimer->get_ticks(pfblur->m_ptimer) - pfblur->m_lastticks) * (pfblur->m_des_blur - pfblur->m_src_blur)) / pfblur->m_timeticks) + pfblur->m_src_blur;
			}
			else
			{
				s = pfblur->m_src_blur - (((pfblur->m_ptimer->get_ticks(pfblur->m_ptimer) - pfblur->m_lastticks) * (pfblur->m_src_blur - pfblur->m_des_blur)) / pfblur->m_timeticks);
			}
		}
		if(pfblur->m_lasts>=0 && pfblur->m_lasts == s)
		{
			break;
		}
		pfblur->m_lasts = s;
		if(s==0)
		{
			memcpy(pfblur->m_image->data, pimg->data, pimg->texw * pimg->texh * pimg->bpb);
		}
		else
		{


			if(pimg->dtype == DISPLAY_PIXEL_FORMAT_8888)
			{
				p32 = (uint32*)pimg->data;
				pdes32 = (uint32*)pfblur->m_image->data;
			}
			else
			{
				p16 = (uint16*)pimg->data;
				pdes16 = (uint16*)pfblur->m_image->data;
			}
			//pfblur->m_image->texid = image_tid++;
			pfblur->m_image->modified = 1;
			switch(pfblur->m_optimization)
			{
			case 0:
				effect_draw_blur_op0(pfblur,s,pdes32,pdes16,p32,p16,pimg);
				break;
			case 1:
				effect_draw_blur_op1(pfblur,s,pdes32,pdes16,p32,p16,pimg);
				break;
			case 2:
				effect_draw_blur_op2(pfblur,s,pdes32,pdes16,p32,p16,pimg);
				break;
			case 3:
				effect_draw_blur_op3(pfblur,s,pdes32,pdes16,p32,p16,pimg);
				break;
			}
		}
		break;
	case EFFECT_STOP:
		break;
	}
	ImageToScreen(pfblur->m_image,dx,dy);
}

static void effect_destroy_blur(image_effect_p effector)
{
	image_effect_blur_p pfblur = (image_effect_blur_p)effector;
	if(pfblur){
		if(pfblur->m_ptimer)
			timer_free(pfblur->m_ptimer);
		if(pfblur->m_image)
			image_free(pfblur->m_image);
	}
	SAFE_FREE(pfblur);
}

static int effect_status_blur(image_effect_p effector)
{
	image_effect_blur_p pfblur = (image_effect_blur_p)effector;
	return pfblur->m_status;
}

image_effect_p effect_create_blur(int src_blur,int des_blur,int timeticks,int optimization)
{
	//int tmp = 0;
	image_effect_blur_p pfblur = (image_effect_blur_p)malloc(sizeof(image_effect_blur_t));
	memset(pfblur,0,sizeof(image_effect_blur_p));

	if(src_blur>=0)
		pfblur->m_src_blur = src_blur;
	if(des_blur>=0)
		pfblur->m_des_blur = des_blur;
	if(timeticks>=0)
		pfblur->m_timeticks = timeticks;
	pfblur->get_param = effect_getparam_blur;
	pfblur->set_param = effect_setparam_blur;
	pfblur->draw      = effect_draw_blur;
	pfblur->destroy   = effect_destroy_blur;
	if(optimization<0 || optimization>3)
		pfblur->m_optimization = 1;
	else
		pfblur->m_optimization = optimization;
	pfblur->status  = effect_status_blur;
	pfblur->m_status = EFFECT_INIT;
	pfblur->m_type = IMAGE_EFFECT_BLUR;
	pfblur->m_effect_fps = DEFAULT_FPS;
	pfblur->m_ptimer = timer_create();
	pfblur->m_lasts = -1;
	//计算一些参数
	return (image_effect_p)pfblur;
}


//////////////////////////////////////////////////////////////////////////
//转场效果开始
//////////////////////////////////////////////////////////////////////////
typedef struct {
	int m_type;
	int m_status;
	int m_effect_fps;
	int m_timeticks;
	nge_timer* m_ptimer;
	//公共函数
	effect_draw draw;
	effect_setparam set_param;
	effect_getparam get_param;
	effect_destroy  destroy;
	effect_getstatus status;
	//transitions privated
	image_p m_src_img;
	image_p m_effect_img;
	int m_reversed;
}image_effect_transitions_t,*image_effect_transitions_p;

static int effect_status_transitions(image_effect_p effector)
{
	image_effect_transitions_p pftran = (image_effect_transitions_p)effector;
	return pftran->m_status;
}



static void effect_draw_transitions(image_effect_p effector,image_p pimg,float dx,float dy)
{
	float gf = 0.0f;
	image_effect_transitions_p pftran = (image_effect_transitions_p)effector;
/*	printf("draw begin1\n");*/
	if(pimg == NULL || pftran == NULL )
		return;
/*	printf("draw begin\n");*/
	switch(pftran->m_status)
	{
	case EFFECT_INIT:
		//printf("init begin\n");
		pftran->m_ptimer = timer_create();
		pftran->m_ptimer->start(pftran->m_ptimer);
		pftran->m_status = EFFECT_PLAY;
		//printf("init ok\n");
	case EFFECT_PLAY:

		gf = pftran->m_ptimer->get_ticks(pftran->m_ptimer) / (float)pftran->m_timeticks;

		if(gf>1.0f || gf<0.0f)
		{
			gf = 1.0f;
			pftran->m_status = EFFECT_STOP;
		}

		if(pftran->m_reversed)
		{
			gf = 1.0f - gf;
		}

		if(pftran->m_src_img)
			DrawImage(pftran->m_src_img, 0, 0, 0, 0, dx, dy, 0, 0);

#if defined WIN32 ||defined IPHONEOS || defined(__linux__)

		//printf("GF:%d / %d\n", pftran->m_ptimer->get_ticks(pftran->m_ptimer), pftran->m_timeticks);

		glEnable(GL_ALPHA_TEST);		// 启用透明度测试
		if(pftran->m_reversed)
			glAlphaFunc(GL_GEQUAL, gf);		// 设定透明度测试方法
		else
			glAlphaFunc(GL_LEQUAL, gf);		// 设定透明度测试方法

		glClearStencil(0);				// 设定模版缓冲区清空值
		glClear(GL_STENCIL_BUFFER_BIT);		// 清空模版缓冲区
		glEnable(GL_STENCIL_TEST);		// 启用模版测试
		glStencilFunc(GL_ALWAYS, 1, 0xffffffff);	// 设定模版测试方法
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);	// 设定模版写入方法

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);	// 停用输出
		glDepthMask(GL_FALSE);			// 停用深度缓冲
		if(pftran->m_effect_img)
			DrawImage(pftran->m_effect_img, 0, 0, 0, 0, dx, dy, ___Max(pimg->w, pftran->m_src_img->w), ___Max(pimg->h, pftran->m_src_img->h));

		glDisable(GL_ALPHA_TEST);		// 停用透明度测试

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);	// 启用输出
		glDepthMask(GL_TRUE);			// 启用深度缓冲

		glStencilFunc(GL_EQUAL, 1, 1);	// 设定模版测试方法
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);	// 设定模版写入方法

		if(pimg)
			DrawImage(pimg, 0, 0, 0, 0, dx, dy, ___Max(pimg->w, pftran->m_src_img->w), ___Max(pimg->h, pftran->m_src_img->h));

		glDisable(GL_STENCIL_TEST);		// 停用模版测试

#else

		sceGuEnable(GU_ALPHA_TEST);		// 启用透明度测试
		if(pftran->m_reversed)
			sceGuAlphaFunc(GU_GEQUAL, gf * 255, 0xff);		// 设定透明度测试方法
		else
			sceGuAlphaFunc(GU_LEQUAL, gf * 255, 0xff);		// 设定透明度测试方法

		sceGuClearStencil(0);				// 设定模版缓冲区清空值
		sceGuClear(GU_STENCIL_BUFFER_BIT);		// 清空模版缓冲区
		sceGuEnable(GU_STENCIL_TEST);		// 启用模版测试
		sceGuStencilFunc(GU_ALWAYS, 1, 0xffffffff);	// 设定模版测试方法
		sceGuStencilOp(GU_KEEP, GU_KEEP, GU_REPLACE);	// 设定模版写入方法

		//sceGuDisable(GU_COLOR_BUFFER_BIT);	// 停用输出
		sceGuDepthMask(GU_FALSE);			// 停用深度缓冲

		if(pftran->m_effect_img)
		{
			//nge_print("\nDraw Effect Image");
			DrawImage(pftran->m_effect_img, 0, 0, 0, 0, dx, dy, ___Max(pimg->w, pftran->m_src_img->w), ___Max(pimg->h, pftran->m_src_img->h));
		}

		sceGuDisable(GU_ALPHA_TEST);		// 停用透明度测试

		//sceGuEnable(GU_COLOR_BUFFER_BIT);	// 启用输出
		sceGuDepthMask(GU_TRUE);			// 启用深度缓冲

		sceGuStencilFunc(GU_EQUAL, 1, 1);	// 设定模版测试方法
		sceGuStencilOp(GU_KEEP, GU_KEEP, GU_KEEP);	// 设定模版写入方法

		DrawImage(pimg, 0, 0, 0, 0, dx, dy, ___Max(pimg->w, pftran->m_src_img->w), ___Max(pimg->h, pftran->m_src_img->h));

		sceGuDisable(GU_STENCIL_TEST);		// 停用模版测试


#endif

		break;
	case EFFECT_STOP:
		if(pimg)
			DrawImage(pimg, 0, 0, 0, 0, dx, dy, 0, 0);
		break;
	}
}

static void effect_destroy_transitions(image_effect_p effector)
{
	image_effect_transitions_p pftran = (image_effect_transitions_p)effector;
	if(pftran){
		if(pftran->m_ptimer)
			timer_free(pftran->m_ptimer);
		if(pftran->m_effect_img)
			image_free(pftran->m_effect_img);
	}
	SAFE_FREE(pftran);
}

void HandleR2A(image_p bp)
{
	int x = 0,y = 0;
	int r,a,b,g;
	uint32 *p32 = NULL;
	if(bp->swizzle)
		unswizzle_swap(bp);
	for(y = 0; y < bp->texh; y++)
	{
		p32 = (uint32 *)bp->data + y * bp->texw;
		for(x = 0; x < bp->texw; x ++)
		{
			GET_RGBA_8888(*(p32 + x), r, g, b, a);
			*(p32 + x) = MAKE_RGBA_8888(r,g,b,r);
		}
	}
	bp->modified = 1;
}

image_effect_p effect_create_transitions(image_p effect_img, image_p src_img, int reversed, int timeticks)
{
	image_effect_transitions_p pftran = (image_effect_transitions_p)malloc(sizeof(image_effect_transitions_t));
	memset(pftran,0,sizeof(image_effect_transitions_t));

	if(timeticks>=0)
		pftran->m_timeticks = timeticks;

	pftran->m_src_img = src_img;
	pftran->m_reversed = reversed;
	pftran->m_effect_img = image_clone(effect_img);

	HandleR2A(pftran->m_effect_img);

	pftran->m_status = EFFECT_INIT;
	pftran->m_type = IMAGE_EFFECT_TRANSITIONS;

	pftran->status = effect_status_transitions;
	pftran->draw = effect_draw_transitions;
	pftran->destroy = effect_destroy_transitions;
	pftran->get_param = NULL;
	pftran->set_param = NULL;
	return (image_effect_p)pftran;
}

//////////////////////////////////////////////////////////////////////////
//其他效果
//////////////////////////////////////////////////////////////////////////
