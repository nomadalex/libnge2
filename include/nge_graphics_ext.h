#ifndef NGE_GRAPHICS_EXT_H
#define NGE_GRAPHICS_EXT_H

#include "nge_define.h"

#include "nge_debug_log.h"

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
#ifdef __cplusplus
}
#endif

#endif
