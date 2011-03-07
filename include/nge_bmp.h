#ifndef NGE_BMP_H_
#define NGE_BMP_H_
#include "nge_define.h"

#define BI_RGB          0
#define BI_RLE8         1
#define BI_RLE4         2
#define BI_BITFIELDS    3


#pragma pack(push, 1)
typedef struct tagBITMAPCOREHEADER {
	uint32    bcSize;
	uint16    bcWidth;
	uint16    bcHeight;
	uint16    bcPlanes;
	uint16    bcBitCnt;
} BITMAPCOREHEADER, *PBITMAPCOREHEADER;

typedef struct tagBITMAPINFOHEADER { /* bmih */
	uint32 biSize;
	uint32 biWidth;
	uint32 biHeight;
	uint16 biPlanes;
	uint16 biBitCount;
	uint32 biCompression;
	uint32 biSizeImage;
	uint32 biXPelsPerMeter;
	uint32 biYPelsPerMeter;
	uint32 biClrUsed;
	uint32 biClrImportant;
} BITMAPINFOHEADER,*PBITMAPINFOHEADER;


typedef struct tagBITMAPFILEHEADER {
	uint16    bfType;
	uint32    bfSize;
	uint16    bfReserved1;
	uint16    bfReserved2;
	uint32    bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;
#pragma pack(pop)

#endif
