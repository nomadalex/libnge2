#include "nge_gif_animation.h"

#ifdef __cplusplus
extern "C"{
#endif
void GifAnimationToScreen(gif_desc_p pgif,float x,float y);
void RenderGifAnimation(gif_desc_p pgif,float sx ,float sy ,float sw ,float sh ,float dx ,float dy ,float xscale  ,float yscale ,float angle ,int mask);
void DrawGifAnimationMask(gif_desc_p pgif,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,int mask);
void DrawGifAnimation(gif_desc_p pgif,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh);
#ifdef __cplusplus
}
#endif