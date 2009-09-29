#include "nge_effect.h"

#define FRAMES_PER_SECOND 60

#include <time.h>
float Random_Float(float min, float max)
{
	static int g_seed = time(NULL);
	g_seed=214013*g_seed+2531011;
	return min+(g_seed>>16)*(1.0f/65535.0f)*(max-min);
}

int Random_Int(int min, int max)
{
	static int g_seed = time(NULL);
	g_seed=214013*g_seed+2531011;
	return min+(g_seed ^ g_seed>>15)%(max-min+1);
}

int effect_shake(image_p pimage,float dx,float dy)
{
	float fdx;
	float fdy;
	float delta;
	int flag = Random_Int(0, 1)==0?-1:1;
	
	fdx = dx+Random_Float(0, 5.0)*flag;
	fdy = dy+Random_Float(0, 5.0)*flag;
	ImageToScreen(pimage,fdx,fdy);
	//RenderQuad(pimage,0,0,0,0,fdx,fdy,(480.0+fdx)/480,(272.0+fdy)/272,0,pimage->mask);
	return 1;
}


static int effect_fps = FRAMES_PER_SECOND;

int effect_set_fps(int fps)
{
	int old_fps = effect_fps;
	effect_fps = fps;
	return old_fps;
}

int effect_fade_in (image_p pimage,float dx,float dy,int src_alpha,int des_alpha,float ticks)
{
	static float mins = (des_alpha - src_alpha)/(ticks*effect_fps);
	static float delta = (float)src_alpha; 
	int color;
	int ret = EFFECT_PROGRESS;
	delta =  delta + mins;
	if(delta < des_alpha){
		color = CreateColor(255,255,255,(int)delta,pimage->dtype);
	}
	else{ 
		color = CreateColor(255,255,255,(int)des_alpha,pimage->dtype);
		ret = EFFECT_DONE;
	}
	RenderQuad(pimage,0,0,0,0,dx,dy,1.0,1.0,0,color);
	return ret;
}


int effect_fade_out(image_p pimage,float dx,float dy,int src_alpha,int des_alpha,float ticks)
{
	static float mins = (src_alpha-des_alpha)/(ticks*effect_fps);
	static float delta = (float)src_alpha; 
	int color;
	int ret = EFFECT_PROGRESS;
	delta =  delta - mins;
	if(delta > des_alpha){
		color = CreateColor(255,255,255,(int)delta,pimage->dtype);
	}
	else {
		color = CreateColor(255,255,255,(int)des_alpha,pimage->dtype);
		ret = EFFECT_DONE;
	}
	RenderQuad(pimage,0,0,0,0,dx,dy,1.0,1.0,0,color);
	return ret;
}

int effect_scale_in(image_p pimage,float dox,float doy,float src_scale,float des_scale,float ticks)
{
	static float mins = (des_scale - src_scale)/(ticks*effect_fps);
	static float delta = (float)src_scale; 
	float dx,dy;
	int ret;

	delta =  (float)(delta + mins);
	if(delta < des_scale){
		ret = EFFECT_PROGRESS;
	}
	else{
		delta = des_scale;
		ret = EFFECT_DONE;
	}
	dx =  (float)(dox-0.5*pimage->w*delta);
	dy =  (float)(doy-0.5*pimage->h*delta);
	RenderQuad(pimage,0,0,0,0,dx,dy,delta,delta,0,pimage->mask);
	return ret;
}

int effect_scale_out(image_p pimage,float dox,float doy,float src_scale,float des_scale,float ticks)
{
	static float mins = (src_scale - des_scale)/(ticks*effect_fps);
	static float delta = (float)src_scale; 
	float dx,dy;
	int ret;
	delta =  (float)(delta - mins);
	if(delta > des_scale){
		ret = EFFECT_PROGRESS;
	}
	else{
		delta = des_scale;
		ret = EFFECT_DONE;
	}
	dx =  (float)(dox-0.5*pimage->w*delta);
	dy =  (float)(doy-0.5*pimage->h*delta);
	RenderQuad(pimage,0,0,0,0,dx,dy,delta,delta,0,pimage->mask);
	return ret;
}