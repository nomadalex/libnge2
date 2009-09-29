#ifndef NGE_IMAGE_LOAD_H_
#define NGE_IMAGE_LOAD_H_
#include "nge_define.h"

extern uint32 image_tid;

#ifdef __cplusplus
extern "C"{
#endif
/**
 *加载png图片,从文件加载
 *@param const char* filename,图片文件名
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,加载图片填充此结构
 */
image_p image_load_png(const char* filename, int displaymode);
/**
 *加载png图片,从内存加载
 *@param const char* mbuf,内存地址
 *@param int bsize,上述buffer大小
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_png_buf(const char* mbuf,int bsize, int displaymode);
/**
 *加载png图片,从文件指针加载
 *@param int handle,文件指针
 *@param int fsize,上述文件大小
 *@param int autoclose,是否关闭文件,0不关闭,1关闭
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_png_fp(int handle,int fsize, int autoclose,int displaymode);
/**
 * 同image_load_png函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_png_colorkey(const char* filename, int displaymode,int colorkey);
/**
 * 同image_load_png_buf函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_png_colorkey_buf(const char* mbuf, int bsize, int displaymode,int colorkey);
/**
 * 同image_load_png_fp函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_png_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey);
/**
 *加载tga图片,从文件加载
 *@param const char* filename,图片文件名
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_tga(const char* filename, int displaymode);
/**
 *加载tga图片,从内存加载
 *@param const char* mbuf,内存地址
 *@param int bsize,上述buffer大小
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_tga_buf(const char* mbuf,int bsize, int displaymode);
/**
 *加载tga图片,从文件指针加载
 *@param int handle,文件指针
 *@param int fsize,上述文件大小
 *@param int autoclose,是否关闭文件,是否关闭文件,0不关闭,1关闭
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_tga_fp(int handle,int fsize, int autoclose,int displaymode);
/**
 * 同image_load_tga函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_tga_colorkey(const char* filename, int displaymode,int colorkey);
/**
 * 同image_load_tga_buf函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_tga_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey);
/**
 * 同image_load_png_fp函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_tga_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey);
/**
 *加载jpg图片,从文件加载
 *@param const char* filename,图片文件名
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_jpg(const char* filename, int displaymode);
/**
 *加载jpg图片,从内存加载
 *@param const char* mbuf,内存地址
 *@param int bsize,上述buffer大小
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_jpg_buf(const char* mbuf,int bsize, int displaymode);
/**
 *加载jpg图片,从文件指针加载
 *@param int handle,文件指针
 *@param int fsize,上述文件大小
 *@param int autoclose,是否关闭文件,是否关闭文件,0不关闭,1关闭
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_jpg_fp(int handle,int fsize, int autoclose,int displaymode);
/**
 * 同image_load_jpg函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_jpg_colorkey(const char* filename, int displaymode,int colorkey);
/**
 * 同image_load_jpg_buf函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_jpg_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey);
/**
 * 同image_load_jpg_fp函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_jpg_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey);
/**
 *加载bmp图片,从文件加载
 *@param const char* filename,图片文件名
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_bmp(const char* filename, int displaymode);
/**
 *加载bmp图片,从内存加载
 *@param const char* mbuf,内存地址
 *@param int bsize,上述buffer大小
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_bmp_buf(const char* mbuf,int bsize, int displaymode);
/**
 *加载bmp图片,从文件指针加载
 *@param int handle,文件指针
 *@param int fsize,上述文件大小
 *@param int autoclose,是否关闭文件,是否关闭文件,0不关闭,1关闭
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_bmp_fp(int handle,int fsize, int autoclose,int displaymode);
/**
 * 同image_load_bmp函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_bmp_colorkey(const char* filename, int displaymode,int colorkey);
/**
 * 同image_load_bmp_buf函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_bmp_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey);
/**
 * 同image_load_bmp_fp函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_bmp_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey);

/**
 *加载图片,从文件加载
 *@param const char* filename,图片文件名
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@param int swizzle,是否做优化,填0不做优化,通常填1
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load(const char* filename, int displaymode,int swizzle);
/**
 *加载图片,从内存加载
 *@param const char* mbuf,内存地址
 *@param int bsize,上述buffer大小
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@param int swizzle,是否做优化,填0不做优化,通常填1
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_buf(const char* mbuf,int bsize, int displaymode,int swizzle);
/**
 *加载图片,从文件指针加载
 *@param int handle,文件指针
 *@param int fsize,上述文件大小
 *@param int autoclose,是否关闭文件,是否关闭文件,0不关闭,1关闭
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@param int swizzle,是否做优化,填0不做优化,通常填1
 *@return image_p,返回0(加载失败),加载图片填充此结构
 */
image_p image_load_fp(int handle,int fsize, int autoclose,int displaymode,int swizzle);
/**
 * 同image_load函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_colorkey(const char* filename, int displaymode,int colorkey,int swizzle);
/**
 * 同image_load_buf函数,只是多了一个设置关键色,colorkey用MAKE_RGB设定
 */
image_p image_load_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey,int swizzle);



/**
 *存储png图片,将image_p结构内容存储为png文件
 *@param image_p,image_p结构指针,里面保存为image_t的数据
 *@param const char*,保存文件名
 *@param uint8,是否保存为含alpha通道的图片
 *@return int,返回0(加载失败)或1(加载成功)
 */
int image_save_png(image_p pimage,const char* filename,uint8 alpha);
/**
 *存储tga图片,将image_p结构内容存储为tga文件
 *@param image_p,image_p结构指针,里面保存为image_t的数据
 *@param const char*,保存文件名
 *@param uint8,是否保存为含alpha通道的图片
 *@param uint8,是否采用rle编码压缩
 *@return int,返回0(加载失败)或1(加载成功)
 */
int image_save_tga(image_p pimage,const char* filename,uint8 alpha,uint8 rle);
/**
 *创建一个image结构,显示模式是displaymode
 *@param int w,image的宽
 *@param int h,image的高
 *@param int displaymode,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p pimage,返回image指针
 */
image_p image_create(int w,int h,int displaymode);
/**
 *创建一个image结构,显示模式是displaymode
 *@param int w,image的宽
 *@param int h,image的高
 *@param int color,颜色,为保证兼容性请用MAKE_RGBA_XXXX生成这个颜色
 *@param int displaymode,显示模式,与颜色一致,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return image_p pimage,返回image指针
 */
image_p image_create_ex(int w,int h,int color,int displaymode);
/**
 *释放一个image
 *@param image_p pimage,待释放的image指针
 *@return void,无返回
 */
void image_free(image_p pimage);
/**
 *clone一个image,并把image_p返回一个pimage的深拷贝
 *@param image_p pimage,待clone的image指针
 *@return image_p,pimage的深拷贝
 */
image_p image_clone(image_p pimage);
/**
 *将数据清空,图像变为无色透明
 *@param image_p pimage,待清除的image指针
 *@return void,无返回
 */
void image_clear(image_p pimage);
/**
 *将源image_p src拷贝到image_p des.注意此函数通常只用作兼容其他使用
 *通常直接用DrawImage到screen是硬件加速.
 *@param const image_p src,源内存图
 *@param const image_p des,目的内存图
 *@param uint32 dx,目的内存图的x坐标
 *@param uint32 dy,目的内存图的y坐标
 *@return
 */
void image_to_image(const image_p src,const image_p des,uint32 dx,uint32 dy);
/**
 *将源image_p src的sx,sh,sw,sh拷贝到image_p des.注意此函数通常只用作兼容其他使用
 *通常直接用DrawImage到screen是硬件加速.
 *@param const image_p src,源内存图
 *@param const image_p des,目的内存图
 *@param uint32 sx,源内存图的x坐标
 *@param uint32 sy,源内存图的y坐标
 *@param uint32 sw,源内存图的宽
 *@param uint32 sh,源内存图的高
 *@param uint32 dx,目的内存图的x坐标
 *@param uint32 dy,目的内存图的y坐标
 *@return
 */
void image_to_image_ex(const image_p src,const image_p des,uint32 sx,uint32 sy,uint32 sw,uint32 sh,uint32 dx,uint32 dy);
/**
 *将源image_p src拷贝到image_p des 的alpha混合.注意此函数通常只用作兼容其他使用
 *通常直接用DrawImage到screen是硬件加速.
 *@param const image_p src,源内存图
 *@param const image_p des,目的内存图
 *@param uint32 dx,目的内存图的x坐标
 *@param uint32 dy,目的内存图的y坐标
 *@param int alpha,alpha混合值,0-255(透明->不透明),例如128就是半透明
 *@return
 */
void image_to_image_alpha(const image_p src,const image_p des,uint32 dx,uint32 dy,int alpha);
/**
 *将源image_p src的sx,sh,sw,sh拷贝到image_p des.注意此函数通常只用作兼容其他使用
 *通常直接用DrawImage到screen是硬件加速.
 *@param const image_p src,源内存图
 *@param const image_p des,目的内存图
 *@param uint32 sx,源内存图的x坐标
 *@param uint32 sy,源内存图的y坐标
 *@param uint32 sw,源内存图的宽
 *@param uint32 sh,源内存图的高
 *@param uint32 dx,目的内存图的x坐标
 *@param uint32 dy,目的内存图的y坐标
 *@param int alpha,alpha混合值,0-255(透明->不透明),例如128就是半透明
 *@return
 */
void image_to_image_alpha_ex(const image_p src,const image_p des,uint32 sx,uint32 sy,uint32 sw,uint32 sh,uint32 dx,uint32 dy,int alpha);

/**
 *将原始位图格式数据拷贝到image,考虑到效率问题,只支持同种displaymode,
 *注意此处未加判定,所以需要保证data数据正确性
 *@param void* data,位图数据
 *@param image_p ,image指针,目标image
 *@param int x,拷贝到image上的dx位置
 *@param int y,拷贝到image上的dy位置
 *@param int w,data的w
 *@param int h,data的h
 *@return void,无返回
 */
void rawdata_to_image(void* data,const image_p des,uint32 x,uint32 y,uint32 w,uint32 h);

/**
 * 将图像做水平翻转，图像坐标x不变
 *@param image_p pimage,待翻转图像
 *@return int,成功1,失败0
 */
int image_fliph(image_p pimage);


/**
 * 将图像做垂直翻转，翻转后x轴对称
 *@param image_p pimage,待翻转图像
 *@return int,成功1,失败0
 */
int image_flipv(image_p pimage); 


/**
 *根据颜色分量创建一个dtype对应的色彩
 *@param uint8 r,r颜色分量
 *@param uint8 g,g颜色分量
 *@param uint8 b,b颜色分量
 *@param uint8 a,a颜色分量
 *@param int dtype,显示模式,(DISPLAY_PIXEL_FORMAT_XXX的一种)
 *@return int ,颜色值
 */
int CreateColor(uint8 r,uint8 g,uint8 b,uint8 a,int dtype);



//以下函数为内部使用
int GET_PSM_COLOR_MODE(int dtype);
void swizzle_swap(image_p pimage);
void unswizzle_swap(image_p pimage);

#ifdef __cplusplus
}
#endif

#endif