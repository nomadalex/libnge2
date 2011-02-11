#ifdef __cplusplus
extern "C"{
#endif
#include "png.h"
#include "jpeglib.h"
#include "nge_tga.h"
#include "nge_bmp.h"
#include "nge_rle.h"
#include "nge_image_load.h"
#include "nge_debug_log.h"
#include "nge_io_file.h"
#ifdef __cplusplus
}
#endif

uint32 image_tid = 0;


#if defined WIN32 || defined IPHONEOS
//win32 and iphone
void swizzle_swap(image_p pimage)
{};
void unswizzle_swap(image_p pimage)
{};
#else
//psp
static void swizzle_fast(uint8* out, const uint8* in, unsigned int width, unsigned int height)
{
	unsigned int blockx, blocky;
	unsigned int j;
	
	unsigned int width_blocks = (width / 16);
	unsigned int height_blocks = (height / 8);
	
	unsigned int src_pitch = (width-16)/4;
	unsigned int src_row = width * 8;
	const uint8 *xsrc;
	const uint32 *src;
	const uint8 *ysrc = in;
	uint32 *dst = (uint32*)out;
	
	for (blocky = 0; blocky < height_blocks; ++blocky)
	{
		xsrc = ysrc;
		for (blockx = 0; blockx < width_blocks; ++blockx)
		{
			src = (uint32*)xsrc;
			for (j = 0; j < 8; ++j)
			{
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				src += src_pitch;
			}
			xsrc += 16;
		}
		ysrc += src_row;
	}
}

//Thanks to Raphael 
static void unswizzle_fast(const uint8* out, const uint8* in, const int width, const int height)
{
	int blockx, blocky;
	int j;
	
	int width_blocks = (width / 16);
	int height_blocks = (height / 8);
	
	int dst_pitch = (width-16)/4;
	int dst_row = width * 8;
	
	uint32* src = (uint32*)in;
	uint8* ydst = (uint8*)out;
	sceKernelDcacheWritebackAll();
	for (blocky = 0; blocky < height_blocks; ++blocky)
	{
		uint8* xdst = ydst;
		for (blockx = 0; blockx < width_blocks; ++blockx)
		{
			uint32* dst;
			if ((uint32)out <= 0x04200000)
				dst = (uint32*)((uint32)xdst | 0x40000000);
			else
				dst = (uint32*)xdst;
			for (j = 0; j < 8; ++j)
			{
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				dst += dst_pitch;
			}
			xdst += 16;
		}
		ydst += dst_row;
	}
}


void swizzle_swap(image_p pimage)
{
	int bsize = pimage->texw*pimage->texh*pimage->bpb;
	uint8* buffer = (uint8*)malloc(bsize);
	memset(buffer,0,bsize);
	swizzle_fast(buffer,pimage->data,pimage->texw*pimage->bpb,pimage->texh);
	SAFE_FREE(pimage->data);
	pimage->data = buffer;
	pimage->swizzle = 1;
}

void unswizzle_swap(image_p pimage)
{
	int bsize = pimage->texw*pimage->texh*pimage->bpb;
	uint8* buffer = (uint8*)malloc(bsize);
	memset(buffer,0,bsize);
	unswizzle_fast(buffer,pimage->data,pimage->texw*pimage->bpb,pimage->texh);
	SAFE_FREE(pimage->data);
	pimage->data = buffer;
	pimage->swizzle = 0;
}

#endif


int CreateColor(uint8 r,uint8 g,uint8 b,uint8 a,int dtype)
{
	int color = 0;
	switch(dtype) 
	{
		case DISPLAY_PIXEL_FORMAT_565:
			color = MAKE_RGBA_565(r,g,b,a);
			break;
		case DISPLAY_PIXEL_FORMAT_5551:
			color = MAKE_RGBA_5551(r,g,b,a);
			break;
		case DISPLAY_PIXEL_FORMAT_4444:
			color = MAKE_RGBA_4444(r,g,b,a);
			break;
		case DISPLAY_PIXEL_FORMAT_8888:
			color = MAKE_RGBA_8888(r,g,b,a);
			break;
		default:
			color = MAKE_RGBA_8888(r,g,b,a);
	}
	return color;
}



int GET_PSM_COLOR_MODE(int dtype)
{
	int psm = 0;
	switch(dtype) 
	{
		case DISPLAY_PIXEL_FORMAT_565:
			psm = PSM_565;
			break;
		case DISPLAY_PIXEL_FORMAT_5551:
			psm = PSM_5551;
			break;
		case DISPLAY_PIXEL_FORMAT_4444:
			psm = PSM_4444;
			break;
		case DISPLAY_PIXEL_FORMAT_8888:
			psm = PSM_8888;
			break;
		default:
			psm = PSM_8888;
	}
	return psm;
}


static int roundpower2(int width)
{
	int b = width;
	int n;
	for (n = 0; b != 0; n++) b >>= 1;
	b = 1 << n;
	if (b == 2 * width) b >>= 1;
	return b;
}

//////////////////////////////////////////////////////////////////////////
//for png load
//////////////////////////////////////////////////////////////////////////
static void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
        // ignore PNG warnings
}
static int offset = 0;

static void png_custom_mread_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
  
	uint8* handle = (uint8*)png_ptr->io_ptr;
	uint8* workptr = handle + offset;
	uint32 i;
   if (handle == NULL)
   {
      png_error(png_ptr, "Read Error!");
   }
   offset +=length;
   for(i=0;i<length;i++){
		data[i]=workptr[i];
   }

}

image_p image_load_png(const char* filename, int displaymode)
{
	image_p pimage = NULL;
	int size = 0;
	char* mbuf = NULL;
	int handle = io_fopen(filename,IO_RDONLY);

	if(handle == 0)
		return 0;
	size = io_fsize(handle);
	mbuf = (char*) malloc(size);
	io_fread(mbuf,1,size,handle);
	io_fclose(handle);
	pimage = image_load_png_buf(mbuf,size,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_png_fp(int handle,int fsize, int autoclose,int displaymode)
{
	image_p pimage = NULL;
	char* mbuf;
	
	if(handle == 0 || fsize == 0)
		return 0;
	
	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_png_buf(mbuf,fsize,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_png_buf(const char* mbuf, int bsize, int displaymode)
{

	uint32* p32;
    uint16* p16;
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type, x, y;
    uint32* line;
	image_p pimage = NULL;
	int texw,texh,bpb,size;
	uint8 done = 0;
	uint8* buffer = NULL;
	uint32 color32;
	uint16 color16;
	static uint8 r,g,b,a;
	offset = 0;

	if (mbuf == 0) return 0;
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
  			return 0;
    }
    png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
            png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
            return 0;
    }
    png_init_io(png_ptr, NULL);
	png_set_read_fn(png_ptr, (png_voidp)mbuf, png_custom_mread_fn);

    png_set_sig_bytes(png_ptr, sig_read);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
    png_set_strip_16(png_ptr);
    png_set_packing(png_ptr);
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
    line = (uint32*) malloc(width * 4);
    if (!line) {
            png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
            return 0;
    }
    
	texw = roundpower2(width);
	texh = roundpower2(height);
	bpb = 4;
	if(displaymode !=DISPLAY_PIXEL_FORMAT_8888){
		bpb = 2;
	}
	size = texw * texh * bpb;
	buffer = (uint8*) malloc(size);
	memset(buffer,0,size);
    if (buffer){
	        p32 = (uint32*) buffer;
	        p16 = (uint16*) p32;
	        for (y = 0; y < (int)height; y++){
                png_read_row(png_ptr, (uint8*) line, png_bytep_NULL);
                for (x = 0; x < (int)width; x++)  {
                    color32 = line[x];
                    color16;
					a = (color32 >> 24) & 0xff;
                    r = color32 & 0xff;
                    g = (color32 >> 8) & 0xff;
                    b = (color32 >> 16) & 0xff;
                    switch (displaymode){
                            case DISPLAY_PIXEL_FORMAT_565:
                                    color16 = MAKE_RGBA_565(r,g,b,a);
                                    *(p16+x) = color16;
                                    break;
                            case DISPLAY_PIXEL_FORMAT_5551:
                                    color16 = MAKE_RGBA_5551(r,g,b,a);
                                    *(p16+x) = color16;
                                    break;
                            case DISPLAY_PIXEL_FORMAT_4444:
                                    color16 = MAKE_RGBA_4444(r,g,b,a);
                                    *(p16+x) = color16;
                                    break;
                            case DISPLAY_PIXEL_FORMAT_8888:
                                    color32 = MAKE_RGBA_8888(r,g,b,a);
                                    *(p32+x) = color32;
                                    break;
                   }
                }
	            p32 += texw;
	            p16 += texw;
	       }
		   done = 1;
	 }
    SAFE_FREE (line);
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
    
	if (done){
		pimage = (image_p)malloc(sizeof(image_t));
		memset(pimage,0,sizeof(image_t));
		pimage->data = (uint8 *)buffer;
		pimage->w    = width;
		pimage->h    = height;
		pimage->texw = texw;
		pimage->texh = texh;
		pimage->bpb  = bpb;
		pimage->dtype = displaymode;
		pimage->rcentrex = pimage->w*1.0f/2;
		pimage->rcentrey = pimage->h*1.0f/2;
		pimage->mode = GET_PSM_COLOR_MODE(displaymode);
		pimage->mask = CreateColor(255,255,255,255,displaymode);
		pimage->texid = image_tid++;
	}
	return pimage;
}




image_p image_load_png_colorkey(const char* filename, int displaymode,int colorkey)
{
	image_p pimage = NULL;
	int handle = io_fopen(filename,IO_RDONLY);
	int size;
	char* mbuf;

	if(handle == 0)
		return 0;
	size = io_fsize(handle);
	mbuf = (char*) malloc(size);
	io_fread(mbuf,1,size,handle);
	io_fclose(handle);
	pimage = image_load_png_colorkey_buf(mbuf,size,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_png_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey)
{
	image_p pimage = NULL;
	char *mbuf;
	
	if(handle == 0 || fsize == 0)
		return 0;
	
	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_png_colorkey_buf(mbuf,fsize,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_png_colorkey_buf(const char* mbuf, int bsize, int displaymode,int colorkey)
{
	
	uint32* p32;
    uint16* p16;
	uint8* buffer;
	uint32* line;
	image_p pimage = NULL;
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type, x, y;
   	int texw,texh,bpb,size;
	uint8 done = 0;
	int pixcolor;
	uint32 color32;
    uint16 color16;
	static uint8 r,g,b,a,alpha;
	
	offset = 0;/*global*/

	if (mbuf == 0) return 0;
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
  			return 0;
    }
    png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
            png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
            return 0;
    }
    png_init_io(png_ptr, NULL);
	png_set_read_fn(png_ptr, (png_voidp)mbuf, png_custom_mread_fn);

    png_set_sig_bytes(png_ptr, sig_read);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
    png_set_strip_16(png_ptr);
    png_set_packing(png_ptr);
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
    line = (uint32*) malloc(width * 4);
    if (!line) {
            png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
            return 0;
    }
    
	texw = roundpower2(width);
	texh = roundpower2(height);
	bpb = 4;
	if(displaymode !=DISPLAY_PIXEL_FORMAT_8888){
		bpb = 2;
	}
	size = texw * texh * bpb;
	buffer = (uint8*) malloc(size);
	memset(buffer,0,size);
	alpha = 0xff;
	
    if (buffer){
	        p32 = (uint32*) buffer;
	        p16 = (uint16*) p32;
	        for (y = 0; y < (int)height; y++){
                png_read_row(png_ptr, (uint8*) line, png_bytep_NULL);
                for (x = 0; x < (int)width; x++)  {
                    color32 = line[x];
     				a = (color32 >> 24) & 0xff;
                    r = color32 & 0xff;
                    g = (color32 >> 8) & 0xff;
                    b = (color32 >> 16) & 0xff;
					pixcolor = MAKE_RGB(r,g,b);
					alpha = 0xff;
					if(colorkey == pixcolor){
						alpha = 0x00;
					}
                    switch (displaymode){
                            case DISPLAY_PIXEL_FORMAT_565:
									if(alpha==0x00)
										color16 = MAKE_RGBA_565(r,g,b,alpha);
									else   
										color16 = MAKE_RGBA_565(r,g,b,a);
                                    *(p16+x) = color16;
                                    break;
                            case DISPLAY_PIXEL_FORMAT_5551:
									if(alpha==0x00)
										color16 = MAKE_RGBA_5551(r,g,b,alpha);
									else   
										color16 = MAKE_RGBA_5551(r,g,b,a);
                                    *(p16+x) = color16;
                                    break;
                            case DISPLAY_PIXEL_FORMAT_4444:
                                    if(alpha==0x00)
										color16 = MAKE_RGBA_4444(r,g,b,alpha);
									else   
										color16 = MAKE_RGBA_4444(r,g,b,a);
                                    *(p16+x) = color16;
                                    break;
                            case DISPLAY_PIXEL_FORMAT_8888:
                                    if(alpha==0x00)
										color32 = MAKE_RGBA_8888(r,g,b,alpha);
									else   
										color32 = MAKE_RGBA_8888(r,g,b,a);
                                    *(p32+x) = color32;
                                    break;
                   }
                }
	            p32 += texw;
	            p16 += texw;
	       }
		   done = 1;
	 }
    SAFE_FREE (line);
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
    
	if (done){
		pimage = (image_p)malloc(sizeof(image_t));
		memset(pimage,0,sizeof(image_t));
		pimage->data = (uint8 *)buffer;
		pimage->w    = width;
		pimage->h    = height;
		pimage->texw = texw;
		pimage->texh = texh;
		pimage->bpb  = bpb;
		pimage->dtype = displaymode;
		pimage->rcentrex = pimage->w*1.0f/2;
		pimage->rcentrey = pimage->h*1.0f/2;
		pimage->mode = GET_PSM_COLOR_MODE(displaymode);
		pimage->mask = CreateColor(255,255,255,255,displaymode);
		pimage->texid = image_tid++;
	}
	return pimage;
}


//////////////////////////////////////////////////////////////////////////
//for tga load
//////////////////////////////////////////////////////////////////////////
#define RD16(x) (uint16)((uint16)x[0] | ((uint16)x[1] << 8))
image_p image_load_tga(const char* filename, int displaymode)
{
	image_p pimage = NULL;
	int size;
	char *mbuf;
	int handle;

	handle = io_fopen(filename,IO_RDONLY);
	if(handle == 0)
		return 0;
	size = io_fsize(handle);
	mbuf = (char*) malloc(size);
	io_fread(mbuf,1,size,handle);
	io_fclose(handle);
	pimage = image_load_tga_buf(mbuf,size,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_tga_buf(const char* mbuf,int bsize, int displaymode)
{
	image_p pimage = NULL;
	char *workptr;
	sint32 width,height,bytesperline,rlesize;
	uint32 texw,texh,fpos,color32,col32;
	uint8 *tdata,*dst,*rle,*src,*xsrc;
	uint8 done,bpb;
	char TRUEVIS[] = "TRUEVISION-XFILE.";
	long extoffs,devoffs;
	uint32 *p32;
	uint16 *p16;
	int x = 0,y = 0,size;
	sint32 ddelta,hdelta,vdelta;
	uint16 color16;
	static uint8 r,g,b,a,alpha;
	TGAFILEHEADER tfh;
	
	offset = 0;
	if (mbuf==NULL){ 
		return 0;
	}
    memset( &tfh, 0, sizeof(TGAFILEHEADER) );
    fpos = 0;
	workptr = (char*)mbuf;
	
	memcpy(&tfh,workptr,sizeof(TGAFILEHEADER));
    fpos += sizeof(TGAFILEHEADER)+tfh.ImageIDSize;
   
	workptr += fpos;
	
    if (tfh.ImageTypeCode!=1 && tfh.ImageTypeCode!=2 && tfh.ImageTypeCode!=3 &&
		tfh.ImageTypeCode!=9 && tfh.ImageTypeCode!=10 && tfh.ImageTypeCode!=11)
	{
		nge_print("Unknown ImageTypeCode.\n");
		return 0;
	}
	if (tfh.Depth!=8 && tfh.Depth!=15 && tfh.Depth!=16 && tfh.Depth!=24 && tfh.Depth!=32)
	{
		nge_print("Unknown Bit Depth.\n");
		return 0;
	}
	
	width = RD16(tfh.Width);// - RD16(tfh.OriginX);
	height = RD16(tfh.Height);// - RD16(tfh.OriginY);
	texw = roundpower2(width);
	texh = roundpower2(height);
	bytesperline = ((width*tfh.Depth)>>3);
	tdata = (uint8*) malloc( height * bytesperline );
	dst = NULL;
	done = 0;
	bpb = 4;
    if (tdata==0){
    	nge_print("malloc failed on tdata.\n");
		return 0;
	}

	if ( (tfh.ImageTypeCode&8) == 8 && (tfh.ImageTypeCode&3) > 0 )
	{
		// READ RLE ENCODED 
		rlesize = bsize-fpos;
		rle = (uint8*) malloc( rlesize );
		if (rle==0)
		{
			free( tdata );
			return 0 ;
		}
		
		memcpy(rle,workptr,rlesize);
		workptr += rlesize;

		if (memcmp( &rle[rlesize-26+8], TRUEVIS, 18 )==0)
		{
			// TRUEVISION-XFILE and may contain Developer and Extension Areas
			rlesize -= 26;
			extoffs = rle[rlesize+0] + (rle[rlesize+1]<<8) + (rle[rlesize+2]<<16) + (rle[rlesize+3]<<24);
			devoffs = rle[rlesize+4] + (rle[rlesize+5]<<8) + (rle[rlesize+6]<<16) + (rle[rlesize+7]<<24);
			if (extoffs!=0 || devoffs!=0)
			{
				// contains developer and/or extension area :/
				// For now we just assume that the developer or extension area is at the start of the whole extension block
				// Actually, we'd need to go through all following area blocks and find the one that comes first (lowest offset)
				if (devoffs<extoffs)
					rlesize = devoffs-fpos;
				else
					rlesize = extoffs-fpos;
			}
		}
		// DECODE RLE:
		decodeRLE( rle, rlesize, tdata, height * bytesperline, tfh.Depth );
		SAFE_FREE( rle );
	}
	else if ( ( tfh.ImageTypeCode == 1 ) || ( tfh.ImageTypeCode == 2 ) || ( tfh.ImageTypeCode == 3 ) )
	{
		memcpy(tdata,workptr,height*bytesperline);
		workptr += height*bytesperline;

	}
	else
	{
		SAFE_FREE( tdata );
		nge_print("Unknown ImageTypeCode.\n");
		return 0;
	}

	
	if((tfh.Depth==24)||(tfh.Depth==32)){
		bpb = 4;
		if(displaymode !=DISPLAY_PIXEL_FORMAT_8888){
			bpb = 2;
		}
		size = texw * texh * bpb;
		dst = (uint8*)malloc(size);
		memset(dst,0,size);
		src = (uint8*)tdata;
		ddelta = (texw-width)*bpb;
		hdelta = bytesperline;
		if ((tfh.ImageDescrip & TGA_DESC_HORIZONTAL) == 0){
			src += (height-1) * bytesperline;
			hdelta = -bytesperline;
		}
		vdelta = (tfh.Depth >> 3);
		if ((tfh.ImageDescrip & TGA_DESC_VERTICAL) != 0){
			src += bytesperline-(tfh.Depth >> 3);
			vdelta = -(tfh.Depth >> 3);
		}
	
		// convert the crappy image and do flipping if neccessary
	
		switch (tfh.Depth)
		{
			case 8:
			case 15:
			case 16:
				break;
			case 24:
				p32 = (uint32*) dst;
				p16 = (uint16*) p32;
				for (y = 0; y < (int)height; y++){
					xsrc = src;
					for (x = 0; x < (int)width; x++)  {
						
						col32 = (uint32)(((uint32)xsrc[2]<<16) | ((uint32)xsrc[1]<<8) | (uint32)xsrc[0] | (0xFF<<24));
						//BGRA
						a = (col32 >> 24) & 0xff;
						b = col32 & 0xff;
						g = (col32 >> 8) & 0xff;
						r = (col32 >> 16) & 0xff;
						switch (displaymode){
						case DISPLAY_PIXEL_FORMAT_565:
							color16 = MAKE_RGBA_565(r,g,b,a);
							*(p16+x) = color16;
							break;
						case DISPLAY_PIXEL_FORMAT_5551:
							color16 = MAKE_RGBA_5551(r,g,b,a);
							*(p16+x) = color16;
							break;
						case DISPLAY_PIXEL_FORMAT_4444:
							color16 = MAKE_RGBA_4444(r,g,b,a);
							*(p16+x) = color16;
							break;
						case DISPLAY_PIXEL_FORMAT_8888:
							color32 = MAKE_RGBA_8888(r,g,b,a);
							*(p32+x) = color32;
							break;
						}
						xsrc += vdelta;
					}
					src += hdelta;
					p32 += texw;
					p16 += texw;
				}
				break;
			case 32:
				p32 = (uint32*) dst;
				p16 = (uint16*) p32;
				for (y = 0; y < (int)height; y++){
					xsrc = src;
					for (x = 0; x < (int)width; x++)  {
						col32 = *(uint32*)(xsrc);
						//BGRA
						a = (col32 >> 24) & 0xff;
						b = col32 & 0xff;
						g = (col32 >> 8) & 0xff;
						r = (col32 >> 16) & 0xff;
						switch (displaymode){
						case DISPLAY_PIXEL_FORMAT_565:
							color16 = MAKE_RGBA_565(r,g,b,a);
							*(p16+x) = color16;
							break;
						case DISPLAY_PIXEL_FORMAT_5551:
							color16 = MAKE_RGBA_5551(r,g,b,a);
							*(p16+x) = color16;
							break;
						case DISPLAY_PIXEL_FORMAT_4444:
							color16 = MAKE_RGBA_4444(r,g,b,a);
							*(p16+x) = color16;
							break;
						case DISPLAY_PIXEL_FORMAT_8888:
							color32 = MAKE_RGBA_8888(r,g,b,a);
							*(p32+x) = color32;
							break;
						}
						xsrc += vdelta;
					}//end for
					src += hdelta;
					p32 += texw;
					p16 += texw;
			   }//end for
			}//end switch
		done = 1;
	}//end if
	
	if (done){
		pimage = (image_p)malloc(sizeof(image_t));
		memset(pimage,0,sizeof(image_t));
		pimage->data = (uint8 *)dst;
		pimage->w    = width;
		pimage->h    = height;
		pimage->texw = texw;
		pimage->texh = texh;
		pimage->bpb  = bpb;
		pimage->dtype = displaymode;
		pimage->rcentrex = pimage->w*1.0f/2;
		pimage->rcentrey = pimage->h*1.0f/2;
		pimage->mode = GET_PSM_COLOR_MODE(displaymode);
		pimage->mask = CreateColor(255,255,255,255,displaymode);
		pimage->texid = image_tid++;
	}

	SAFE_FREE(tdata);
	return pimage;
}

image_p image_load_tga_fp(int handle,int fsize, int autoclose,int displaymode)
{
	image_p pimage = NULL;
	char* mbuf;
	
	if(handle == 0 || fsize == 0)
		return 0;
	
	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_tga_buf(mbuf,fsize,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}



image_p image_load_tga_colorkey(const char* filename, int displaymode,int colorkey)
{
	image_p pimage;
	int size;
	char *mbuf;
	int handle = io_fopen(filename,IO_RDONLY);

	if(handle == 0)
		return 0;
	size= io_fsize(handle);
	mbuf = (char*) malloc(size);
	io_fread(mbuf,1,size,handle);
	io_fclose(handle);
	pimage = image_load_tga_colorkey_buf(mbuf,size,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_tga_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey)
{
	image_p pimage = NULL;
	char *workptr;
	sint32 width,height,bytesperline,rlesize,pixcolor;
	uint32 texw,texh,fpos,color32,col32;
	uint8 *tdata,*dst,*rle,*src,*xsrc;
	uint8 done,bpb;
	char TRUEVIS[] = "TRUEVISION-XFILE.";
	long extoffs,devoffs;
	uint32 *p32;
	uint16 *p16;
	int x = 0,y = 0,size;
	sint32 ddelta,hdelta,vdelta;
	uint16 color16;
	static uint8 r,g,b,a,alpha;
	TGAFILEHEADER tfh;
	
	if (mbuf==NULL){ 
		return 0;
	}

	offset = 0;
    memset( &tfh, 0, sizeof(TGAFILEHEADER) );
    fpos  = 0;
	workptr = (char*)mbuf;
	
	memcpy(&tfh,workptr,sizeof(TGAFILEHEADER));
    fpos += sizeof(TGAFILEHEADER)+tfh.ImageIDSize;
   
	workptr += fpos;
	
    if (tfh.ImageTypeCode!=1 && tfh.ImageTypeCode!=2 && tfh.ImageTypeCode!=3 &&
		tfh.ImageTypeCode!=9 && tfh.ImageTypeCode!=10 && tfh.ImageTypeCode!=11)
	{
		nge_print("Unknown ImageTypeCode.\n");
		return 0;
	}
	if (tfh.Depth!=8 && tfh.Depth!=15 && tfh.Depth!=16 && tfh.Depth!=24 && tfh.Depth!=32)
	{
		nge_print("Unknown Bit Depth.\n");
		return 0;
	}
	
	width = RD16(tfh.Width);// - RD16(tfh.OriginX);
	height = RD16(tfh.Height);// - RD16(tfh.OriginY);
	texw = roundpower2(width);
	texh = roundpower2(height);
	bytesperline = ((width*tfh.Depth)>>3);
	tdata = (uint8*) malloc( height * bytesperline );
	dst = NULL;
	done = 0;
	bpb = 4;
    if (tdata == 0){
    	nge_print("malloc failed on tdata.\n");
		return 0;
	}

	if ( (tfh.ImageTypeCode&8) == 8 && (tfh.ImageTypeCode&3) > 0 )
	{
		// READ RLE ENCODED 
		rlesize = bsize-fpos;
		rle = (uint8*) malloc( rlesize );
		if (rle==0)
		{
			free( tdata );
			return 0 ;
		}
		
		memcpy(rle,workptr,rlesize);
		workptr += rlesize;

		if (memcmp( &rle[rlesize-26+8], TRUEVIS, 18 )==0)
		{
			// TRUEVISION-XFILE and may contain Developer and Extension Areas
			rlesize -= 26;
			extoffs = rle[rlesize+0] + (rle[rlesize+1]<<8) + (rle[rlesize+2]<<16) + (rle[rlesize+3]<<24);
			devoffs = rle[rlesize+4] + (rle[rlesize+5]<<8) + (rle[rlesize+6]<<16) + (rle[rlesize+7]<<24);
			if (extoffs!=0 || devoffs!=0)
			{
				// contains developer and/or extension area :/
				// For now we just assume that the developer or extension area is at the start of the whole extension block
				// Actually, we'd need to go through all following area blocks and find the one that comes first (lowest offset)
				if (devoffs<extoffs)
					rlesize = devoffs-fpos;
				else
					rlesize = extoffs-fpos;
			}
		}
		// DECODE RLE:
		decodeRLE( rle, rlesize, tdata, height * bytesperline, tfh.Depth );
		SAFE_FREE( rle );
	}
	else if ( ( tfh.ImageTypeCode == 1 ) || ( tfh.ImageTypeCode == 2 ) || ( tfh.ImageTypeCode == 3 ) )
	{
		memcpy(tdata,workptr,height*bytesperline);
		workptr += height*bytesperline;

	}
	else
	{
		SAFE_FREE( tdata );
		nge_print("Unknown ImageTypeCode.\n");
		return 0;
	}

	
	if((tfh.Depth==24)||(tfh.Depth==32)){
		bpb = 4;
		if(displaymode !=DISPLAY_PIXEL_FORMAT_8888){
			bpb = 2;
		}
		size = texw * texh * bpb;
		dst = (uint8*)malloc(size);
		memset(dst,0,size);
		src = (uint8*)tdata;
	
		ddelta = (texw-width)*bpb;
	
		hdelta = bytesperline;
		if ((tfh.ImageDescrip & TGA_DESC_HORIZONTAL) == 0){
			src += (height-1) * bytesperline;
			hdelta = -bytesperline;
		}
	
		vdelta = (tfh.Depth >> 3);
		if ((tfh.ImageDescrip & TGA_DESC_VERTICAL) != 0){
			src += bytesperline-(tfh.Depth >> 3);
			vdelta = -(tfh.Depth >> 3);
		}
	
		// convert the crappy image and do flipping if neccessary
		switch (tfh.Depth)
		{
			case 8:
			case 15:
			case 16:
				break;
			case 24:
				p32 = (uint32*) dst;
				p16 = (uint16*) p32;
				for (y = 0; y < (int)height; y++){
					xsrc = src;
					for (x = 0; x < (int)width; x++)  {
						
						col32 = (uint32)(((uint32)xsrc[2]<<16) | ((uint32)xsrc[1]<<8) | (uint32)xsrc[0] | (0xFF<<24));
						//BGRA
						a = (col32 >> 24) & 0xff;
						b = col32 & 0xff;
						g = (col32 >> 8) & 0xff;
						r = (col32 >> 16) & 0xff;
						pixcolor = MAKE_RGB(r,g,b);
						if(colorkey == pixcolor){
							a = 0x00;
						}
						switch (displaymode){
						case DISPLAY_PIXEL_FORMAT_565:
							color16 = MAKE_RGBA_565(r,g,b,a);
							*(p16+x) = color16;
							break;
						case DISPLAY_PIXEL_FORMAT_5551:
							color16 = MAKE_RGBA_5551(r,g,b,a);
							*(p16+x) = color16;
							break;
						case DISPLAY_PIXEL_FORMAT_4444:
							color16 = MAKE_RGBA_4444(r,g,b,a);
							*(p16+x) = color16;
							break;
						case DISPLAY_PIXEL_FORMAT_8888:
							color32 = MAKE_RGBA_8888(r,g,b,a);
							*(p32+x) = color32;
							break;
						}
						xsrc += vdelta;
					}
					src += hdelta;
					p32 += texw;
					p16 += texw;
				}
				break;
			case 32:
				p32 = (uint32*) dst;
				p16 = (uint16*) p32;
				for (y = 0; y < (int)height; y++){
					xsrc = src;
					for (x = 0; x < (int)width; x++)  {
						col32 = *(uint32*)(xsrc);
						//BGRA
						a = (col32 >> 24) & 0xff;
						b = col32 & 0xff;
						g = (col32 >> 8) & 0xff;
						r = (col32 >> 16) & 0xff;
						pixcolor = MAKE_RGB(r,g,b);
						if(colorkey == pixcolor){
							a = 0x00;
						}
						switch (displaymode){
						case DISPLAY_PIXEL_FORMAT_565:
							color16 = MAKE_RGBA_565(r,g,b,a);
							*(p16+x) = color16;
							break;
						case DISPLAY_PIXEL_FORMAT_5551:
							color16 = MAKE_RGBA_5551(r,g,b,a);
							*(p16+x) = color16;
							break;
						case DISPLAY_PIXEL_FORMAT_4444:
							color16 = MAKE_RGBA_4444(r,g,b,a);
							*(p16+x) = color16;
							break;
						case DISPLAY_PIXEL_FORMAT_8888:
							color32 = MAKE_RGBA_8888(r,g,b,a);
							*(p32+x) = color32;
							break;
						}
						xsrc += vdelta;
					}//end for
					src += hdelta;
					p32 += texw;
					p16 += texw;
			   }//end for
			}//end switch
		done = 1;
	}//end if
	
	if (done){
		pimage = (image_p)malloc(sizeof(image_t));
		memset(pimage,0,sizeof(image_t));
		pimage->data = (uint8 *)dst;
		pimage->w    = width;
		pimage->h    = height;
		pimage->texw = texw;
		pimage->texh = texh;
		pimage->bpb  = bpb;
		pimage->dtype = displaymode;
		pimage->rcentrex = pimage->w*1.0f/2;
		pimage->rcentrey = pimage->h*1.0f/2;
		pimage->mode = GET_PSM_COLOR_MODE(displaymode);
		pimage->mask = CreateColor(255,255,255,255,displaymode);
		pimage->texid = image_tid++;
	}

	SAFE_FREE(tdata);
	return pimage;
}

image_p image_load_tga_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey)
{
	image_p pimage = NULL;
	char *mbuf;
	
	if(handle == 0 || fsize == 0)
		return 0;
	
	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_tga_colorkey_buf(mbuf,fsize,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}

//////////////////////////////////////////////////////////////////////////
//for jpeg load
//////////////////////////////////////////////////////////////////////////
static void jpg_null(j_decompress_ptr cinfo)
{
}

static boolean jpg_fill_input_buffer(j_decompress_ptr cinfo)
{
	return 1;
}

static void jpg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{

	cinfo->src->next_input_byte += (size_t) num_bytes;
	cinfo->src->bytes_in_buffer -= (size_t) num_bytes;

}

static void jpeg_mem_src(j_decompress_ptr cinfo, uint8 *mem, int len)
{
	cinfo->src = (struct jpeg_source_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));
	cinfo->src->init_source = jpg_null;
	cinfo->src->fill_input_buffer = jpg_fill_input_buffer;
	cinfo->src->skip_input_data = jpg_skip_input_data;
	cinfo->src->resync_to_restart = jpeg_resync_to_restart;
	cinfo->src->term_source = jpg_null;
	cinfo->src->bytes_in_buffer = len;
	cinfo->src->next_input_byte = mem;
}


image_p image_load_jpg(const char* filename, int displaymode)
{
	image_p pimage = NULL;
	int size;
	char *mbuf;
	int handle = io_fopen(filename,IO_RDONLY);
	if(handle == 0)
		return 0;
	size = io_fsize(handle);
	mbuf = (char*) malloc(size);
	io_fread(mbuf,1,size,handle);
	io_fclose(handle);
	pimage = image_load_jpg_buf(mbuf,size,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_jpg_buf(const char* mbuf,int bsize, int displaymode)
{
	image_p pimage = NULL;
	struct jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr jerr;
	uint8 *rawdata,*scanline, *p,*data = NULL;
	int	rawsize,texw,texh,width,height,bpb=2,size;
	int r,g,b,x;
	uint32* p32;
	uint16* p16;
	uint16 color16;
	uint32 color32;
	
	if(mbuf == NULL||bsize==0)
		return 0;
	memset(&cinfo,0,sizeof(struct jpeg_decompress_struct));
	memset(&jerr,0,sizeof(struct jpeg_error_mgr));
	
	rawsize = bsize;
	rawdata = (uint8*)mbuf;

	if (rawdata[6] != 'J' || rawdata[7] != 'F' || rawdata[8] != 'I' || rawdata[9] != 'F') 
	{ 
		return 0; 
	} 

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, rawdata, rawsize);

	jpeg_read_header(&cinfo, 1);

	jpeg_start_decompress(&cinfo);

	if(cinfo.output_components != 3 && cinfo.output_components != 4)
	{
		jpeg_destroy_decompress(&cinfo);
		return 0;
	}
	texw = roundpower2(cinfo.output_width);
	texh = roundpower2(cinfo.output_height);
	width = cinfo.output_width;
	height = cinfo.output_height;

	if(displaymode == DISPLAY_PIXEL_FORMAT_8888){
		bpb = 4;
	}
	size = texw * texh * bpb;
	data = (uint8*)malloc(size);
	memset(data,0,size);
	scanline = (uint8*)malloc(cinfo.output_width * 3);
	if(!scanline){
		jpeg_destroy_decompress(&cinfo);
		SAFE_FREE (data);
		return 0;
	}
	p32 = (uint32*)data;
	p16 = (uint16*) p32;

	while(cinfo.output_scanline < cinfo.output_height){
		jpeg_read_scanlines(&cinfo, &scanline, 1);
		p = (uint8*)scanline;
		for(x=0; x<(int)cinfo.output_width; x++){
		
			r = p[0];
			g = p[1];
			b = p[2];
			switch (displaymode){
				case DISPLAY_PIXEL_FORMAT_565:
					color16 = MAKE_RGBA_565(r,g,b,0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					color16 = MAKE_RGBA_5551(r,g,b,0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					color16 = MAKE_RGBA_4444(r,g,b,0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_8888:
					color32 = MAKE_RGBA_8888(r,g,b,0xff);
					*(p32+x) = color32;
					break;
			}
			p+=3;
		}
		p32 += texw;
		p16 += texw;
	}
	SAFE_FREE(scanline);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	pimage = (image_p)malloc(sizeof(image_t));
	memset(pimage,0,sizeof(image_t));
	pimage->data = (uint8 *)data;
	pimage->w    = width;
	pimage->h    = height;
	pimage->texw = texw;
	pimage->texh = texh;
	pimage->bpb  = bpb;
	pimage->dtype = displaymode;
	pimage->rcentrex = pimage->w*1.0f/2;
	pimage->rcentrey = pimage->h*1.0f/2;
	pimage->mode = GET_PSM_COLOR_MODE(displaymode);
	pimage->mask = CreateColor(255,255,255,255,displaymode);
	pimage->texid = image_tid++;
	return pimage;
}

image_p image_load_jpg_fp(int handle,int fsize, int autoclose,int displaymode)
{
	image_p pimage = NULL;
	char* mbuf;
	
	if(handle == 0 || fsize == 0)
		return 0;
	
	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_jpg_buf(mbuf,fsize,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_jpg_colorkey(const char* filename, int displaymode,int colorkey)
{
	image_p pimage = NULL;
	int size;
	char *mbuf;
	int handle = io_fopen(filename,IO_RDONLY);
	
	if(handle == 0)
		return 0;
	size = io_fsize(handle);
	mbuf = (char*) malloc(size);
	io_fread(mbuf,1,size,handle);
	io_fclose(handle);
	pimage = image_load_jpg_colorkey_buf(mbuf,size,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_jpg_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey)
{
	image_p pimage = NULL;
	struct jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr jerr;
	uint8 *rawdata,*scanline, *p,*data = NULL;
	int	rawsize,texw,texh,width,height,bpb=2,size;
	int r,g,b,x,color;
	uint32* p32;
	uint16* p16;
	uint16 color16;
	uint32 color32;
	uint8 alpha;
	
	if(mbuf == NULL||bsize==0)
		return 0;

	rawsize = bsize;
	rawdata = (uint8*)mbuf;

	if (rawdata[6] != 'J' || rawdata[7] != 'F' || rawdata[8] != 'I' || rawdata[9] != 'F') 
	{ 
		return 0; 
	} 


	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, rawdata, rawsize);

	jpeg_read_header(&cinfo, 1);

	jpeg_start_decompress(&cinfo);

	if(cinfo.output_components != 3 && cinfo.output_components != 4)
	{
		jpeg_destroy_decompress(&cinfo);
		return 0;
	}

	texw = roundpower2(cinfo.output_width);
	texh = roundpower2(cinfo.output_height);
	width = cinfo.output_width;
	height = cinfo.output_height;
	
	if(displaymode == DISPLAY_PIXEL_FORMAT_8888){
		bpb = 4;
	}
	size = texw * texh * bpb;
	data = (uint8*)malloc(size);
	memset(data,0,size);
		
	scanline = (uint8*)malloc(cinfo.output_width * 3);
	if(!scanline){
		jpeg_destroy_decompress(&cinfo);
		SAFE_FREE (data);
		return 0;
	}
	p32 = (uint32*)data;
	p16 = (uint16*) p32;
	while(cinfo.output_scanline < cinfo.output_height){
		jpeg_read_scanlines(&cinfo, &scanline, 1);
		p = (uint8*)scanline;
		for(x=0; x<(int)cinfo.output_width; x++){
		
			r = p[0];
			g = p[1];
			b = p[2];
			color = MAKE_RGB(r,g,b);
			alpha = 0xff;
			if(colorkey == color){
				alpha = 0x00;
			}
			switch (displaymode){
				case DISPLAY_PIXEL_FORMAT_565:
					color16 = MAKE_RGBA_565(r,g,b,alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					color16 = MAKE_RGBA_5551(r,g,b,alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					color16 = MAKE_RGBA_4444(r,g,b,alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_8888:
					color32 = MAKE_RGBA_8888(r,g,b,alpha);
					*(p32+x) = color32;
					break;
			}
			p+=3;
		}
		p32 += texw;
		p16 += texw;
	}
	SAFE_FREE(scanline);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	pimage = (image_p)malloc(sizeof(image_t));
	memset(pimage,0,sizeof(image_t));
	pimage->data = (uint8 *)data;
	pimage->w    = width;
	pimage->h    = height;
	pimage->texw = texw;
	pimage->texh = texh;
	pimage->bpb  = bpb;
	pimage->dtype = displaymode;
	pimage->rcentrex = pimage->w*1.0f/2;
	pimage->rcentrey = pimage->h*1.0f/2;
	pimage->mode = GET_PSM_COLOR_MODE(displaymode);
	pimage->mask = CreateColor(255,255,255,255,displaymode);
	pimage->texid = image_tid++;
	return pimage;
}

image_p image_load_jpg_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey)
{
	image_p pimage = NULL;
	char *mbuf;
	
	if(handle == 0 || fsize == 0)
		return 0;
	
	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_jpg_colorkey_buf(mbuf,fsize,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}

//////////////////////////////////////////////////////////////////////////
//load bmp
//support 24/32 bmp (NOT support 16)
//////////////////////////////////////////////////////////////////////////
image_p image_load_bmp(const char* filename, int displaymode)
{
	image_p pimage = NULL;
	int size ;
	uint8* pbuf;

	int fd = io_fopen(filename,IO_RDONLY);
	if(fd == 0)
		return 0;
	size = io_fsize(fd);
	pbuf = (uint8*)malloc(size); 
	io_fread(pbuf,1,size,fd);
	io_fclose(fd);
	pimage = image_load_bmp_buf((const char*)pbuf,size,displaymode);
	SAFE_FREE(pbuf);
	return pimage;
}

image_p image_load_bmp_buf(const char* mbuf,int bsize, int displaymode)
{
	image_p pimage = NULL;
	uint8 *pdata,*data,*line;
	int dsize,w,h,texw,texh,bpb,size,x,y,done = 0;
	uint32* p32;
	uint16* p16;
	uint16 color16;
	uint32 color32;
	PBITMAPFILEHEADER pbfh = (PBITMAPFILEHEADER)mbuf;
	PBITMAPINFOHEADER pbih;

	if(pbfh->bfType !=0x4d42){
		nge_print("not bmp file\n");
		return 0;
	}
	pbih = (PBITMAPINFOHEADER)(mbuf+sizeof(BITMAPFILEHEADER));
	dsize = sizeof(BITMAPFILEHEADER)+pbih->biSize;
	pdata =  (uint8*)mbuf+dsize;
	w = pbih->biWidth;
	h = pbih->biHeight;
	texw = roundpower2(w);
	texh = roundpower2(h);
	bpb  = 4;
	if(displaymode != DISPLAY_PIXEL_FORMAT_8888){
		bpb = 2;
	}
	//int biSizeImage = ((((pbih->biWidth * pbih->biBitCount) + 31) & ~31) / 8) * pbih->biHeight; 
	size = texw * texh * bpb;
	data = (uint8*)malloc(size);
	memset(data,0,size);
	p32 = (uint32*)data;
	p16 = (uint16*) p32;
	color16;
	color32;
	line = NULL;
	if(pbih->biBitCount == 24){
		for (y = h;y>0;y--){
			line = pdata+(y-1)*w*3;
			for (x=0;x<w;x++){
				switch(displaymode)
				{
				case DISPLAY_PIXEL_FORMAT_8888:
					color32 = MAKE_RGBA_8888(line[2],line[1],line[0],0xff);
					*(p32+x) = color32;
					break;
				case DISPLAY_PIXEL_FORMAT_565:
					color16 = MAKE_RGBA_565(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					color16 = MAKE_RGBA_5551(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					color16 = MAKE_RGBA_4444(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				}
				line+=3;
			}
			p32 += texw;
			p16 += texw;
		}
		done = 1;
	}
	else if(pbih->biBitCount == 32){
		for (y = h;y>0;y--){
			line = pdata+(y-1)*w*4;
			for (x=0;x<w;x++){
				switch(displaymode)
				{
				case DISPLAY_PIXEL_FORMAT_8888:
					color32 = MAKE_RGBA_8888(line[2],line[1],line[0],0xff);
					*(p32+x) = color32;
					break;
				case DISPLAY_PIXEL_FORMAT_565:
					color16 = MAKE_RGBA_565(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					color16 = MAKE_RGBA_5551(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					color16 = MAKE_RGBA_4444(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				}
				line+=4;
			}
			p32 += texw;
			p16 += texw;
		}
		done = 1;
	}
	
	if (done){
		pimage = (image_p)malloc(sizeof(image_t));
		memset(pimage,0,sizeof(image_t));
		pimage->data = (uint8 *)data;
		pimage->w    = w;
		pimage->h    = h;
		pimage->texw = texw;
		pimage->texh = texh;
		pimage->bpb  = bpb;
		pimage->dtype = displaymode;
		pimage->rcentrex = pimage->w*1.0f/2;
		pimage->rcentrey = pimage->h*1.0f/2;
		pimage->mode = GET_PSM_COLOR_MODE(displaymode);
		pimage->mask = CreateColor(255,255,255,255,displaymode);
		pimage->texid = image_tid++;
	}
	return pimage;
}

image_p image_load_bmp_fp(int handle,int fsize, int autoclose,int displaymode)
{
	image_p pimage = NULL;
	char* mbuf;
	
	if(handle == 0 || fsize == 0)
		return 0;
	
	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_bmp_buf(mbuf,fsize,displaymode);
	SAFE_FREE(mbuf);
	return pimage;
}



image_p image_load_bmp_colorkey(const char* filename, int displaymode,int colorkey)
{
	image_p pimage = NULL;
	int size;
	uint8* pbuf;
	int fd = io_fopen(filename,IO_RDONLY);
	
	if(fd == 0)
		return 0;
	size = io_fsize(fd);
	pbuf = (uint8*)malloc(size); 
	io_fread(pbuf,1,size,fd);
	io_fclose(fd);
	pimage = image_load_bmp_colorkey_buf((const char*)pbuf,size,displaymode,colorkey);
	SAFE_FREE(pbuf);
	return pimage;
}

image_p image_load_bmp_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey)
{
	image_p pimage = NULL;
	uint8 *pdata,*data,*line;
	int dsize,w,h,texw,texh,bpb,size,x,y,done = 0,pixcolor;
	uint32* p32;
	uint16* p16;
	uint16 color16;
	uint32 color32;
	PBITMAPFILEHEADER pbfh = (PBITMAPFILEHEADER)mbuf;
	PBITMAPINFOHEADER pbih;
	uint8 alpha;
	
	if(pbfh->bfType !=0x4d42){
		nge_print("not bmp file\n");
		return 0;
	}
	
	pbih = (PBITMAPINFOHEADER)(mbuf+sizeof(BITMAPFILEHEADER));
	dsize = sizeof(BITMAPFILEHEADER)+pbih->biSize;
	pdata =  (uint8*)mbuf+dsize;
	w = pbih->biWidth;
	h = pbih->biHeight;
	texw = roundpower2(w);
	texh = roundpower2(h);
	bpb  = 4;
	if(displaymode != DISPLAY_PIXEL_FORMAT_8888){
		bpb = 2;
	}
	//int biSizeImage = ((((pbih->biWidth * pbih->biBitCount) + 31) & ~31) / 8) * pbih->biHeight; 
	size = texw * texh * bpb;
	data = (uint8*)malloc(size);
	memset(data,0,size);
	p32 = (uint32*)data;
	p16 = (uint16*) p32;
	alpha = 0xff;
	line = NULL;
	if(pbih->biBitCount == 24){
		for (y = h;y>0;y--){
			line = pdata+(y-1)*w*3;
			for (x=0;x<w;x++){
				pixcolor = MAKE_RGB(line[2],line[1],line[0]);
				alpha = 0xff;
				if(colorkey == pixcolor){
					alpha = 0x00;
				}
				switch(displaymode)
				{
				case DISPLAY_PIXEL_FORMAT_8888:
					color32 = MAKE_RGBA_8888(line[2],line[1],line[0],alpha);
					*(p32+x) = color32;
					break;
				case DISPLAY_PIXEL_FORMAT_565:
					color16 = MAKE_RGBA_565(line[2],line[1],line[0],alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					color16 = MAKE_RGBA_5551(line[2],line[1],line[0],alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					color16 = MAKE_RGBA_4444(line[2],line[1],line[0],alpha);
					*(p16+x) = color16;
					break;
				}
				line+=3;
			}
			p32 += texw;
			p16 += texw;
		}
		done = 1;
	}
	else if(pbih->biBitCount == 32){
		for (y = h;y>0;y--){
			line = pdata+(y-1)*w*4;
			for (x=0;x<w;x++){
				pixcolor = MAKE_RGB(line[2],line[1],line[0]);
				alpha = 0xff;
				if(colorkey == pixcolor){
					alpha = 0x00;
				}
				switch(displaymode)
				{
				case DISPLAY_PIXEL_FORMAT_8888:
					color32 = MAKE_RGBA_8888(line[2],line[1],line[0],alpha);
					*(p32+x) = color32;
					break;
				case DISPLAY_PIXEL_FORMAT_565:
					color16 = MAKE_RGBA_565(line[2],line[1],line[0],alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					color16 = MAKE_RGBA_5551(line[2],line[1],line[0],alpha);
					*(p16+x) = color16;
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					color16 = MAKE_RGBA_4444(line[2],line[1],line[0],0xff);
					*(p16+x) = color16;
					break;
				}
				line+=4;
			}
			p32 += texw;
			p16 += texw;
		}
		done = 1;
	}
	
	if (done){
		pimage = (image_p)malloc(sizeof(image_t));
		memset(pimage,0,sizeof(image_t));
		pimage->data = (uint8 *)data;
		pimage->w    = w;
		pimage->h    = h;
		pimage->texw = texw;
		pimage->texh = texh;
		pimage->bpb  = bpb;
		pimage->dtype = displaymode;
		pimage->rcentrex = pimage->w*1.0f/2;
		pimage->rcentrey = pimage->h*1.0f/2;
		pimage->mode = GET_PSM_COLOR_MODE(displaymode);
		pimage->mask = CreateColor(255,255,255,255,displaymode);
		pimage->texid = image_tid++;
	}
	return pimage;
}

image_p image_load_bmp_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey)
{
	image_p pimage = NULL;
	char *mbuf;
	
	if(handle == 0 || fsize == 0)
		return 0;
	
	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_bmp_colorkey_buf(mbuf,fsize,displaymode,colorkey);
	SAFE_FREE(mbuf);
	return pimage;
}

//////////////////////////////////////////////////////////////////////////
//save image_p
//support:png tga
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//to png
//////////////////////////////////////////////////////////////////////////
static void png_custom_fwrite_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t check;
	
	int handle = (int)(png_ptr->io_ptr);
	
	check = io_fwrite(data,1,length,handle);
	if (check != length)
	{
		nge_print("Read Error!");
		png_error(png_ptr, "Read Error!");
	}
}

int image_save_png(image_p pimage,const char* filename,uint8 alpha)
{
	png_structp png_ptr;
	png_infop info_ptr;
	uint8* line;
	uint32 col_type;
	int handle;
	uint8 *src,*src8;
	uint16 *src16 ;
	uint32 *src32 ;
	uint32 x,y,i;
	uint8 r = 0, g = 0, b = 0, a = 0;
	uint16 col16;
	uint32 col32;
		
	
	if (pimage==0) 
		return 0;
	if (pimage->data==0 || pimage->w==0 || pimage->h==0) 
		return 0;


	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return 0;
	
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return 0;
	}
	
	col_type = PNG_COLOR_TYPE_RGB;
	if (alpha!=0)
		col_type = PNG_COLOR_TYPE_RGBA;

	handle = io_fopen(filename, IO_WRONLY);
	if (handle == 0) 
		return 0 ;
	
	png_init_io(png_ptr, NULL);
	png_set_write_fn(png_ptr, (png_voidp)handle, png_custom_fwrite_fn,NULL);
	png_set_IHDR(png_ptr, info_ptr, pimage->w, pimage->h,
		8, col_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	
	line = (uint8*) malloc(pimage->w * 4);
	if (line==0)
	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		io_fclose(handle);
		return 0;
	}
	
	src = (uint8*)pimage->data;
	src8 = (uint8*)src;
	src16 = (uint16*)src;
	src32 = (uint32*)src;
	for (y = 0; y < pimage->h; y++)
	{
		//uint32 swap = 0;
		src8 = (uint8*)src;
		src16 = (uint16*)src;
		src32 = (uint32*)src;
		for (i = 0, x = 0; x < pimage->w; x++)
		{
			switch (pimage->dtype)
			{
				case DISPLAY_PIXEL_FORMAT_565:
					col16 = *src16++;
					GET_RGBA_565(col16,r,g,b,a);
					break;
				case DISPLAY_PIXEL_FORMAT_5551:
					col16 = *src16++;
					GET_RGBA_5551(col16,r,g,b,a);
					break;
				case DISPLAY_PIXEL_FORMAT_4444:
					col16 = *src16++;
					GET_RGBA_4444(col16,r,g,b,a);
					break;
				case DISPLAY_PIXEL_FORMAT_8888:
					col32 = *src32++;
					GET_RGBA_8888(col32,r,g,b,a);
					break;
			}
			line[i++] = r;
			line[i++] = g;
			line[i++] = b;
			if (alpha!=0)
				line[i++] = a;
		}
		png_write_row(png_ptr, line);
		src += ((pimage->texw*pimage->bpb));
	}
	SAFE_FREE(line);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	io_fclose(handle);
	return 1;
}

//////////////////////////////////////////////////////////////////////////
//to tga
//////////////////////////////////////////////////////////////////////////
int image_save_tga(image_p pimage,const char* filename,uint8 alpha,uint8 rle)
{
	int fd;
	uint8 *line,*rleline;
	TGAFILEHEADER tfh;
    uint32 x,y;
	static uint8 b,g,r,a;
	long pos;
	uint8 *src;
	uint16 col16;
	
	if (pimage == 0) 
		return 0;
	if (pimage->data==0 || pimage->w==0 || pimage->h==0) 
		return 0;
	fd = io_fopen(filename, IO_WRONLY);
	if (!fd) 
		return 0;
    memset( &tfh, 0, sizeof(TGAFILEHEADER) );
	tfh.ImageTypeCode = 2;
	tfh.Depth = (alpha?32:24);
	if (rle)
		tfh.ImageTypeCode |= 8;
	tfh.ImageDescrip = TGA_DESC_HORIZONTAL;
	if (alpha)
		tfh.ImageDescrip |= TGA_DESC_ABITS;
	tfh.Width[1] = pimage->w/256;
	tfh.Width[0] = pimage->w%256;
	tfh.Height[1] = pimage->h/256;
	tfh.Height[0] = pimage->h%256;
	
    io_fwrite( &tfh, 1, sizeof(tfh), fd );
    
	line = (uint8*) malloc(pimage->w * 4);
	if (line == 0){
		io_fclose(fd);
		return 0;
	}
	memset( line, 0, pimage->w*4 );
	
	rleline = 0;
	if (rle){
		rleline = (uint8*) malloc(pimage->w * 6);
		// NO RLE if FAIL
		if (rleline == 0){
			rle = 0;
			tfh.ImageTypeCode &= ~8;
			pos = io_fseek( fd, 0, IO_SEEK_CUR );
			io_fseek( fd, 0, IO_SEEK_SET );
			io_fwrite( &tfh, 1, sizeof(tfh), fd);
			io_fseek( fd, pos, IO_SEEK_SET );
		}
	}
	
	src = (uint8*)pimage->data;

	for (y = 0;y<pimage->h;y++)
	{
		switch (pimage->dtype)
		{
			case DISPLAY_PIXEL_FORMAT_5551:
				if (alpha)
					for (x=0;x<pimage->w;x++){
						col16 = (src[0 + x*2] | (src[1 + x*2] << 8));
						GET_RGBA_5551(col16,r,g,b,a);
						line[0 + x*4] = b;
						line[1 + x*4] = g;
						line[2 + x*4] = r;
						line[3 + x*4] = a;
					}
				else
					for (x=0;x<pimage->w;x++){
						col16 = (src[0 + x*2] | (src[1 + x*2] << 8));
						GET_RGBA_5551(col16,r,g,b,a);
						line[0 + x*3] = b;
						line[1 + x*3] = g;
						line[2 + x*3] = r;
					}
				break;
			case DISPLAY_PIXEL_FORMAT_565:
				if (alpha)
					for (x=0;x<pimage->w;x++){
						col16 = (src[0 + x*2] | (src[1 + x*2] << 8));
						GET_RGBA_565(col16,r,g,b,a);
						line[0 + x*4] = b;
						line[1 + x*4] = g;
						line[2 + x*4] = r;
						line[3 + x*4] = a;
					}
					else
						for (x=0;x<pimage->w;x++){
							col16 = (src[0 + x*2] | (src[1 + x*2] << 8));
							GET_RGBA_565(col16,r,g,b,a);
							line[0 + x*3] = b;
							line[1 + x*3] = g;
							line[2 + x*3] = r;
						}
				break;
			case DISPLAY_PIXEL_FORMAT_8888:
				if (alpha)
					for (x=0;x<pimage->w;x++){
						line[0 + x*4] = src[2 + x*4];
						line[1 + x*4] = src[1 + x*4];
						line[2 + x*4] = src[0 + x*4];
						line[3 + x*4] = src[3 + x*4];
					}
				else
					for (x=0;x<pimage->w;x++){
						line[0 + x*3] = src[2 + x*4];
						line[1 + x*3] = src[1 + x*4];
						line[2 + x*3] = src[0 + x*4];
					}
				break;
			case DISPLAY_PIXEL_FORMAT_4444:
				if (alpha)
					for (x=0;x<pimage->w;x++){
						col16 = (src[0 + x*2] | (src[1 + x*2] << 8));
						GET_RGBA_4444(col16,r,g,b,a);
						line[0 + x*4] = b;
						line[1 + x*4] = g;
						line[2 + x*4] = r;
						line[3 + x*4] = a;
					}
				else
					for (x=0;x<pimage->w;x++){
						col16 = (src[0 + x*2] | (src[1 + x*2] << 8));
						GET_RGBA_4444(col16,r,g,b,a);
						line[0 + x*3] = b;
						line[1 + x*3] = g;
						line[2 + x*3] = r;
					}
				break;
		}
		if (rle)
		{
			long sz = encodeRLE( line, pimage->w*tfh.Depth>>3, 0, rleline, pimage->w*6, tfh.Depth );
			io_fwrite( rleline, 1, sz, fd );
		}
		else
			io_fwrite( line, 1, pimage->w*tfh.Depth>>3, fd );
		src += pimage->texw*pimage->bpb;
	}
	if (rle)
		SAFE_FREE(rleline);
	SAFE_FREE(line);
	io_fclose(fd);
	return 1;
}


image_p image_load(const char* filename, int displaymode,int swizzle)
{
	image_p pimage = NULL;
	char flags[12]={0};
 	int fd = io_fopen(filename,IO_RDONLY);

	if(fd ==0 )
		return 0;
	io_fread(flags,1,12,fd);
	io_fclose(fd);
	if(flags[0]==(char)0x89&&flags[1]=='P'&&flags[2]=='N'&&flags[3]=='G'){
		pimage = image_load_png(filename,displaymode);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(flags[0]=='B'&&flags[1]=='M'){
		pimage = image_load_bmp(filename,displaymode);
		if(pimage == NULL){
			nge_print("bmp file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(flags[6]=='J'&&flags[7]=='F'&&flags[8]=='I'){
		pimage = image_load_jpg(filename,displaymode);
		if(pimage == NULL){
			nge_print("jpg file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(flags[0]==(char)0x0){
		pimage = image_load_tga(filename,displaymode);
		if(pimage == NULL){
			nge_print("tga file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else{
		nge_print("not support file type!\n");
	}
	return 0;
}

image_p image_load_buf(const char* mbuf,int bsize, int displaymode,int swizzle)
{
	image_p pimage = NULL;
	if(bsize < 12||mbuf == 0)
		return 0;
	if(mbuf[0]==(char)0x89&&mbuf[1]=='P'&&mbuf[2]=='N'&&mbuf[3]=='G'){
		pimage = image_load_png_buf(mbuf,bsize,displaymode);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(mbuf[0]=='B'&&mbuf[1]=='M'){
		pimage = image_load_bmp_buf(mbuf,bsize,displaymode);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(mbuf[6]=='J'&&mbuf[7]=='F'&&mbuf[8]=='I'){
		pimage =  image_load_jpg_buf(mbuf,bsize,displaymode);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(mbuf[0]==(char)0x0){
		pimage =  image_load_tga_buf(mbuf,bsize,displaymode);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else{
		nge_print("not support file type!\n");
	}
	return 0;
}

image_p image_load_fp(int handle,int fsize, int autoclose,int displaymode,int swizzle)
{
	image_p pimage = NULL;
	char *mbuf;
	
	if(handle == 0 || fsize == 0)
		return 0;
	
	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_buf(mbuf,fsize,displaymode,swizzle);
	SAFE_FREE(mbuf);
	return pimage;
}

image_p image_load_colorkey(const char* filename, int displaymode,int colorkey,int swizzle)
{
	image_p pimage = NULL;
	char flags[12]={0};
	int fd = io_fopen(filename,IO_RDONLY);

	if(fd==0)
		return 0;
	io_fread(flags,1,12,fd);
	io_fclose(fd);
	if(flags[0]==(char)0x89&&flags[1]=='P'&&flags[2]=='N'&&flags[3]=='G'){
		pimage = image_load_png_colorkey(filename,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(flags[0]=='B'&&flags[1]=='M'){
		pimage =  image_load_bmp_colorkey(filename,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(flags[6]=='J'&&flags[7]=='F'&&flags[8]=='I'){
		pimage = image_load_jpg_colorkey(filename,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(flags[0]==(char)0x00){
		pimage =  image_load_tga_colorkey(filename,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else{
		nge_print("not support file type!\n");
	}
	return 0;
}

image_p image_load_colorkey_buf(const char* mbuf,int bsize, int displaymode,int colorkey,int swizzle)
{
	image_p pimage = NULL;
	if(bsize < 12||mbuf == 0)
		return 0;
	if(mbuf[0]==(char)0x89&&mbuf[1]=='P'&&mbuf[2]=='N'&&mbuf[3]=='G'){
		
		pimage =  image_load_png_colorkey_buf(mbuf,bsize,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(mbuf[0]=='B'&&mbuf[1]=='M'){
		//printf("bmp\n");
		pimage =  image_load_bmp_colorkey_buf(mbuf,bsize,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(mbuf[6]=='J'&&mbuf[7]=='F'&&mbuf[8]=='I'){
		//printf("jpg\n");
		pimage =  image_load_jpg_colorkey_buf(mbuf,bsize,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	else if(mbuf[0]==(char)0x00){
		//printf("tga\n");
		pimage =  image_load_tga_colorkey_buf(mbuf,bsize,displaymode,colorkey);
		if(pimage == NULL){
			nge_print("png file error!\n");
			return NULL;
		}
		if(swizzle == 1){
			swizzle_swap(pimage);
		}
		return pimage;
	}
	return 0;
}

image_p image_load_colorkey_fp(int handle,int fsize, int autoclose,int displaymode,int colorkey,int swizzle)
{
	image_p pimage = NULL;
	char *mbuf;
	
	if(handle == 0 || fsize == 0)
		return 0;
	
	mbuf = (char*) malloc(fsize);
	io_fread(mbuf,1,fsize,handle);
	if(autoclose)
		io_fclose(handle);
	pimage = image_load_colorkey_buf(mbuf,fsize,displaymode,colorkey,swizzle);
	SAFE_FREE(mbuf);
	return pimage;
}



image_p image_create(int w,int h,int displaymode)
{
	image_p pimage = (image_p)malloc(sizeof(image_t));
	int size;
	memset(pimage,0,sizeof(image_t));
	pimage->w = w;
	pimage->h = h;
	pimage->texw = roundpower2(w);
	pimage->texh = roundpower2(h);
	pimage->bpb  = (displaymode==DISPLAY_PIXEL_FORMAT_8888)?4:2;
	pimage->dtype = displaymode;
	pimage->rcentrex = pimage->w*1.0f/2;
	pimage->rcentrey = pimage->h*1.0f/2;
	pimage->mode = GET_PSM_COLOR_MODE(displaymode);
	pimage->mask = CreateColor(255,255,255,255,displaymode);
	pimage->texid = image_tid++;
	size = pimage->texw*pimage->texh*pimage->bpb;
	pimage->data = (uint8*)malloc(size);
	memset(pimage->data,0,size);
	return pimage;
}

image_p image_create_ex(int w,int h,int color,int displaymode)
{
	image_p pimage = (image_p)malloc(sizeof(image_t));
	int size,i,j;
	uint16* img16;
	uint32* img32;
	memset(pimage,0,sizeof(image_t));
	pimage->w = w;
	pimage->h = h;
	pimage->texw = roundpower2(w);
	pimage->texh = roundpower2(h);
	pimage->bpb  = (displaymode==DISPLAY_PIXEL_FORMAT_8888)?4:2;
	pimage->dtype = displaymode;
	pimage->rcentrex = pimage->w*1.0f/2;
	pimage->rcentrey = pimage->h*1.0f/2;
	pimage->mode = GET_PSM_COLOR_MODE(displaymode);
	pimage->mask = CreateColor(255,255,255,255,displaymode);
	pimage->texid = image_tid++;
	size = pimage->texw*pimage->texh*pimage->bpb;
	pimage->data = (uint8*)malloc(size);
	memset(pimage->data,0,size);
	if(displaymode==DISPLAY_PIXEL_FORMAT_8888){
		img32 = (uint32*)pimage->data;
		for(i=0;i<pimage->h;i++)
			for(j=0;j<pimage->w;j++){
				*(img32+i*pimage->texw+j) = color;
			}
	}
	else{
		img16 = (uint16*)pimage->data;
		for(i=0;i<pimage->h;i++)
			for(j=0;j<pimage->w;j++){
				*(img16+i*pimage->texw+j) = (uint16)(color&0xffff);
			}
	}
	return pimage;
}


void image_free(image_p pimage)
{
	if(pimage){
		SAFE_FREE(pimage->data);
		SAFE_FREE(pimage);
	}
}

void image_clear(image_p pimage)
{
	memset(pimage->data,0,pimage->texw*pimage->texh*pimage->bpb);
	pimage->modified = 1;
}

image_p image_clone(image_p pimage)
{
	int size;
	image_p clone = (image_p)malloc(sizeof(image_t));

	memcpy(clone,pimage,sizeof(image_t));
	clone->texid = image_tid++;
	size = pimage->texw*pimage->texh*pimage->bpb;
	clone->data = (uint8*)malloc(size);
	memcpy(clone->data,pimage->data,size);
	return clone;
}


/*alpha blend!
 *0 完全透明，255不透明
 *Dst=( Src0*(255-Alpha) + Src1*Alpha ) / 255
 *#define MAKEALPHA(SRC,DES,ALPHA) (( SRC*(255-ALPHA) + DES*ALPHA ) /255)*/
#define MAKEALPHA(SRC,DES,ALPHA) (( SRC*ALPHA + DES*(255-ALPHA) ) /255)

void image_to_image_alpha_ex(const image_p src,const image_p des,uint32 sx,uint32 sy,uint32 sw,uint32 sh,uint32 dx,uint32 dy,int alpha)
{
	uint8 sr,sg,sb,sa;
	uint8 dr,dg,db,da;
	uint32 i,j;
	uint16 *cpbegin16;
	uint16 *bmp16;
	
	uint32 *cpbegin32,*bmp32;
	if(src->swizzle ==1)
		unswizzle_swap(src);
	if(des->swizzle ==1)
		unswizzle_swap(des);
	des->modified = 1;
	if(sw == 0 && sh == 0){
		sw = src->w;
		sh = src->h;
	}
	
	if(des->dtype==DISPLAY_PIXEL_FORMAT_4444){
		cpbegin16 = (uint16*)des->data+dy*des->texw+dx;
		bmp16 = (uint16*)src->data+sy*src->texw+sx;
		for(i = 0;i<sh;i++){
			for(j = 0;j<sw;j++){
				if((j+dx>des->texw)||(i+dy>des->texh))
					continue;
				GET_RGBA_4444(bmp16[j],sr,sg,sb,sa);
				if(alpha == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin16[j] = bmp16[j];
				}
				else{
					GET_RGBA_4444(cpbegin16[j],dr,dg,db,da);
					cpbegin16[j] = MAKE_RGBA_4444(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin16 += des->texw;
			bmp16     += src->texw;
		}
	
	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_5551){
		cpbegin16 = (uint16*)des->data+dy*des->texw+dx;
		bmp16 = (uint16*)src->data+sy*src->texw+sx;
		for(i = 0;i<sh;i++){
			for(j = 0;j<sw;j++){
				if((j+dx>des->texw)||(i+dy>des->texh))
					continue;
				GET_RGBA_5551(bmp16[j],sr,sg,sb,sa);
				if(alpha == 0||sa == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin16[j] = bmp16[j];
				}
				else{
					GET_RGBA_5551(cpbegin16[j],dr,dg,db,da);
					cpbegin16[j] = MAKE_RGBA_5551(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin16 += des->texw;
			bmp16     += src->texw;
		}
	
	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_565){
		cpbegin16 = (uint16*)des->data+dy*des->texw+dx;
		bmp16 = (uint16*)src->data+sy*src->texw+sx;
		for(i = 0;i<sh;i++){
			for(j = 0;j<sw;j++){
				if((j+dx>des->texw)||(i+dy>des->texh))
					continue;
				GET_RGBA_565(bmp16[j],sr,sg,sb,sa);
				if(alpha == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin16[j] = bmp16[j];
				}
				else{
					GET_RGBA_565(cpbegin16[j],dr,dg,db,da);
					cpbegin16[j] = MAKE_RGBA_565(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin16 += des->texw;
			bmp16     += src->texw;
		}
	
	}
	else{
		cpbegin32 = (uint32*)des->data+dy*des->texw+dx;
		bmp32 = (uint32*)src->data+sy*src->texw+sx;
		for(i = 0;i<sh;i++){
			for(j = 0;j<sw;j++){
				if((j+dx>des->texw)||(i+dy>des->texh))
					continue;
				GET_RGBA_8888(bmp32[j],sr,sg,sb,sa);
				if(alpha == 0||sa == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin32[j] = bmp32[j];
				}
				else{
					GET_RGBA_8888(cpbegin32[j],dr,dg,db,da);
					cpbegin32[j] = MAKE_RGBA_8888(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin32 += des->texw;
			bmp32     += src->texw;
		}
	}

}

void image_to_image_alpha(const image_p src,const image_p des,uint32 x,uint32 y,int alpha)
{
	uint8 sr,sg,sb,sa;
	uint8 dr,dg,db,da;
	uint32 i,j;
	uint16 *cpbegin16;
	uint16 *bmp16;
	uint32 w = src->w;
	uint32 h = src->h;
	uint32 *cpbegin32,*bmp32;
	
	if(src->swizzle ==1)
		unswizzle_swap(src);
	if(des->swizzle ==1)
		unswizzle_swap(des);
	des->modified = 1;
	if(des->dtype==DISPLAY_PIXEL_FORMAT_4444){
		cpbegin16 = (uint16*)des->data+y*des->texw+x;
		bmp16 = (uint16*)src->data;
		for(i =0;i<h;i++){
			for(j =0;j<w;j++){
				if((j+x>des->texw)||(i+y>des->texh))
					continue;
				GET_RGBA_4444(bmp16[i*src->texw+j],sr,sg,sb,sa);
				if(alpha == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin16[j] = bmp16[i*src->texw+j];
				}
				else{
					GET_RGBA_4444(cpbegin16[j],dr,dg,db,da);
					cpbegin16[j] = MAKE_RGBA_4444(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin16 += des->texw;
		}
	
	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_5551){
		cpbegin16 = (uint16*)des->data+y*des->texw+x;
		bmp16 = (uint16*)src->data;
		for(i =0;i<h;i++){
			for(j =0;j<w;j++){
				if((j+x>des->texw)||(i+y>des->texh))
					continue;
				GET_RGBA_5551(bmp16[i*src->texw+j],sr,sg,sb,sa);
				if(alpha == 0||sa == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin16[j] = bmp16[i*src->texw+j];
				}
				else{
					GET_RGBA_5551(cpbegin16[j],dr,dg,db,da);
					cpbegin16[j] = MAKE_RGBA_5551(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin16 += des->texw;
		}
	
	}
	else if(des->dtype==DISPLAY_PIXEL_FORMAT_565){
		cpbegin16 = (uint16*)des->data+y*des->texw+x;
		bmp16 = (uint16*)src->data;
		for(i =0;i<h;i++){
			for(j =0;j<w;j++){
				if((j+x>des->texw)||(i+y>des->texh))
					continue;
				GET_RGBA_565(bmp16[i*src->texw+j],sr,sg,sb,sa);
				if(alpha == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin16[j] = bmp16[i*src->texw+j];
				}
				else{
					GET_RGBA_565(cpbegin16[j],dr,dg,db,da);
					cpbegin16[j] = MAKE_RGBA_565(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin16 += des->texw;
		}
	
	}
	else{
		cpbegin32 = (uint32*)des->data+y*des->texw+x;
		bmp32 = (uint32*)src->data;
		for(i =0;i<h;i++){
			for(j =0;j<w;j++){
				if((j+x>des->texw)||(i+y>des->texh))
					continue;
				GET_RGBA_8888(bmp32[i*src->texw+j],sr,sg,sb,sa);
				if(alpha == 0||sa == 0){
					continue;
				}
				else if(alpha == 255){
					cpbegin32[j] = bmp32[i*src->texw+j];
				}
				else{
					GET_RGBA_8888(cpbegin32[j],dr,dg,db,da);
					cpbegin32[j] = MAKE_RGBA_8888(MAKEALPHA(sr,dr,alpha),MAKEALPHA(sg,dg,alpha),MAKEALPHA(sb,db,alpha),MAKEALPHA(sa,da,alpha));
				}
			}
			cpbegin32 += des->texw;
		}
	}

}


void image_to_image_ex(const image_p src,const image_p des,uint32 sx,uint32 sy,uint32 sw,uint32 sh,uint32 dx,uint32 dy)
{
	uint16 *cpbegin16,*bmp16;
	uint32 i,j;
	uint32 *cpbegin32,*bmp32;
	if(sw == 0 && sh == 0){
		sw = src->w;
		sh = src->h;
	}
	if(src->swizzle ==1)
		unswizzle_swap(src);
	if(des->swizzle ==1)
		unswizzle_swap(des);
	des->modified = 1;
	if(des->bpb==2){
		cpbegin16 = (uint16*)des->data+dy*des->texw+dx;
		bmp16 = (uint16*)src->data+sy*src->texw+sx;
		for(i = 0;i<sh;i++){
			for(j = 0;j<sw;j++){
				if((j+dx>des->texw)||(i+dy>des->texh))
					continue;
				cpbegin16[j] = bmp16[j];
			}
			cpbegin16 += des->texw;
			bmp16     += src->texw;
		}		
	}
	else{
		cpbegin32 = (uint32*)des->data+dy*des->texw+dx;
		bmp32 = (uint32*)src->data+sy*src->texw+sx;
		for(i = 0;i<sh;i++){
			for(j = 0;j<sw;j++){
				if((j+dx>=des->texw)||(i+dy>=des->texh))
					continue;
				cpbegin32[j] = bmp32[j];
			}
			cpbegin32 += des->texw;
			bmp32     += src->texw;
		}
	}
}


void image_to_image(const image_p src,const image_p des,uint32 x,uint32 y)
{
	uint32 w = src->w;
	uint32 h = src->h;
	uint16 *cpbegin16,*bmp16;
	uint32 i,j;
	uint32 *cpbegin32,*bmp32;
	if(src->swizzle ==1)
		unswizzle_swap(src);
	if(des->swizzle ==1)
		unswizzle_swap(des);
	des->modified = 1;
	if(des->bpb==2){
		cpbegin16 = (uint16*)des->data+y*des->texw+x;
		bmp16 = (uint16*)src->data;
		for(i =0;i<h;i++){
			for(j =0;j<w;j++){
				if((j+x>des->texw)||(i+y>des->texh))
					continue;
				cpbegin16[j] = bmp16[i*src->texw+j];
			}
			cpbegin16 += des->texw;
		}
	
	}
	else{
		cpbegin32 = (uint32*)des->data+y*des->texw+x;
		bmp32 = (uint32*)src->data;
		for(i =0;i<h;i++){
			for(j =0;j<w;j++){
				if((j+x>=des->texw)||(i+y>=des->texh))
					continue;
				cpbegin32[j] = bmp32[i*src->texw+j];
			}
			cpbegin32 += des->texw;
		}
	}
}

void rawdata_to_image(void* data,const image_p des,uint32 x,uint32 y,uint32 w,uint32 h)
{
	uint16 *cpbegin16,*bmp16;
	uint32 *cpbegin32,*bmp32;
	uint32 i,j;
	if(des->swizzle ==1)
		unswizzle_swap(des);
	des->modified = 1;
	if(des->bpb==2){
		cpbegin16 = (uint16*)des->data+y*des->texw+x;
		bmp16 = (uint16*)data;
		for(i = 0;i<h;i++){
			for(j = 0;j<w;j++){
				if((j+x>des->texw)||(i+y>des->texh))
					continue;
				cpbegin16[j] = bmp16[i*w+j];
			}
			cpbegin16 += des->texw;
		}
	
	}
	else{
		cpbegin32 = (uint32*)des->data+y*des->texw+x;
		bmp32= (uint32*)data;
		for(i = 0;i < h;i++){
			for(j = 0;j < w;j++){
				if((j+x >= des->texw)||(i+y >= des->texh))
					continue;
				cpbegin32[j] = bmp32[i*w+j];
			}
			cpbegin32 += des->texw;
		}
	}
}

int image_fliph(image_p pimage) 
{
	uint32 line,width,height;
	uint8 *new_bits,*bits;
	uint32 y,c;
	if (!pimage) return 0;
	if(pimage->swizzle ==1)
		unswizzle_swap(pimage);
	pimage->modified = 1;
	line   = pimage->texw*pimage->bpb;
	width = pimage->w*pimage->bpb;
	height	= pimage->texh;
	// copy between aligned memories
	new_bits = (uint8*)malloc(line * sizeof(uint8));
	if (!new_bits) return 0;
	// mirror the buffer
	for (y = 0; y < height; y++) {
		bits = pimage->data+y*line;
		memcpy(new_bits, bits, width);
		for(c = 0; c < width; c += pimage->bpb) {
					memcpy(bits + c, new_bits + width - c - pimage->bpb, pimage->bpb);						
		}
	}
	free(new_bits);
	return 1;
}

int image_flipv(image_p pimage)
{
	uint8 *from, *mid;
	uint32 pitch,height,line_s,line_t;
	uint32 y;
	
	if (!pimage) return 0;
	if(pimage->swizzle ==1)
		unswizzle_swap(pimage);
	pimage->modified = 1;
	// swap the buffer
	pitch  = pimage->texw*pimage->bpb;
	height = pimage->texh;
	// copy between aligned memories
	mid = (uint8*)malloc(pitch * sizeof(char));
	if (!mid) return 0;
	from = pimage->data;
	line_s = 0;
	line_t = (height-1) * pitch;
	for(y = 0; y < height/2; y++) {
		memcpy(mid, from + line_s, pitch);
		memcpy(from + line_s, from + line_t, pitch);
		memcpy(from + line_t, mid, pitch);
		line_s += pitch;
		line_t -= pitch;
	}
	free(mid);
	
	return 1;
}



