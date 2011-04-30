#include "libnge2.h"
#include <string.h>

/**
 * nge_test:测试nge2的按键输入字体
 */

int game_quit = 0;
image_p pimage_text,pimage_bg,pimage_box;
image_p pimage_icon[2];

int mask8888 = MAKE_RGBA_8888(255,255,255,255);
int mask4444 = MAKE_RGBA_4444(255,255,255,255);
//半透明的box
int maskbox = MAKE_RGBA_8888(255,255,255,128);

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
		game_quit = 1;
		break;
	case PSP_BUTTON_START:
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
		game_quit = 1;
		break;
	case PSP_BUTTON_START:
		game_quit = 1;
		break;
    }
}

void DrawScene()
{

	BeginScene(1);
	ImageToScreen(pimage_bg,0,0);

	RenderQuad(pimage_box,0,0,0,0,70,185,1,1,0,maskbox);
	RenderQuad(pimage_box,0,0,0,0,70,5,1,1,0,maskbox);
	DrawImage(pimage_icon[0],0,0,0,0,-20,-37,128,128);
	DrawImage(pimage_icon[1],0,0,0,0,-20,143,128,128);
	ImageToScreen(pimage_text,0,0);

	EndScene();
}

extern "C"
int main(int argc, char* argv[])
{
	PFont pf[2] ;
	int i;

	NGE_Init(INIT_VIDEO);
	InitInput(btn_down,btn_up,1);

	//创建一个显示image,字就显示在这个上面注意DISPLAY_PIXEL_FORMAT必须与创建字体的DISPLAY_PIXEL_FORMAT一致
	pimage_text = image_create(512,512,DISPLAY_PIXEL_FORMAT_4444);
	//创建字体
	pf[0] = create_font_hzk("fonts/GBK14","fonts/ASC14",14,DISPLAY_PIXEL_FORMAT_4444);
	pf[1] = create_font_freetype("fonts/simfang.ttf",13,DISPLAY_PIXEL_FORMAT_4444);
	char str[3][128]={"【小萝莉】","众芳摇落独暄妍，占尽风情向小园。","疏影横斜水清浅，暗香浮动月黄昏。"};
	//显示GBK Font
	font_setcolor(pf[0],MAKE_RGBA_4444(128,0,0,255));
	font_drawtext(pf[0],str[0],strlen(str[0]),pimage_text,100,195,FONT_SHOW_NORMAL);
	for(i = 1;i<3;i++){
		font_drawtext(pf[0],str[i],strlen(str[i]),pimage_text,120,200+i*20,FONT_SHOW_SHADOW);
		font_setcolor(pf[0],MAKE_RGBA_4444(255,0,0,255));
	}
	//显示freetype
	font_setcolor(pf[1],MAKE_RGBA_4444(128,0,0,255));
	font_drawtext(pf[1],str[0],strlen(str[0]),pimage_text,100,30,FONT_SHOW_NORMAL);
	for(i =1;i<3;i++){
		font_drawtext(pf[1],str[i],strlen(str[i]),pimage_text,120,35+i*20,FONT_SHOW_NORMAL);
		font_setcolor(pf[1],MAKE_RGBA_4444(255,0,0,255));
	}
	pimage_bg = image_load("images/demo1_bg.jpg",DISPLAY_PIXEL_FORMAT_8888,1);
	if(pimage_bg == NULL)
		printf("can not open file\n");
	pimage_box = image_load("images/demo1_box.jpg",DISPLAY_PIXEL_FORMAT_8888,1);
	if(pimage_box == NULL)
		printf("can not open file\n");
	pimage_icon[0] = image_load_colorkey("images/demo1_icon0.bmp",DISPLAY_PIXEL_FORMAT_8888,MAKE_RGB(0,0,0),1);
    pimage_icon[1] = image_load_colorkey("images/demo1_icon1.png",DISPLAY_PIXEL_FORMAT_8888,MAKE_RGB(0,0,0),1);
	while ( !game_quit )
	{
		ShowFps();
		InputProc();
		DrawScene();
	}
	font_destory(pf[0]);
	font_destory(pf[1]);
	image_free(pimage_bg);
	image_free(pimage_text);
	image_free(pimage_box);
	NGE_Quit();
	return 0;
}
