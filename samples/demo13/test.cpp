#include "libnge2.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "nge_app.h"
/**
 * nge_test:����nge2�İ�����������
 * �������µ���Ƶ�ӿ�~
 */

#ifdef ANDROID
#define RES_PATH(path) ("/sdcard/libnge2/demo13/" path)
#else
#define RES_PATH(path) (path)
#endif

int game_quit = 0;
image_p pimage_text,pimage_bg,pimage_box;
image_p pimage_icon[2];
//���ֲ���

//��ʼ����Ϊ128������ΧΪ��0-128��
int volume = 128;

int mask8888 = MAKE_RGBA_8888(255,255,255,255);
int mask4444 = MAKE_RGBA_4444(255,255,255,255);
//��͸����box
int maskbox = MAKE_RGBA_8888(255,255,255,128);
int wav_ret;

audio_play_p audio[3];
PFont pf[2];
static BOOL startTouch = FALSE;

#ifdef NGE_INPUT_BUTTON_SUPPORT
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
		//O���ָ�����ͷ
		if(audio[1]->iseof(audio[1])==1)
			audio[1]->rewind(audio[1]);
		audio[1]->play(audio[1],1,0);
		break;
	case PSP_BUTTON_CROSS:
		break;
    case PSP_BUTTON_SQUARE:
		break;
	case PSP_BUTTON_SELECT:
		break;
	case PSP_BUTTON_START:
		//��ʼ���˳�
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
#endif

void DrawScene()
{

	BeginScene(1);
	ImageToScreen(pimage_bg,0,0);

	RenderQuad(pimage_box,0,0,0,0,70,185,1,1,0,maskbox);
	RenderQuad(pimage_box,0,0,0,0,70,5,1,1,0,maskbox);
	DrawImage(pimage_icon[0],0,0,0,0,-20,-37,128,128);
	DrawImage(pimage_icon[1],0,0,0,0,-20,143,128,128);
	ImageToScreen(pimage_text,0,0);

	//����2��ȼ�������2��
	//RenderQuad(bg,0,0,0,0,0,0,1,1,0,mask8888);
	//RenderQuad(pimage_text,0,0,0,0,0,0,1,1,0,mask4444);
	EndScene();
}

static int step = 0;
static int temp_count = 0;

#ifdef NGE_INPUT_MOUSE_SUPPORT
void mouseButtonProc(int type, int x, int y)
{
	switch (type) {
	case MOUSE_LBUTTON_DOWN:
		if (startTouch)
			step = 6;
		nge_print("mouse lbutton down\n");
		break;
	}
}
#endif

int init()
{
	NGE_Init(INIT_ALL);
	//audio_play_p mp3 = CreateMp3Player();

#ifdef NGE_INPUT_BUTTON_SUPPORT
	InitInput(btn_down,btn_up,1);
#endif

#ifdef NGE_INPUT_MOUSE_SUPPORT
	InitMouse(mouseButtonProc, NULL);
#endif

	int i;
	//����һ����ʾimage,�־���ʾ���������ע��DISPLAY_PIXEL_FORMAT�����봴�������DISPLAY_PIXEL_FORMATһ��
	pimage_text = image_create(512,512,DISPLAY_PIXEL_FORMAT_4444);
	//��������
	pf[0] = create_font_hzk(RES_PATH("fonts/GBK14"),RES_PATH("fonts/ASC14"),14,DISPLAY_PIXEL_FORMAT_4444);
	pf[1] = create_font_freetype(RES_PATH("fonts/simfang.ttf"),13,DISPLAY_PIXEL_FORMAT_4444);
	char str1[3][128]={"����ĻԱ��","���沥�ŵ�����ѩ���ݳ�����������","��ӭ���������"};
	char str2[3][128]={"��ѩ���˿��","�����ۡ�����̫���ˣ�","XXXXŮ��SAMA���Ұ��㡣"};
	//��ʾGBK Font
	font_setcolor(pf[0],MAKE_RGBA_4444(128,0,0,255));
	font_drawtext(pf[0],str2[0],strlen(str2[0]),pimage_text,100,195,FONT_SHOW_NORMAL);
	for(i = 1;i<3;i++){
		font_drawtext(pf[0],str2[i],strlen(str2[i]),pimage_text,120,200+i*20,FONT_SHOW_SHADOW);
		font_setcolor(pf[0],MAKE_RGBA_4444(255,0,0,255));
	}
	//��ʾfreetype
	font_setcolor(pf[1],MAKE_RGBA_4444(128,0,0,255));
	font_drawtext(pf[1],str1[0],strlen(str1[0]),pimage_text,100,30,FONT_SHOW_NORMAL);
	for(i =1;i<3;i++){
		font_drawtext(pf[1],str1[i],strlen(str1[i]),pimage_text,120,35+i*20,FONT_SHOW_NORMAL);
		font_setcolor(pf[1],MAKE_RGBA_4444(255,0,0,255));
	}

	pimage_bg = image_load(RES_PATH("images/demo2_bg.jpg"),DISPLAY_PIXEL_FORMAT_8888,1);
	if(pimage_bg == NULL)
		printf("can not open file\n");
	pimage_box = image_load(RES_PATH("images/demo2_box.jpg"),DISPLAY_PIXEL_FORMAT_8888,1);
	if(pimage_box == NULL)
		printf("can not open file\n");
	pimage_icon[0] = image_load_colorkey(RES_PATH("images/demo2_icon1.png"),DISPLAY_PIXEL_FORMAT_8888,MAKE_RGB(0,0,0),1);
	pimage_icon[1] = image_load_colorkey(RES_PATH("images/demo2_icon0.bmp"),DISPLAY_PIXEL_FORMAT_8888,MAKE_RGB(0,0,0),1);

	//����3����һ�Ქ��
	//0-mp3
	audio[0] = CreateMp3Player();
	audio[0]->load(audio[0], RES_PATH("music/simple1.mp3"));
	//1-wav
	audio[1] = CreateWavPlayer();
	audio[1]->load(audio[1], RES_PATH("music/simple3.wav"));
	//2-ogg
	audio[2] = CreateOggPlayer();
	audio[2]->load(audio[2], RES_PATH("music/test.ogg"));

	step = 1;
	temp_count = 60*100;

	return 0;
}

int mainloop()
{
	if (game_quit)
		return NGE_APP_QUIT;

	switch (step) {
	case 0:
		break;

	case 1:
		// play ogg and wait end
		audio[2]->play(audio[2], 1, 0);
		step = 2;
		break;

	case 2:
		if (audio[2]->iseof(audio[2]))
			step = 3;
		break;

	case 3:
		// play wav 2 times and wait end
		audio[1]->play(audio[1], 2, 0);
		step = 4;
		break;

	case 4:
		if (audio[1]->iseof(audio[1]))
			step = 5;
		break;

	case 5:
		audio[0]->play(audio[0], 1, 0);
		audio[2]->playstop(audio[2]);
		audio[1]->play(audio[1], 1, 0);
		startTouch = TRUE;
		step = 0;
		break;

	case 6:
		audio[0]->seek(audio[0], -3000, AUDIO_SEEK_CUR);
		nge_print("seek -3000 ms\n");
		step = 0;
		break;
	}

	//ShowFps();
	InputProc();
//	DrawScene();
	LimitFps(60);

	return NGE_APP_NORMAL;
}

int fini()
{
	font_destory(pf[0]);
	font_destory(pf[1]);
	image_free(pimage_bg);
	image_free(pimage_text);
	image_free(pimage_box);
	//�ͷ�������Դ
	audio[0]->destroy(audio[0]);
	audio[1]->destroy(audio[1]);
	audio[2]->destroy(audio[2]);
	NGE_Quit();

	return 0;
}

static nge_app_t app;

extern "C"
int main(int argc, char* argv[])
{
	app.init = init;
	app.mainloop = mainloop;
	app.fini = fini;
	nge_register_app(&app);
	return 0;
}