#ifndef NGE_GRAPHICS_EXT_H
#define NGE_GRAPHICS_EXT_H
#include "nge_graphics.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * 建立 src 图像的灰度版本
 *
 * @param src 源图像
 * @param gray 灰度 0 到 100, 0 为源图像, 100 为纯灰色图像
 */
image_p create_gray_image(image_p src, int gray);


/**
 * 建立 src 图像的亮度，饱和度版本
 *
 * @param src 源图像
 * @param saturation 饱合度 -100(灰度图像) 至 100(高彩色比图像)
 * @param brightness 亮度 -100(全黑) 至 100(全白)
 */
image_p create_saturation_brightness_image(image_p src, int saturation, int brightness);

/**
 * 建立 src 图像的其他颜色模式图片
 * @param src 源图像
 * @param dtype 颜色模式,DISPLAY_PIXEL_FORMAT_XXXX的一种
 * @return image_p 生成的图片
 */
image_p image_conv(image_p src, int dtype);

/**
 * 建立 src 图像的缩放图，提供0-4四种缩放滤镜
 * @param src 源图像
 * @param w 缩放的宽
 * @param h 缩放的高
 * @param mode 缩放模式0-4
 * @return image_p 缩放图
 */
image_p image_scale(image_p src, int w, int h,int mode);

#ifdef __cplusplus
}
#endif

#endif
