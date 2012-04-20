#ifndef NGE_IMAGE_EFFECT_H_
#define NGE_IMAGE_EFFECT_H_

#include "nge_common.h"
#include "nge_image.h"
#include "nge_timer.h"

//支持的效果器
enum{
	IMAGE_EFFECT_FADEIN = 0,
	IMAGE_EFFECT_FADEOUT,
	IMAGE_EFFECT_SCALEIN,
	IMAGE_EFFECT_SCALEOUT,
	IMAGE_EFFECT_SHAKE,
	IMAGE_EFFECT_BLUR,
	IMAGE_EFFECT_TRANSITIONS
};
//效果器的状态
enum{
	EFFECT_INIT = 0,//<<初始化
    EFFECT_PLAY,    //<<进行中
	EFFECT_STOP     //<<效果器完成
};

//seteffect的参数，分为公共参数的私有参数
//私有参数请自行定义掩码
enum{
	//公共参数
	SET_EFFECT_FPS = 0,
	SET_EFFECT_TIMETICKS,
	//私有参数for fade in/out
    SET_EFFECT_FADE_SRC,
	SET_EFFECT_FADE_DES,
	SET_EFFECT_SHAKE_X,
	SET_EFFECT_SHAKE_Y,
	//私有参数for blur
	SET_EFFECT_BLUR_SRC,
	SET_EFFECT_BLUR_DES,
	SET_EFFECT_BLUR_OPTIMIZATION
};

struct tag_image_effect;

//公共函数draw用于显示
typedef void (*effect_draw)(struct tag_image_effect* effector,image_p pimg,float dx,float dy);
//公共函数setparam用于设置参数,参数用float不用int在于保持精度
typedef float (*effect_setparam)(struct tag_image_effect* effector,float param,int flags);
//公共函数getparam用于取得参数。
typedef float (*effect_getparam)(struct tag_image_effect* effector,int flags);
//公共函数getstatus用于取得效果器的状态。
typedef int (*effect_getstatus)(struct tag_image_effect* effector);
//销毁函数
typedef void (*effect_destroy)(struct tag_image_effect* effector);

//效果器的"基类"，注意私有的必须在这个基类后面加
typedef struct tag_image_effect{
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
}image_effect_t,*image_effect_p;

#ifdef __cplusplus
extern "C"{
#endif

/**
 *创建一个渐入的效果器,渐入是alpha从一个小值变化到一个大值
 *@param int src_alpha,开始的alpha值0-255
 *@param int des_alpha,结束的alpha值0-255
 *@param int timeticks,完成时间以毫秒记,1000是1秒
 *@return image_effect_p,返回效果器的指针
 */
	NGE_API image_effect_p effect_create_fadein(int src_alpha,int des_alpha,int timeticks);

/**
 *创建一个渐出的效果器,渐入是alpha从一个大值变化到一个小值
 *@param int src_alpha,开始的alpha值0-255
 *@param int des_alpha,结束的alpha值0-255
 *@param int timeticks,完成时间以毫秒记,1000是1秒
 *@return image_effect_p,返回效果器的指针
 */
	NGE_API image_effect_p effect_create_fadeout(int src_alpha,int des_alpha,int timeticks);

/**
 *创建一个抖动的效果器
 *@param float shake_x,x方向上的抖动范围
 *@param float shake_y,y方向上的抖动范围
 *@param int timeticks,完成时间以毫秒记,1000是1秒
 *@return image_effect_p,返回效果器的指针
 */
	NGE_API image_effect_p effect_create_shake(float shake_x,float shake_y,int timeticks);

/**
 *创建一个模糊的效果器
 *@param float src_blur,开始时的模糊度 0 为不模糊
 *@param float shake_y,结束时的模糊度 0 为不模糊
 *@param int timeticks,完成时间以毫秒记,1000是1秒
 *@param int optimization,速度优化选项，0为不优化，效果好速度慢，1为一般优化，速度效果一般，2为最大优化，速度最快，但效果最差，一般情况下使用 1
 *@return image_effect_p,返回效果器的指针
 */
	NGE_API image_effect_p effect_create_blur(int src_blur,int des_blur,int timeticks, int optimization);

/**
 *创建一个转场效果器
 *@param image_p effect_img, 效果图，效果器使用此图片的像素亮度产生中间效果
 *@param image_p src_img, 原图
 *@param image_p dst_img, 目标图
 *@param int reversed, 反转效果
 *@param int timeticks, 完成时间以毫秒记,1000是1秒
 *@return image_effect_p,返回效果器的指针
 */
	NGE_API image_effect_p effect_create_transitions(image_p effect_img, image_p src_img, int reversed, int timeticks);

#ifdef __cplusplus
}
#endif

#endif
