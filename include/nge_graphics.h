#ifndef NGE_GRAPHICS_H_
#define NGE_GRAPHICS_H_

#include "nge_common.h"
#include "nge_platform.h"

#ifdef NGE_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "nge_image.h"

#define SCREEN_WIDTH_PSP  480
#define SCREEN_HEIGHT_PSP 272

#define SCREEN_WIDTH_IPHONE  320
#define SCREEN_HEIGHT_IPHONE 480

#define SCREEN_WIDTH_ANDROID  240
#define SCREEN_HEIGHT_ANDROID 320

#ifdef NGE_IPHONEOS
#define	SCREEN_WIDTH  SCREEN_WIDTH_IPHONE
#define	SCREEN_HEIGHT SCREEN_HEIGHT_IPHONE
#elif defined NGE_ANDROID
#define	SCREEN_WIDTH  SCREEN_WIDTH_ANDROID
#define	SCREEN_HEIGHT SCREEN_HEIGHT_ANDROID
#else
#define	SCREEN_WIDTH  SCREEN_WIDTH_PSP
#define	SCREEN_HEIGHT SCREEN_HEIGHT_PSP
#endif

#define SCREEN_BPP    32
#define DEFAULT_FPS 60


//we merge *_Trans to Java like function DrawRegion.

#define INVERTED_AXES	 0x4

//sprite transform type
enum TRANS_TYPE{
	TRANS_NONE		    = 0,	//无翻转
	TRANS_ROT90		    = 5,	//顺时针旋转90
	TRANS_ROT180		= 3,	//顺时针旋转180
	TRANS_ROT270		= 6,	//顺时针旋转270
	TRANS_MIRROR		= 2,	//FLIPX
	TRANS_MIRROR_ROT90	= 7,	//逆时针旋转90 镜像
	TRANS_MIRROR_ROT180	= 1,	//FLIPY
	TRANS_MIRROR_ROT270	= 4,	//逆时针旋转270 镜像
};

enum ANCHOR_TYPE{
	ANCHOR_SOLID		= 0,
	ANCHOR_HCENTER		= 0x1,		//1
	ANCHOR_VCENTER		= 0x2,		//2
	ANCHOR_LEFT		    = 0x4,		//4
	ANCHOR_RIGHT		= 0x8,		//8
	ANCHOR_TOP		    = 0x10,		//16
	ANCHOR_BOTTOM		= 0x20,		//32
	ANCHOR_BASELINE		= 0x40,		//64
};

/* note: bpp and fullscreen are not used on windows at current */
typedef struct {
	char *name;
	int width;
	int height;
	int bpp;
	int fullscreen;

	int ori_width;
	int ori_height;
	float rate_w;
	float rate_h;
}screen_context_t, *screen_context_p;

typedef struct{
	float x;
	float y;
} pointf;

typedef struct
{
	float u, v;
	uint32_t color;
	float x,y,z;
}vertexf;

typedef struct{
	vertexf v[4];
	image_p tex;
}quadf;

typedef struct{
	float top;
	float left;
	float right;
	float bottom;
} rectf;

#ifdef __cplusplus
extern "C"{
#endif

/**
 *得到屏幕信息
 *@return 屏幕信息
 */
	NGE_API screen_context_p GetScreenContext();
/**
 * 初始化函数,由nge2系统调用,用户无须显式调用
 */
	NGE_API void InitGraphics();
/**
 * 结束函数,由nge2系统调用,用户无须显式调用
 */
	NGE_API void FiniGraphics();
/**
 * 屏幕绘制开始函数,所有绘制到屏幕均在BeginScene,EndScene之间调用
 *@param[in] clear 清屏标志，默认为1清屏
 *@return 无
 */
	NGE_API void BeginScene(uint8_t clear);
/**
 * 屏幕绘制结束函数,所有绘制到屏幕均在BeginScene,EndScene之间调用
 *@return 无
 */
	NGE_API void EndScene();

/**
 * 绘制到图片开始函数,所有绘制到图片均在BeginTarget,EndTarget之间调用
 *@param[in] _img 所要绘制到的图片
 *@return 无
 *@warning 如果所要使用的图片需要多次进行绘制，请务必将dontswizzle属性设置为1
 */
	NGE_API BOOL BeginTarget(image_p _img);
/**
 * 绘制到图片结束函数,所有绘制到图片均在BeginTarget,EndTarget之间调用
 *@return 无
 */
	NGE_API void EndTarget();

/**
 * 设置是否显示FPS和调试信息
 *@return 无
 */
	NGE_API void ShowFps();
/**
 * 限制帧数
 *@param[in] fps,目标帧数
 *@return 无
 */
	NGE_API void LimitFps(uint32_t fps);
/**
 * 设置裁剪区域，在目标区域内的才显示，同J2ME的SetClip
 *@param[in] x 目标区域x坐标
 *@param[in] y 目标区域y坐标
 *@param[in] w 目标区域裁剪的width
 *@param[in] h 目标区域裁剪的height
 *@return 无
 */
	NGE_API void SetClip(int x,int y,int w,int h);
/**
 * 重置置裁剪区域，等同于调用SetClip(0,0,SCREEN_WIDTH,SCREEN_HEIGHT)
 *@return 无
 */
	NGE_API void ResetClip();
/**
 * 画线函数1，输入坐标点
 *@param[in] x1 第一个点x坐标
 *@param[in] y1 第一个点y坐标
 *@param[in] x2 第二个点x坐标
 *@param[in] y2 第二个点y坐标
 *@param[in] color 颜色与下面对应，例如下面用的5551，这里就用MAKE_RGBA5551
 *@param[in] dtype 显示模式，与上面的对应
 *@return 无
 */
	NGE_API void DrawLine(float x1, float y1, float x2, float y2, int color,int dtype);
/**
 * 画线函数2，输入坐标的pointf
 *@param[in] p1 第一个点xy坐标
 *@param[in] p2 第二个点xy坐标
 *@param[in] color 颜色与下面对应，例如下面用的5551，这里就用MAKE_RGBA5551
 *@param[in] dtype 显示模式，与上面的对应
 *@return 无
 */
	NGE_API void DrawLineEx(pointf p1,pointf p2, int color,int dtype);
/**
 * 画矩形线框的函数1，输入为位置和宽高
 *@param[in] dx 矩形左上角的x坐标
 *@param[in] dy 矩形左上角的y坐标
 *@param[in] width 矩形的宽
 *@param[in] height 矩形的高
 *@param[in] color 颜色与下面对应，例如下面用的5551，这里就用MAKE_RGBA5551
 *@param[in] dtype 显示模式，与上面的对应
 *@return 无
 */
	NGE_API void DrawRect(float dx, float dy, float width, float height,int color,int dtype);
/**
 * 画矩形线框的函数2，输入为矩形位置坐标
 *@param[in] rect 矩形位置坐标
 *@param[in] color 颜色与下面对应，例如下面用的5551，这里就用MAKE_RGBA5551
 *@param[in] dtype 显示模式，与上面的对应
 *@return 无
 */
	NGE_API void DrawRectEx(rectf rect,int color,int dtype);
/**
 * 画填充矩形的函数1，输入为位置和宽高
 *@param[in] dx 矩形左上角的x坐标
 *@param[in] dy 矩形左上角的y坐标
 *@param[in] width 矩形的宽
 *@param[in] height 矩形的高
 *@param[in] color 颜色与下面对应，例如下面用的5551，这里就用MAKE_RGBA5551
 *@param[in] dtype 显示模式，与上面的对应
 *@return 无
 */
	NGE_API void NGE_FillRect(float dx, float dy, float width, float height,int color,int dtype);
#define FillRect NGE_FillRect
/**
 * 画填充矩形的函数2，输入为矩形位置坐标
 *@param[in] rect 矩形位置坐标
 *@param[in] color 颜色与下面对应，例如下面用的5551，这里就用MAKE_RGBA5551
 *@param[in] dtype 显示模式，与上面的对应
 *@return 无
 */
	NGE_API void FillRectEx(rectf rect,int color,int dtype);
/**
 * 画填充渐变矩形的函数1，输入为位置和宽高
 *@param[in] dx 矩形左上角的x坐标
 *@param[in] dy 矩形左上角的y坐标
 *@param[in] width 矩形的宽
 *@param[in] height 矩形的高
 *@param[in] colors 4个顶点的color,颜色与下面对应，例如下面用的5551，这里就用MAKE_RGBA5551
 *@param[in] dtype 显示模式，与上面的对应
 *@return 无
 */
	NGE_API void FillRectGrad(float dx, float dy, float width, float height,int* colors,int dtype);
/**
 * 画填充矩形的函数2，输入为矩形位置坐标
 *@param[in] rect 矩形位置坐标
 *@param[in] colors 4个顶点的color,颜色与下面对应，例如下面用的5551，这里就用MAKE_RGBA5551
 *@param[in] dtype 显示模式，与上面的对应
 *@return 无
 */
	NGE_API void FillRectGradEx(rectf rect,int* colors,int dtype);
/**
 * 画图函数1，效率依次是ImageToScreen>DrawImage>DrawImageMask>RenderQuad;\n
 * sx,sy,sw,sh,构成待显示的图片范围，例如要显示一张图片200*200的图片的\n
 * 20，20到50宽50高的子图块，这里就填依次20,20,50,50。如果要显示原图,都填上0
 * 即可.都填0是显示0,0,tex->w,tex->h。xscale,yscale是放大缩小因子1是原始比例，\n
 * 如果放大一倍，都填2，缩小1倍填0.5。mask是颜色遮罩，用于对图片进行混色，\n
 * 例如半透明效果等，默认是显示原色，这里用texture->mask即可\n
 * 例子1:将200*200的图片tex显示在屏幕100，0处，并放大一倍显示\n
 * RenderQuad(tex,0,0,0,0,100,0,2,2,0,tex->mask);\n
 * 例子2:将图片tex的32，32开始的64宽,64高的部分显示在40，80处并旋转90度\n
 * RenderQuad(tex,32,32,64,64,40,80,1,1,90,tex->mask);\n
 * 例子3:将图片tex(tex显示模式是8888)的80，80开始的64宽,64高的部分显示在100，100处并半透明显示\n
 * RenderQuad(tex,80,80,64,64,100,100,1,1,0,MAKE_RGBA_8888(255,255,255,128));\n
 *@param[in] texture 图片指针
 *@param[in] sx 图片x坐标
 *@param[in] sy 图片y坐标
 *@param[in] sw 图片宽
 *@param[in] sh 图片高,
 *@param[in] dx 屏幕x坐标
 *@param[in] dy 屏幕y坐标
 *@param[in] xscale 图片x方向放大缩小因子
 *@param[in] yscale 图片y方向放大缩小因子
 *@param[in] angle 旋转角度
 *@param[in] mask 颜色遮罩
 *@return 无
 */
	NGE_API void RenderQuad(image_p texture,float sx ,float sy ,float sw ,float sh ,float dx ,float dy ,float xscale  ,float yscale ,float angle ,int mask);
/**
 * 画图函数2,sw,sh为0是画原图，dw，dh为0是显示sw和dh大小\n
 * 例子1:将200*200的图片tex显示在屏幕100，0处\n
 * DrawImage(tex,0,0,0,0,100,0,0,0);\n
 * 或者用严格方式：DrawImage(tex,0,0,200,200,100,0,200,200);\n
 * @see RenderQuad
 *@param[in] texture 图片指针
 *@param[in] sx 图片x坐标
 *@param[in] sy 图片y坐标
 *@param[in] sw 图片宽
 *@param[in] sh 图片高
 *@param[in] dx 屏幕x坐标
 *@param[in] dy 屏幕y坐标
 *@param[in] dw 屏幕宽
 *@param[in] dh 屏幕高
 *@return 无
 */
	NGE_API void DrawImage(image_p texture,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh);
/**
 * 画图函数3\n
 * 同DrawImage，只是多了个MASK\n
 * @see DrawImage
 *@param[in] tex 图片指针
 *@param[in] sx 图片x坐标
 *@param[in] sy 图片y坐标
 *@param[in] sw 图片宽
 *@param[in] sh 图片高
 *@param[in] dx 屏幕x坐标
 *@param[in] dy 屏幕y坐标
 *@param[in] dw 屏幕宽
 *@param[in] dh 屏幕高
 *@param[in] mask 颜色遮罩
 *@return 无
 */
	NGE_API void DrawImageMask(image_p tex,float sx , float sy, float sw, float sh, float dx, float dy, float dw, float dh,int mask);
/**
 * 画图函数4，最简单的画图函数,将图片画到dx,dy
 *@param[in] texture 图片指针
 *@param[in] dx 屏幕x坐标
 *@param[in] dy 屏幕y坐标
 *@return 无
 */
	NGE_API void ImageToScreen(image_p texture,float dx,float dy);
/**
 * 画图函数5，本函数兼容Java函数的DrawRegion
 *@param[in] tex 图片指针
 *@param[in] sx 图片x坐标
 *@param[in] sy 图片y坐标
 *@param[in] sw 图片宽
 *@param[in] sh 图片高
 *@param[in] transform 变换方式,同Java
 *@param[in] dx 屏幕x坐标
 *@param[in] dy 屏幕y坐标
 *@param[in] anchor 锚点,同Java
 *@return 无
 */
	NGE_API void DrawRegion(image_p	texture, int sx, int sy, int sw, int sh, int transform, int dx, int dy, int anchor);

/**
 *将屏幕内容保存在image_p中
 *@return image_p 返回image_p指针,出错返回NULL
 */
	NGE_API image_p ScreenToImage();
/**
 * 保存一个snapshot图片打文件,这个图片是png格式的
 *@param[in] filename 保存图片文件名
 *@return 无
 */
	NGE_API void ScreenShot(const char* filename);
/**
 *设置屏幕颜色，r,g,b,a
 *@param[in] r 设置r分量(0-255)
 *@param[in] g 设置g分量(0-255)
 *@param[in] b 设置b分量(0-255)
 *@param[in] a 设置a分量(0-255)
 *@return uint32_t 上一个屏幕颜色值,可用宏GET_RGBA_8888分离出各个颜色分量
 */
	NGE_API uint32_t SetScreenColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
/**
 * 设置当前的混合方式\n
 * 混合因子的取值可以是\n
 * BLEND_ZERO: 该颜色不参与混色\n
 * BLEND_ONE: 该颜色完全参与混色\n
 * BLEND_SRC_COLOR: 该因子取源颜色\n
 * BLEND_ONE_MINUS_SRC_COLOR: 用1.0减去源颜色作为因子\n
 * BLEND_SRC_ALPHA: 该因子取目标颜色\n
 * BLEND_ONE_MINUS_SRC_ALPHA: 用1.0减去目标颜色作为因子\n
 * BLEND_DST_ALPHA: 该因子取源颜色的alpha值\n
 * BLEND_ONE_MINUS_DST_ALPHA: 用1.0减去源颜色的alpha值作为因子\n
 * BLEND_DST_COLOR: 该因子取目标颜色的alpha值\n
 * BLEND_ONE_MINUS_DST_COLOR: 用1.0减去目标颜色的alpha值作为因子\n
 * BLEND_SRC_ALPHA_SATURATE: 允许对源颜色不同的颜色分量进行不同的因子计算\n
 *@param[in] src_blend 源因子,即源颜色参与运算的模式
 *@param[in] des_blend 目标因子,即目标颜色参与运算的模式
 */
	NGE_API void SetTexBlend(int src_blend, int des_blend);
/**
 *将混合方式恢复为默认方式
 */
	NGE_API void ResetTexBlend();

/**
 * 在屏幕上画一个点的函数
 *@param[in] x 横坐标
 *@param[in] y 纵坐标
 *@param[in] color 颜色
 *@param[in] dtype 颜色类型
 *@return
 */
	NGE_API void PutPix(float x,float y ,int color,int dtype);

/**
 * 画圆形线框函数
 *@param[in] x 圆心横坐标
 *@param[in] y 圆心纵坐标
 *@param[in] radius 圆半径
 *@param[in] color 颜色
 *@param[in] dtype 颜色类型
 *@return
 */
	NGE_API void DrawCircle(float x, float y, float radius, int color,int dtype);

/**
 * 画实心圆函数
 *@param[in] x 圆心横坐标
 *@param[in] y 圆心纵坐标
 *@param[in] radius 圆半径
 *@param[in] color 颜色
 *@param[in] dtype 颜色类型
 *@return
 */
	NGE_API void FillCircle(float x, float y, float radius, int color,int dtype);

/**
 * 画椭圆线框函数
 *@param[in] x 圆心横坐标
 *@param[in] y 圆心纵坐标
 *@param[in] xradius 椭圆x半径
 *@param[in] yradius 椭圆y半径
 *@param[in] color 颜色
 *@param[in] dtype 颜色类型
 *@return
 */
	NGE_API void DrawEllipse(float x,float y ,float xradius,float yradius,int color,int dtype);

/**
 * 画实心椭圆函数
 *@param[in] x 圆心横坐标
 *@param[in] y 圆心纵坐标
 *@param[in] xradius 椭圆x半径
 *@param[in] yradius 椭圆y半径
 *@param[in] color 颜色
 *@param[in] dtype 颜色类型
 *@return
 */
	NGE_API void FillEllipse(float x,float y ,float xradius,float yradius,int color,int dtype);

/**
 * 画任意多边形线框函数
 * 注意顶点顺序是顺时针方向
 *@param[in] x 顶点横坐标数组
 *@param[in] y 顶点纵坐标数组
 *@param[in] count 顶点个数
 *@param[in] color 颜色
 *@param[in] dtype 颜色类型
 *@return
 */
	NGE_API void DrawPolygon(float* x, float* y, int count, int color,int dtype);

/**
 * 画任意实心多边形函数
 * 注意顶点顺序是顺时针方向
 *@param[in] x 顶点横坐标数组
 *@param[in] y 顶点纵坐标数组
 *@param[in] count 顶点个数
 *@param[in] color 颜色
 *@param[in] dtype 颜色类型
 *@return
 */
	NGE_API void FillPolygon(float* x, float* y, int count, int color,int dtype);

/**
 * 画渐进颜色任意实心多边形函数
 * 注意顶点顺序是顺时针方向
 *@param[in] x 顶点横坐标数组
 *@param[in] y 顶点纵坐标数组
 *@param[in] count 顶点个数
 *@param[in] colors 颜色数组注意与顶点个数相同
 *@param[in] dtype 颜色类型
 *@return
 */
	NGE_API void FillPolygonGrad(float* x, float* y, int count, int* colors,int dtype);


/**
 *  清空显示缓存空间。
 */
	NGE_API void ResetGraphicsCache();

/**
 *  Internal Use Only
 */
	NGE_API void RealRenderQuad(quadf quad);

#ifdef __cplusplus
}
#endif
#endif
