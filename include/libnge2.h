#ifndef LIBNGE2_H_
#define LIBNGE2_H_
#if defined WIN32 || defined IPHONEOS
#include <SDL.h>
#endif

#ifdef __cplusplus
extern "C"{
#endif

#include "nge_image_load.h"
#include "nge_io_file.h"
#include "nge_debug_log.h"
#include "nge_font.h"
#include "nge_graphics.h"
#include "nge_input_proc.h"
#include "nge_main.h"
#include "nge_misc.h"
#include "nge_timer.h"
#include "audio_interface.h"
#include "movieplay/nge_movie.h"
#include "nge_gif_animation.h"
#include "nge_gif_show.h"
#include "nge_sys_compatible.h"
#include "nge_io_dir.h"
#include "nge_graphics_ext.h"

#ifdef __cplusplus
}
#endif


#if defined WIN32 || defined IPHONEOS
#define INIT_VIDEO SDL_INIT_VIDEO
#define INIT_AUDIO SDL_INIT_AUDIO
#define INIT_ALL   INIT_VIDEO|INIT_AUDIO
#else
#define INIT_VIDEO 1
#define INIT_AUDIO 2
#define INIT_ALL   INIT_VIDEO|INIT_AUDIO
#endif

#define NGE_VERSION "2.0.0"


#ifdef __cplusplus
extern "C"{
#endif
/**
 *NGE初始化函数
 *@param int flags,初始化标志位,INIT_VIDEO(视频),INIT_AUDIO(音频)或者INIT_ALL
 *@return 无
 */
void NGE_Init(int flags);
/**
 *NGE退出函数
 *@return 无
 */
void NGE_Quit();
/**
 *设置屏幕窗口
 *@param const char* winname,窗口名字
 *@param int screen_width,窗口宽
 *@param int screen_height,窗口高
 *@param int screen_bpp,窗口bpp,通常填32
 *@param int screen_full,是否全屏0-窗口,1-全屏
 *@return 
 */
void NGE_SetScreenContext(const char* winname,int screen_width,int screen_height,int screen_bpp,int screen_full);

#ifdef __cplusplus
}
#endif

#endif