#ifndef NGE_FONT_H_
#define NGE_FONT_H_

#include "nge_common.h"
#include "nge_image.h"

#ifdef __cplusplus
extern "C"{
#endif

//encoding flags
#define ENCODING_ASCII   0
#define ENCODING_UNICODE 1
#define ENCODING_GBK     2
//type flags
#define FONT_TYPE_HZK      1
#define FONT_TYPE_GBK      2
#define FONT_TYPE_FREETYPE 4
//show flags
#define FONT_SHOW_NORMAL   0
#define FONT_SHOW_SHADOW   1
//freetype private flags
#define FONT_ANTIALIAS 32
#define FONT_KERNING   64
#define FLAGS_FREETYPE_BOLDER 1
#define FLAGS_FREETYPE_NORMAL 0

enum{
	SET_ATTR_FIXWIDTH = 0,
	SET_ATTR_BOLD,
	SET_ATTR_MARGIN,
	SET_ATTR_SIZE
};

typedef struct {
	/**
	* Maximum advance width of any character.
	*/
	int maxwidth;
	/**
	* Height of "most characters" in the font. This does not include any
	* leading (blank space between lines of text).
	* Always equal to (baseline+descent).
	*/
	int height;
	/**
	* The ascent (height above the baseline) of "most characters" in
	* the font.
	*
	* Note: This member variable should be called "ascent", to be
	* consistent with FreeType 2, and also to be internally consistent
	* with the "descent" member.  It has not been renamed because that
	* would break backwards compatibility.  FIXME
	*/
	int baseline;
	/**
	* The descent (height below the baseline) of "most characters" in
	* the font.
	*
	* Should be a POSITIVE number.
	*/
	int descent;
	/**
	* Maximum height of any character above the baseline.
	*/
	int maxascent;
	/**
	* Maximum height of any character below the baseline.
	* Should be a POSITIVE number.
	*/
	int maxdescent;
	/**
	* The distance between the baselines of two consecutive lines of text.
	* This is usually height plus some font-specific "leading" value.
	*/
	int linespacing;
	/**
	* First character in the font.
	*/
	int firstchar;
	/**
	* Last character in the font.
	*/
	int lastchar;
	/**
	* True (nonzero) if font is fixed width.  In that case, maxwidth
	* gives the width for every character in the font.
	*/
	BOOL fixed;
	/**
	* Table of character advance widths for characters 0-255.
	* Note that fonts can contain characters with codes >255 - in that
	* case this table contains the advance widths for some but not all
	* characters.  Also note that if the font contains kerning
	* information, the advance width of the string "AV" may differ from
	* the sum of the advance widths for the characters 'A' and 'V'.
	*/
	uint8 widths[256];
} FontInfo, *PFontInfo;

struct _fontproc;
typedef struct _pfont{		/* common hdr for all font structures*/
	struct _fontproc*		procs;	/* font-specific rendering routines*/
	int			size;	/* font height in pixels*/
	int			rotation;	/* font rotation*/
	uint32			disp;	/* font attributes: kerning/antialias*/
	/* font-specific rendering data here*/
}TFont,*PFont;

typedef struct _fontproc{
	int		encoding;	/* routines expect this encoding*/
	BOOL	(*GetFontInfo)(PFont pfont, PFontInfo pfontinfo);
	void 	(*GetTextSize)(PFont pfont, const void *text, int cc,int flags, int *pwidth, int *pheight,int *pbase);
	void	(*GetTextBits)(PFont pfont, int ch, const uint8 **retmap,int *pwidth, int *pheight,int *pbase);
	void	(*DestroyFont)(PFont pfont);
	void	(*DrawText)(PFont pfont, image_p pimage, int x, int y,const void *str, int count, int flags);
	void	(*DrawTextShadow)(PFont pfont, image_p pimage, int x, int y,const void *str, int count, int flags);
	uint32  (*SetFontColor)(PFont pfont, uint32 color);
	void    (*SetFontSize)(PFont pfont, int fontsize);
	void    (*SetFontRotation)(PFont pfont, int rot);
	void    (*SetFontAttr)(PFont pfont, int setflags, int clrflags);
	PFont   (*Duplicate) (PFont psrcfont, int fontsize);
	void    (*SetFlags)(PFont pfont,int flags);
	void    (*SetShadowColor)(PFont pfont, uint32 color_fg,uint32 color_bg,uint32 color_sh);
}FontProcs,*PFontProcs;

/**
 *创建一个GBK(hzk)font,默认为GBK的点阵
 *@param const char* cname,中文字库通常是GBKxx
 *@param const char* aname,英文字库通常是ASCxx
 *@param int height,高度,例如字库为GBK16,这里就填16
 *@param int disp,显示模式,4种显示模式之一,推荐使用5551,4444
 *@return PFont pfont,返回font指针
 */
PFont create_font_hzk(const char* cname,const char* aname, int height,int disp);

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
PFont create_font_hzk_buf(const char *cfbuf,int csize,const char* afbuf,int asize, int height,int disp);

/**
 *创建一个ttf(freetype)font
 *@param const char* cname,freetype文件名
 *@param int height,高度像素
 *@param int disp,显示模式,4种显示模式之一,推荐使用8888,4444
 *@return PFont pfont,返回font指针
 */
PFont create_font_freetype(const char* ttfname, int height,int disp);

/**
 *从一个内存buf上创建一个ttf(freetype)font
 *@param const char *cfbuf,freetype的内存buf
 *@param int csize,freetype的内存buf
 *@param int height,高度像素
 *@param int disp,显示模式,4种显示模式之一,推荐使用8888,4444
 *@return PFont pfont,返回font指针
 */
PFont create_font_freetype_buf(const char *cfbuf,int csize, int height,int disp);

/**
 * 显示一行字到内存图textarea,注意必须用create_image建立内存图textarea
 *@param PFont pf,字体指针,由create_font_xxx创建
 *@param const char* text,显示的文字
 *@param int len,显示文字的长度,用strlen计算
 *@param image_p textarea,内存图textarea
 *@param int dx,拷贝到内存图的x位置坐标
 *@param int dy,拷贝到内存图的y位置坐标
 *@param int flag,显示标志,目前只有0(正常),1(阴影)
 *@return 无
 */
void font_drawtext(PFont pf,const char* text,int len,image_p textarea,int dx,int dy,int flags);
/**
 *设置字体的属性
 *@param PFont pf,字体指针
 *@param int attr,属性值
 *@param int setflag,需要设置的属性标志
 *@return 无
 */
void font_setattr(PFont pf,int attr,int setflag);
/**
 *释放一个字体
 *@param PFont pf,待释放的字体指针
 *@return 无
 */
void font_destory(PFont pf);
/**
 *设置字体颜色
 *@param PFont pf,字体指针
 *@param uint32 color,字体颜色，颜色值与建立字体的dtype对应,例如建立是16位5551,这里就是MAKE_RGBA5551设置
 *@return int,上一个颜色
 */
int font_setcolor(PFont pf,uint32 color);
/**
 *增强型设置字体颜色,视支持而定
 *@param PFont pf,字体指针
 *@param uint32 color_fg,前景色(字体颜色)，颜色值与建立字体的dtype对应,例如建立是16位5551,这里就是MAKE_RGBA5551设置
 *@param uint32 color_bg,背景色，颜色值与建立字体的dtype对应
 *@param uint32 color_sh,字体阴影色，颜色值与建立字体的dtype对应
 *@return int,无
 */
void font_setcolor_ex(PFont pfont, uint32 color_fg,uint32 color_bg ,uint32 color_sh );
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
void font_textsize(PFont pfont, const void *text, int cc,int *pwidth, int *pheight,int *pbase);


PFont create_font_nfont_buf(const char *nfbuf,int nsize,int disp);
PFont create_font_nfont(const char* name,int disp);

#ifdef __cplusplus
}
#endif


#endif
