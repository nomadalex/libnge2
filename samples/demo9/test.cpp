#include "libnge2.h"
#include "nge_jpg.h"
#include "unzip.h"


PFont pf[1];
int game_quit = 0;
image_p pimage_text;

image_p pimage_jpg = NULL;


char filename[1024][256];
int filenum=0;


//txt文件
char txtfilename[256];
char txt[1024]={0};
int txtsize = 0;

//img文件
char imagefilename[256];
	char *imagebuf;
	int imagesize;


//获取zip 内文件 目录
int open_zip_filename(char* zipfile)
{
	unzFile unzf = unzOpen(zipfile);

	if(unzf == NULL)
		return 0;

	if(unzGoToFirstFile(unzf) != UNZ_OK)
	{
		unzClose(unzf);
		return 1;
	}



	do
	{
		char fname[256];
		unz_file_info file_info;

		if(unzGetCurrentFileInfo(unzf, &file_info, fname, 256, NULL, 0, NULL, 0) != UNZ_OK)
			break;

		strcpy(filename[filenum],fname);
		filenum++;

	} while(unzGoToNextFile(unzf) == UNZ_OK);


	unzClose(unzf);

	//显示文件名
	for	(int i =0;i<filenum;i++)
	{
		font_drawtext(pf[0],filename[i],strlen(filename[i]),pimage_text,0,0+i*20,FONT_SHOW_NORMAL);

	}


	return 1;
}

//获取zip 内txt文件
int open_zip_file(char* zipfile,char *zipfilename)
{

	unzFile unzf = unzOpen(zipfile);

	if(unzf == NULL)
	{
		return 0;
	}
	if(unzLocateFile(unzf, zipfilename, 0) != UNZ_OK || unzOpenCurrentFile(unzf) != UNZ_OK)
	{
		unzClose(unzf);
		return 0;
	}
	strcpy(txtfilename, zipfilename);

	unz_file_info info;

	if(unzGetCurrentFileInfo(unzf, &info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK)
	{
		unzCloseCurrentFile(unzf);
		unzClose(unzf);
		return 0;
	}
	txtsize = info.uncompressed_size;


	txtsize = unzReadCurrentFile(unzf, txt, txtsize);
	unzCloseCurrentFile(unzf);
	unzClose(unzf);


	font_drawtext(pf[0],txt,strlen(txt),pimage_text,200,10,FONT_SHOW_NORMAL);


	return 1;
}


//获取zip 内image文件
int open_zip_image(char* zipfile,char *zipimagename)
{



	unzFile unzf = unzOpen(zipfile);

	if(unzf == NULL)
	{
		return 0;
	}
	if(unzLocateFile(unzf, zipimagename, 0) != UNZ_OK || unzOpenCurrentFile(unzf) != UNZ_OK)
	{
		unzClose(unzf);
		return 0;
	}

	strcpy(imagefilename, zipimagename);

	unz_file_info info;

	if(unzGetCurrentFileInfo(unzf, &info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK)
	{
		unzCloseCurrentFile(unzf);
		unzClose(unzf);
		return 0;
	}

	imagesize = info.uncompressed_size;

	imagebuf = (char*) malloc(imagesize);

	imagesize = unzReadCurrentFile(unzf, imagebuf, imagesize);


	pimage_jpg = image_load_jpg_buf(imagebuf,imagesize,DISPLAY_PIXEL_FORMAT_8888);

	unzCloseCurrentFile(unzf);
	unzClose(unzf);


	free(imagebuf);


	return 1;
}


void btn_down(int keycode)
{
	switch(keycode)
	{
	case PSP_BUTTON_UP:
		break;
	case PSP_BUTTON_DOWN:
		break;
	case PSP_BUTTON_LEFT:
		break;
	case PSP_BUTTON_RIGHT:
		break;
	case PSP_BUTTON_TRIANGLE:
		break;
	case PSP_BUTTON_CIRCLE:
		break;
	case PSP_BUTTON_CROSS:
		break;
	case PSP_BUTTON_SQUARE:
		break;
	case PSP_BUTTON_SELECT:
		break;
	case PSP_BUTTON_START:
		//开始键退出
		game_quit = 1;
		break;
	}
}

void btn_up(int keycode)
{
	switch(keycode)
	{
	case PSP_BUTTON_UP:
		break;
	case PSP_BUTTON_DOWN:
		break;
	case PSP_BUTTON_LEFT:
		break;
	case PSP_BUTTON_RIGHT:
		break;
	case PSP_BUTTON_TRIANGLE:
		break;
	case PSP_BUTTON_CIRCLE:
		break;
	case PSP_BUTTON_CROSS:
		break;
	case PSP_BUTTON_SQUARE:
		break;
	case PSP_BUTTON_SELECT:
		break;
	case PSP_BUTTON_START:
		break;
	}
}

void DrawScene()
{
	BeginScene(1);
	if(pimage_jpg)
		ImageToScreen(pimage_jpg,0,0);
	if(pimage_text)
		ImageToScreen(pimage_text,0,0);

	EndScene();
}

extern "C"
int main(int argc, char* argv[])
{
	NGE_Init(INIT_ALL);


	InitInput(btn_down,btn_up,1);

	//创建一个显示image,字就显示在这个上面注意DISPLAY_PIXEL_FORMAT必须与创建字体的DISPLAY_PIXEL_FORMAT一致
	pimage_text = image_create(512,512,DISPLAY_PIXEL_FORMAT_4444);

	//创建字体
	pf[0] = create_font_hzk("fonts/GBK12","fonts/ASC12",12,DISPLAY_PIXEL_FORMAT_4444);

	//显示GBK Font
	font_setcolor(pf[0],MAKE_RGBA_4444(0,0,0,255));

	open_zip_filename("database/data.zip");

	open_zip_file("database/data.zip","1.txt");

	open_zip_image("database/data.zip","1.jpg");

	while ( !game_quit )
	{
		//ShowFps();
		InputProc();
		DrawScene();
		LimitFps(60);
	}
	font_destory(pf[0]);

	image_free(pimage_jpg);
	image_free(pimage_text);
	NGE_Quit();
	return 0;
}
