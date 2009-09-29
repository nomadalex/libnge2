#ifndef NGE_GIF_ANIMATION_H_
#define NGE_GIF_ANIMATION_H_
#include "nge_define.h"
#include "nge_image_load.h"
#include "nge_graphics.h"
#include "nge_timer.h"

typedef struct _image_chain{
	image_p pimage;
	uint32  frame_delay;
	struct _image_chain* next;
}image_chains_t,*image_chains_p;

//struct tag_gif_desc_t;
#define GIF_SHOW_INIT 0
#define GIF_SHOW_PLAY 1
#define GIF_SHOW_STOP 2
//typedef image_p (*gif_nextframe)(struct tag_gif_desc_t* pgif);

typedef struct  tag_gif_desc_t{
	image_chains_p gif_image_chains;
	uint32	       gif_delay;
	sint32		   gif_transparent;
	uint32         gif_dispose;
	uint32		   gif_loopcount;
	sint32         gif_rgbtransparent;
	sint32         gif_rgbbackgnd;
	uint32         gif_w;
	uint32         gif_h;
	uint32         gif_texw;
	uint32         gif_texh;
	uint32         gif_framecount;
	/*for display*/
	nge_timer*     gif_inner_timer;
	uint32         gif_last_ticks;
	uint32         gif_current_ticks;
	uint32         gif_show_status;
	image_p        gif_current_frame;
	image_chains_p gif_current_chains;
}gif_desc_t,*gif_desc_p;



#ifdef __cplusplus
extern "C"{
#endif

/**
 * gif动画加载程序,将gif动画帧全部加载到内存,
 * 本函数是预先加载函数,请注意内存的使用量
 *@param const char* filename,gif文件名
 *@param int displaymode,显示模式
 *@param int swizzle,优化标志,通常填1
 *@return gif_desc_p,gif绘图文件句柄,供显示函数调用
 */
gif_desc_p gif_animation_load(const char* filename,int displaymode,int swizzle);

/**
 * 释放一个gif动画资源
 *@param gif_desc_p pgif,由gif_animation_load加载的描述符
 *@return
 */
void gif_animation_free(gif_desc_p pgif);






#ifdef __cplusplus
}
#endif
 
#endif