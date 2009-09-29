#ifndef NGE_EFFECT_H_
#define NGE_EFFECT_H_
#include "../libnge2.h"

#define EFFECT_DONE     1
#define EFFECT_PROGRESS 0

#ifdef __cplusplus
extern "C"{
#endif
int effect_set_fps(int fps);
int effect_fade_in (image_p pimage,float dx,float dy,int src_alpha,int des_alpha,float ticks);
int effect_fade_out(image_p pimage,float dx,float dy,int src_alpha,int des_alpha,float ticks);
int effect_scale_in(image_p pimage,float dox,float doy,float src_scale,float des_scale,float ticks);
int effect_scale_out(image_p pimage,float dox,float doy,float src_scale,float des_scale,float ticks);
#ifdef __cplusplus
}
#endif

#endif