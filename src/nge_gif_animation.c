#include "nge_gif.h"
#include "gif_lib.h"
#include "stdlib.h"
#include "string.h"

//#define gif_color32(c) MAKE_RGBA_8888(palette->Colors[c].Red,palette->Colors[c].Green,  palette->Colors[c].Blue,255)

#define GIF_TRANSPARENT		0x01
#define GIF_USER_INPUT		0x02
#define GIF_DISPOSE_MASK	0x07
#define GIF_DISPOSE_SHIFT	2

#define GIF_NOT_TRANSPARENT	-1

#define GIF_DISPOSE_NONE	0		// No disposal specified. The decoder is
// not required to take any action.
#define GIF_DISPOSE_LEAVE	1		// Do not dispose. The graphic is to be left
// in place.
#define GIF_DISPOSE_BACKGND	2		// Restore to background color. The area used by the
// graphic must be restored to the background color.

#define GIF_DISPOSE_RESTORE	3		// Restore to previous. The decoder is required to
// restore the area overwritten by the graphic with
// what was there prior to rendering the graphic.

// Netscape 2.0 looping extension block
static GifByteType Netscape20ext[] = "\x0bNETSCAPE2.0";
#define NSEXT_LOOP      0x01        // Loop Count field code

const sint32 InterlacedOffset[] = { 0, 4, 2, 1 }; /* The way Interlaced image should. */
const sint32 InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */

#define MAKEWORD(a,b) ((short)((char)(a))|(((short)(char)(b))<<8))

static void gif_copy_frame_line(GifPixelType * dest, GifPixelType *src,uint32 width,sint32 transparent);
static void gif_fill_frame_line(GifPixelType* pDst, const sint32 rgb, sint32 width);

static void gif_copy_frame_line(GifPixelType * dest, GifPixelType *src,uint32 width,sint32 transparent)
{
	uint32 i;
	GifPixelType b;
	for (i=0;i<width;i++)
	{
		b = *src++;
		if (b != transparent)
			*dest = b;
		dest++;
	}
}

static void gif_fill_frame_line(GifPixelType* pDst, const sint32 rgb, sint32 width)
{
	if (width)
	{
		do
		{
			*pDst++ = (GifPixelType)rgb;
		}
		while (--width);
	}
}

gif_desc_p gif_animation_load(const char* filename,int displaymode,int swizzle)
 {
     uint32 cxScreen,cyScreen,dwRowBytes,dwScreen,m,n;
     sint32 Size,rgbFill,flag,ExtCode,iSubCode,bNetscapeExt;
	 uint32 *pdata32 = NULL;
	 uint16 *pdata16 = NULL;
     GifRecordType RecordType;
     GifByteType *pExtension = NULL;
	 GifByteType bLength;
     GifPixelType *pLine = NULL;
	 GifFileType *GifFileIn = NULL;
	 GifPixelType*  m_current_frame_buf;
	 GifPixelType*  m_next_frame_buf;
	 ColorMapObject* ColorMap = NULL;
	 GifPixelType* Gifdata = NULL;
	 GifWord left,top,Width,Height,i,j;
	 gif_desc_p  pgif = NULL; 
	 image_chains_p ptail = NULL,pitem = NULL;
	 image_p pimage = NULL;
	 
	 GifFileIn =  DGifOpenFileName(filename);
	 pgif = (gif_desc_p)malloc(sizeof(gif_desc_t));
	 if(GifFileIn == NULL || pgif == NULL)
		 return NULL;
	 
	 memset(pgif,0,sizeof(gif_desc_t));
	 pgif->gif_inner_timer = timer_create();
	 pgif->gif_delay = 10;
	 pgif->gif_dispose = 0;
	 pgif->gif_transparent = GIF_NOT_TRANSPARENT;
       
     cxScreen =  GifFileIn->SWidth;
     cyScreen =  GifFileIn->SHeight;
	 Size = cxScreen*cyScreen*sizeof(GifPixelType);
	 m_current_frame_buf = (GifPixelType*)malloc(Size);
	 memset(m_current_frame_buf,0,Size);
	 m_next_frame_buf =    (GifPixelType*)malloc(Size);
	 memset(m_next_frame_buf, GifFileIn->SBackGroundColor,Size);
	 if (GifFileIn->SColorMap){
		 pgif->gif_rgbtransparent=pgif->gif_rgbbackgnd = GifFileIn->SBackGroundColor;
	 }
	 pgif->gif_w = GifFileIn->SWidth;
	 pgif->gif_h = GifFileIn->SHeight;
	 pgif->gif_texw = roundpower2(pgif->gif_w);
     pgif->gif_texh = roundpower2(pgif->gif_h);
     dwRowBytes = cxScreen * sizeof(GifPixelType);
#define XYOFFSET(x,y)   ((y) * dwRowBytes + (x) * sizeof(GifPixelType))
      dwScreen = dwRowBytes * cyScreen;
      do {
         if (DGifGetRecordType(GifFileIn, &RecordType) == GIF_ERROR)
         {
             break;
         }
         switch (RecordType)
         {
         case IMAGE_DESC_RECORD_TYPE:
             if (DGifGetImageDesc(GifFileIn) != GIF_ERROR)
             {
                 left= GifFileIn->Image.Left;
                 top = GifFileIn->Image.Top;
                 Width = GifFileIn->Image.Width;
                 Height = GifFileIn->Image.Height;
 
                 pLine = (GifPixelType*)malloc((uint16)(Width * sizeof(GifPixelType)));
				 memset(pLine,0,Width * sizeof(GifPixelType));
                 pgif->gif_framecount++;
                 memcpy(m_current_frame_buf, m_next_frame_buf, dwScreen);
 
                 if (GifFileIn->Image.Interlace)
                 {
                     /* Need to perform 4 passes on the images: */
                     for (i = 0; i < 4; i++)
                     {
                         for (j = top + InterlacedOffset[i]; j < top + Height;j += InterlacedJumps[i])
                         {
                             if (DGifGetLine(GifFileIn, pLine,Width) == GIF_ERROR)
                             {
                                 SAFE_FREE(pLine);
                                 return 0;
                             }
                             gif_copy_frame_line(m_current_frame_buf+XYOFFSET(left,j),pLine,Width,pgif->gif_transparent);
 
                         }
                     }
 
                 }
                 else
                 {
                     for (i = 0; i < Height; i++)
                     {
 
                         if (DGifGetLine(GifFileIn, pLine,Width) == GIF_ERROR)
                         {
                                 SAFE_FREE(pLine);
                                 return 0;
                         }
                         gif_copy_frame_line(m_current_frame_buf+XYOFFSET(left,(top+i)),pLine,Width,pgif->gif_transparent);
 
                     }
 
                 }
				 if(displaymode == DISPLAY_PIXEL_FORMAT_8888){
					 pimage = image_create(pgif->gif_w,pgif->gif_h,DISPLAY_PIXEL_FORMAT_8888);
					 ColorMap = (GifFileIn->Image.ColorMap? GifFileIn->Image.ColorMap: GifFileIn->SColorMap);
					 pdata32 = (uint32*)pimage->data;
					 Gifdata = m_current_frame_buf;
					 for(m = 0; m < pgif->gif_h;m++ ){
						 for(n = 0;n <pgif->gif_w;n++){
							 if(Gifdata[m*pgif->gif_w+n]!=pgif->gif_transparent)	
								 pdata32[m*pimage->texw+n]= MAKE_RGBA_8888(ColorMap->Colors[Gifdata[m*pgif->gif_w+n]].Red,ColorMap->Colors[Gifdata[m*pgif->gif_w+n]].Green,ColorMap->Colors[Gifdata[m*pgif->gif_w+n]].Blue,0xff) ;
						 } 
					 }
				 }
				 else{
					pimage = image_create(pgif->gif_w,pgif->gif_h,displaymode);
					 ColorMap = (GifFileIn->Image.ColorMap? GifFileIn->Image.ColorMap: GifFileIn->SColorMap);
					 pdata16 = (uint16*)pimage->data;
					 Gifdata = m_current_frame_buf;
					 for(m = 0; m < pgif->gif_h;m++ ){
						 for(n = 0;n <pgif->gif_w;n++){
							 if(Gifdata[m*pgif->gif_w+n]!=pgif->gif_transparent)	
								 pdata16[m*pimage->texw+n]= CreateColor(ColorMap->Colors[Gifdata[m*pgif->gif_w+n]].Red,ColorMap->Colors[Gifdata[m*pgif->gif_w+n]].Green,ColorMap->Colors[Gifdata[m*pgif->gif_w+n]].Blue,0xff,displaymode) ;
						 } 
					 }
				 }
				 pitem = (image_chains_p)malloc(sizeof(image_chains_t));
				 memset(pitem,0,sizeof(image_chains_t));
				 pitem->pimage = pimage;
				 pitem->frame_delay = pgif->gif_delay;
				 if(pgif->gif_image_chains == NULL){
					 pgif->gif_image_chains = pitem;
					 ptail = pitem;
				 }
				 ptail->next = pitem;
				 ptail = pitem;		 
                 if (pgif->gif_dispose == GIF_DISPOSE_BACKGND)
                 {
 
                     // Clear next image to background index
                     // Note: if transparent restore to transparent color (else use GIF background color)
                     rgbFill = (pgif->gif_transparent == GIF_NOT_TRANSPARENT) ? pgif->gif_rgbbackgnd : pgif->gif_rgbtransparent;
				
                     for (i = 0; i < Height; ++i)
                         gif_fill_frame_line(m_next_frame_buf + XYOFFSET(left,top+i), rgbFill, Width);
                 }
                 else if (pgif->gif_dispose != GIF_DISPOSE_RESTORE)
                 {
                     // Copy current -> next (Update)
                     memcpy(m_next_frame_buf, m_current_frame_buf, dwScreen);
                 }
                 pgif->gif_dispose = 0;
                 SAFE_FREE(pLine);
                 
             }
             break;
         case EXTENSION_RECORD_TYPE:
         {
             if (DGifGetExtension(GifFileIn, &ExtCode, &pExtension) == GIF_ERROR)
             {
                  return 0;
             }
             switch (ExtCode)
             {
             case COMMENT_EXT_FUNC_CODE:
                  break;
             case GRAPHICS_EXT_FUNC_CODE:
             {
                 flag = pExtension[1];
                 pgif->gif_delay  = MAKEWORD(pExtension[2], pExtension[3]);
				 if(pgif->gif_delay == 0) 
					pgif->gif_delay = 10;
				 //pgif->gif_delay = 13;
				 //printf("pgif->delay=%d\n",pgif->gif_delay);
                 pgif->gif_transparent = (flag & GIF_TRANSPARENT) ? pExtension[4] : GIF_NOT_TRANSPARENT;
                 pgif->gif_dispose = (flag >> GIF_DISPOSE_SHIFT) & GIF_DISPOSE_MASK;
                 break;
             }
             case PLAINTEXT_EXT_FUNC_CODE:
                  break;
             case APPLICATION_EXT_FUNC_CODE:
             {
                 if (memcmp(pExtension, Netscape20ext, Netscape20ext[0]) == 0)
                 {
                      bNetscapeExt = TRUE;
                 }
                 break;
             }
             default:
 
                 break;
             }
       
             do
             {
                 if (DGifGetExtensionNext(GifFileIn, &pExtension) == GIF_ERROR)
                 {
     
                     return 0;
                 }
                 // Process Netscape 2.0 extension (GIF looping)
                 if (pExtension && bNetscapeExt)
                 {
                     bLength = pExtension[0];
                     iSubCode = pExtension[1] & 0x07;
                     if (bLength == 3 && iSubCode == NSEXT_LOOP)
                     {
                         pgif->gif_loopcount = MAKEWORD(pExtension[2], pExtension[3]) - 1;
					 }
                 }
             }while (pExtension);
             break;
         }
         case TERMINATE_RECORD_TYPE:
             break;
         default:                     // Should be trapped by DGifGetRecordType
             break;
         }
     }
     while (RecordType != TERMINATE_RECORD_TYPE);
     return pgif;
 }
 
void gif_animation_free(gif_desc_p pgif)
{
	image_chains_p pchain = NULL;
	image_chains_p pnext = NULL;
	if(pgif == NULL)
		return;
	for (pchain = pgif->gif_image_chains;pchain;pchain = pnext){
		if(pchain->pimage)
			image_free(pchain->pimage);
		pnext = pchain->next;
		SAFE_FREE(pchain);
	}
	SAFE_FREE(pgif);
}

