#ifndef NGE_FONT_H_
#define NGE_FONT_H_

#include "nge_common.h"
#include "nge_image.h"

//show flags
#define FONT_SHOW_NORMAL   0
#define FONT_SHOW_SHADOW   1

#define NGE_ENCODING_GBK   0
#define NGE_ENCODING_UTF_8 1

enum{
	SET_ATTR_FIXWIDTH = 0,
	SET_ATTR_BOLD,
	SET_ATTR_MARGIN,
	SET_ATTR_SIZE
};

typedef struct _pfont *PFont;

#ifdef __cplusplus
extern "C"{
#endif

/**
 *设置字体系统当前的编码
 *@param uint8,编码值(如NGE_ENCODING_GBK或NGE_ENCODING_UTF_8)
 */
	NGE_API void NGE_SetFontEncoding(uint8 encoding);

/**
 *创建一个GBK(hzk)font,默认为GBK的点阵
 *@param const char* cname,中文字库通常是GBKxx
 *@param const char* aname,英文字库通常是ASCxx
 *@param int height,高度,例如字库为GBK16,这里就填16
 *@param int disp,显示模式,4种显示模式之一,推荐使用5551,4444
 *@return PFont pfont,返回font指针
 */
	NGE_API PFont create_font_hzk(const char* cname,const char* aname, int height,int disp);

/**
 *从一个内存buf上创建一个GBK(hzk)font,默认为GBK的点阵
 *@param const char *cfbuf,中文字库buf通常是GBKxx
 *@param int csize,上面buf的大小
 *@param const char *afbuf,中文字库cbuf通常是ASCxx
 *@param int asize,上面abuf的大小
 *@param int height,高度,例如字库为GBK16,这里就填16
 *@param int disp,显示模式,4种显示模式之一,推荐使用5551,4444
 *@return PFont pfont,返回font指针
 */
	NGE_API PFont create_font_hzk_buf(const char *cfbuf,int csize,const char* afbuf,int asize, int height,int disp);

/**
 *创建一个ttf(freetype)font
 *@param const char* cname,freetype文件名
 *@param int height,高度像素
 *@param int disp,显示模式,4种显示模式之一,推荐使用8888,4444
 *@return PFont pfont,返回font指针
 */
	NGE_API PFont create_font_freetype(const char* ttfname, int height,int disp);

/**
 *从一个内存buf上创建一个ttf(freetype)font
 *@param const char *cfbuf,freetype的内存buf
 *@param int csize,freetype的内存buf
 *@param int height,高度像素
 *@param int disp,显示模式,4种显示模式之一,推荐使用8888,4444
 *@return PFont pfont,返回font指针
 */
	NGE_API PFont create_font_freetype_buf(const char *cfbuf,int csize, int height,int disp);

/**
 *创建一个nfont
 *@param const char* cname,nfont文件名
 *@param int disp,显示模式,4种显示模式之一,推荐使用8888,4444
 *@return PFont pfont,返回font指针
 */
	NGE_API PFont create_font_nfont(const char* name,int disp);

/**
 *从一个内存buf上创建一个nfont
 *@param const char *cfbuf,nfont的内存buf
 *@param int csize,buf的长度
 *@param int disp,显示模式,4种显示模式之一,推荐使用8888,4444
 *@return PFont pfont,返回font指针
 */
	NGE_API PFont create_font_nfont_buf(const char *nfbuf,int nsize,int disp);

/**
 * 显示一行字到内存图textarea,注意必须用create_image建立内存图textarea
 *@param PFont pf,字体指针,由create_font_xxx创建
 *@param const char* text,显示的文字
 *@param int len,显示文字的长度,用strlen计算
 *@param image_p textarea,内存图textarea
 *@param int dx,拷贝到内存图的x位置坐标
 *@param int dy,拷贝到内存图的y位置坐标
 *@param int showflag,显示标志,目前只有0(正常),1(阴影)
 *@return 无
 */
	NGE_API void font_drawtext(PFont pf,const char* text,int len,image_p textarea,int dx,int dy,int flags);
/**
 *设置字体的属性
 *@param PFont pf,字体指针
 *@param int attr,属性值
 *@param int setflag,需要设置的属性标志
 *@return 无
 */
	NGE_API void font_setattr(PFont pf,int attr,int setflag);
/**
 *释放一个字体
 *@param PFont pf,待释放的字体指针
 *@return 无
 */
	NGE_API void font_destory(PFont pf);
/**
 *设置字体颜色
 *@param PFont pf,字体指针
 *@param uint32 color,字体颜色，颜色值与建立字体的dtype对应,例如建立是16位5551,这里就是MAKE_RGBA5551设置
 *@return int,上一个颜色
 */
	NGE_API int font_setcolor(PFont pf,uint32 color);
/**
 *增强型设置字体颜色,视支持而定
 *@param PFont pf,字体指针
 *@param uint32 color_fg,前景色(字体颜色)，颜色值与建立字体的dtype对应,例如建立是16位5551,这里就是MAKE_RGBA5551设置
 *@param uint32 color_bg,背景色，颜色值与建立字体的dtype对应
 *@param uint32 color_sh,字体阴影色，颜色值与建立字体的dtype对应
 *@return int,无
 */
	NGE_API void font_setcolor_ex(PFont pfont, uint32 color_fg,uint32 color_bg ,uint32 color_sh );
/**
 *得到一段文字的范围
 *@param PFont pf,字体指针
 *@param const void *text,文字指针
 *@param int cc,文字长度
 *@param int *pwidth  宽
 *@param int *pheight 高
 *@param int *pbase   基线
 *@return
 */
	NGE_API void font_textsize(PFont pfont, const void *text, int cc,int *pwidth, int *pheight,int *pbase);

#ifdef __cplusplus
}
#endif


#endif
