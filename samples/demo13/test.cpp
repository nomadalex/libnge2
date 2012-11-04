#include "libnge2.h"
#include "nge_app.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * 测试音频接口
 */

#define RES_PATH(path) (path)

int game_quit = 0;

int step = 0;

audio_play_p audio[3];
bool audio_playing[3];

BOOL startTouch = FALSE;

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
		//O键恢复到开头
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
#endif

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

int pause()
{
	for (int i = 0; i < 3; i++) {
		if (audio_playing[i])
			audio[i]->pause(audio[i]);
	}
	nge_print("notify pause\n");
	return 0;
}

int resume()
{
	for (int i = 0; i < 3; i++) {
		if (audio_playing[i])
			audio[i]->resume(audio[i]);
	}
	nge_print("notify resume\n");
	return 0;
}

int init()
{
	NGE_Init(INIT_ALL);

#ifdef NGE_INPUT_BUTTON_SUPPORT
	InitInput(btn_down,btn_up,1);
#endif

#ifdef NGE_INPUT_MOUSE_SUPPORT
	InitMouse(mouseButtonProc, NULL);
#endif

	//载入3声音一会播放
	//0-mp3
	audio[0] = CreateMp3Player();
	audio[0]->load(audio[0], RES_PATH("music/simple1.mp3"));
	//1-wav
	audio[1] = CreateWavPlayer();
	audio[1]->load(audio[1], RES_PATH("music/simple3.wav"));
	//2-ogg
	audio[2] = CreateOggPlayer();
	audio[2]->load(audio[2], RES_PATH("music/test.ogg"));

	for (int i = 0; i < 3; i++) {
		audio_playing[i] = false;
	}

	step = 1;

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
		audio_playing[2] = true;
		step = 2;
		break;

	case 2:
		if (audio[2]->iseof(audio[2])) {
			audio_playing[2] = false;
			step = 3;
		}
		break;

	case 3:
		// play wav 2 times and wait end
		audio[1]->play(audio[1], 2, 0);
		audio_playing[1] = true;
		step = 4;
		break;

	case 4:
		if (audio[1]->iseof(audio[1])) {
			audio_playing[1] = false;
			step = 5;
		}
		break;

	case 5:
		audio[0]->play(audio[0], 1, 0);
		audio[2]->playstop(audio[2]);
		audio[1]->play(audio[1], 1, 0);
		audio_playing[0] = true;
		audio_playing[1] = true;
		audio_playing[2] = true;
		startTouch = TRUE;
		step = 7;
		break;

	case 6:
		if (audio[1]->iseof(audio[1])) {
			audio[1]->playstop(audio[1]);
		}
		// for test seek
		// audio[0]->seek(audio[0], -3000, AUDIO_SEEK_CUR);
		// nge_print("seek -3000 ms\n");
		step = 7;
		break;

	case 7:
		for (int i = 0; i < 3; i++) {
			if (audio_playing[i])
				if (audio[i]->iseof(audio[i]))
					audio_playing[i] = false;
		}
		break;
	}

	InputProc();

	return NGE_APP_NORMAL;
}

int fini()
{
	//释放声音资源
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
	nge_init_app(&app);

	app.init = init;
	app.mainloop = mainloop;
	app.fini = fini;

	app.pause = pause;
	app.resume = resume;

	nge_register_app(&app);
	return 0;
}
