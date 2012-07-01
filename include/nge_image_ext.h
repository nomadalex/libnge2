#ifndef NGE_GRAPHICS_EXT_H
#define NGE_GRAPHICS_EXT_H

#include "nge_common.h"
#include "nge_image.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * 建立 src 图像的灰度版本
 *
 *@param[in] src 源图像
 *@param[in] gray 灰度 0 到 100, 0 为源图像, 100 为纯灰色图像
 *@return image_p,生成的图像的灰度版本
 */
	NGE_API image_p create_gray_image(image_p src, int gray);

/**
 * 建立 src 图像的亮度，饱和度版本
 *
 * @param[in] src 源图像
 * @param[in] saturation 饱合度 -100(灰度图像) 至 100(高彩色比图像)
 * @param[in] brightness 亮度 -100(全黑) 至 100(全白)
 */
	NGE_API image_p create_saturation_brightness_image(image_p src, int saturation, int brightness);

/**
 * 建立 src 图像的其他颜色模式图片
 * @param[in] src 源图像
 * @param[in] dtype 颜色模式,DISPLAY_PIXEL_FORMAT_XXXX的一种
 * @return image_p 生成的图片
 */
	NGE_API image_p image_conv(image_p src, int dtype);

/**
 * 建立 src 图像的缩放图，提供0-4四种缩放滤镜
 * @param[in] src 源图像
 * @param[in] w 缩放的宽
 * @param[in] h 缩放的高
 * @param[in] mode 缩放模式0-4
 * @return image_p 缩放图
 */
	NGE_API image_p image_scale(image_p src, int w, int h,int mode);

/**
 * 将图片进行角度为rot的色相旋转
 * @remark 可能有性能问题,且多次使用会丢失图片质量,请慎用
 * @param[in] pimage 源图像
 * @param[in] rot 色相旋转的角度
 * @return int 成功1,失败0
 */
	NGE_API int image_hue_rotate(image_p pimage, float rot);

/**
 * 填充图片的一块矩形区域
 * @param[in] pimage 源图像
 * @param[in] x 填充矩形左上角距左边距离
 * @param[in] y 填充矩形左上角距上边距离
 * @param[in] w 填充矩形宽
 * @param[in] h 填充矩形高
 * @param[in] color 要填充的颜色
 * @return 无
 */
	NGE_API void image_fillrect(image_p pimage, int x, int y, int w, int h, int color);

#ifdef __cplusplus
}
#endif

#endif
