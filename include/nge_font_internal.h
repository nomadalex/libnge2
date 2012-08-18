/**
 * @file  nge_font_internal.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/05 12:08:10
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _NGE_FONT_INTERNAL_H
#define _NGE_FONT_INTERNAL_H

//type flags
#define FONT_TYPE_HZK      1
#define FONT_TYPE_GBK      2
#define FONT_TYPE_FREETYPE 4

//freetype private flags
#define FONT_ANTIALIAS 32
#define FONT_KERNING   64
#define FLAGS_FREETYPE_BOLD 1
#define FLAGS_FREETYPE_ITALICS (1 << 1)

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
	uint8_t widths[256];
} FontInfo, *PFontInfo;

struct _fontproc{
	BOOL	(*GetFontInfo)(PFont pfont, PFontInfo pfontinfo);
	void 	(*GetTextSize)(PFont pfont, const void *text, int cc,int flags, int *pwidth, int *pheight,int *pbase);
	void	(*GetTextBits)(PFont pfont, int ch, const uint8_t **retmap,int *pwidth, int *pheight,int *pbase);
	void	(*DestroyFont)(PFont pfont);
	void	(*DrawText)(PFont pfont, image_p pimage, int x, int y,const void *str, int count, int flags);
	void	(*DrawTextShadow)(PFont pfont, image_p pimage, int x, int y,const void *str, int count, int flags);
	uint32_t  (*SetFontColor)(PFont pfont, uint32_t color);
	void    (*SetFontSize)(PFont pfont, int fontsize);
	void    (*SetFontRotation)(PFont pfont, int rot);
	void    (*SetFontAttr)(PFont pfont, int setflags, int clrflags);
	PFont   (*Duplicate) (PFont psrcfont, int fontsize);
	void    (*SetFlags)(PFont pfont,int flags);
	void    (*SetShadowColor)(PFont pfont, uint32_t color_fg,uint32_t color_bg,uint32_t color_sh);
};

typedef struct _fontproc FontProcs;
typedef struct _fontproc *PFontProcs;

typedef struct{
	char*       data;
	int         datalen;
} workbuf;

struct _pfont { /* common hdr for all font structures*/
	struct _fontproc* procs; /* font-specific rendering routines*/
	int	size; /* font height in pixels*/
	int	rotation; /* font rotation*/
	uint32_t disp; /* font attributes: kerning/antialias*/
	int flags;
	workbuf	encodingBuf;

	/* font-specific rendering data here*/
};

typedef struct _pfont TFont;

extern uint8_t nge_font_encoding;

#endif /* _NGE_FONT_INTERNAL_H */
