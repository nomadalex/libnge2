#include "libnge2.h"
#include "hgedistort.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/**
 * nge_test:nge程序:图片扭曲
 */

#define RES_PATH(path) (path)

//退出标识
int game_quit = 0;
//背景图片
image_p p_bg = NULL;
//logo图片
image_p p_logo  = NULL;
hgeDistortionMesh* mDistortionMesh;

nge_timer* timer;
//last tick;以秒为单位
float mlast = 0;

const int nRows=16;
const int nCols=16;

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
		break;
	case PSP_BUTTON_CROSS:
		break;
    case PSP_BUTTON_SQUARE:
		break;
	case PSP_BUTTON_SELECT:
		break;
	case PSP_BUTTON_START:
		//按下开始键退出
		game_quit = 1;
		break;
    }
}
#endif

void Update()
{
	int i, j;
	mlast = timer->get_ticks(timer)/1000.0;
	for(i=1;i<nRows-1;i++)
		for(j=1;j<nCols-1;j++)
		{
			mDistortionMesh->SetDisplacement(j,i,cosf(10.0*mlast+(i+j)/2)*5,sinf(10.0*mlast+(i+j)/2)*5,HGEDISP_NODE);
		}
}

void DrawScene()
{
	BeginScene(1);
	ImageToScreen(p_bg, 0, 0);
	mDistortionMesh->Render(20,20);
	EndScene();
}

int init() {
	//初始化NGE分为VIDEO,AUDIO，这里是只初始化VIDEO，如果初始化所有用INIT_VIDEO|INIT_AUDIO,或者INIT_ALL
	NGE_Init(INIT_VIDEO);
	//初始化按键处理btn_down是按下响应,后面是弹起时的响应，0是让nge处理home消息(直接退出),填1就是让PSP系统处理
	//home消息,通常填1正常退出（1.50版的自制程序需要填0）
#ifdef NGE_INPUT_BUTTON_SUPPORT
	InitInput(btn_down,NULL,1);
#endif
	p_bg = image_load(RES_PATH("images/demo0.jpg"),DISPLAY_PIXEL_FORMAT_8888,1);
	if(p_bg == NULL) {
		printf("can not open file\n");
	}
	p_logo = image_load(RES_PATH("images/nge2logo.png"),DISPLAY_PIXEL_FORMAT_8888,1);
	if(p_logo == NULL) {
		printf("can not open file\n");
	}
	//初始化扭曲设置
	mDistortionMesh = new hgeDistortionMesh(nCols, nRows);
	mDistortionMesh->SetTexture(p_logo);
	mDistortionMesh->SetTextureRect(0,0,p_logo->w,p_logo->h);
	mDistortionMesh->Clear(0xFFFFFFFF);
	timer = nge_timer_create();
	timer->start(timer);

	return 0;
}

int mainloop() {
	if (game_quit)
		return NGE_APP_QUIT;

	ShowFps();
#ifdef NGE_INPUT_HAS_PROC
	InputProc();
#endif
	Update();
	DrawScene();
	LimitFps(60);

	return NGE_APP_NORMAL;
}

int fini() {
	image_free(p_logo);
	p_logo = NULL;
	image_free(p_bg);
	p_bg = NULL;
	delete mDistortionMesh;
	mDistortionMesh = NULL;
	nge_timer_free(timer);
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
	nge_register_app(&app);
	return 0;
}
