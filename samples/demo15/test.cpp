#include "libnge2.h"
#include "hgedistort.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
/**
 * nge_test:nge程序:图片扭曲
 */
//退出标识
int game_quit = 0;
//背景图片
image_p p_bg = NULL;
//logo图片
image_p p_logo  = NULL;
hgeDistortionMesh* mDistortionMesh;

nge_timer* timer;
//last tick;
int mlast = 0;

const int nRows=16;
const int nCols=16;

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
		//按下选择键退出
		game_quit = 1;
		break;
	case PSP_BUTTON_START:
		//按下开始键退出
		game_quit = 1;
		break;
    }
}

void Update()
{
	int i, j;
	mlast = timer->get_ticks(timer);
	for(i=1;i<nRows-1;i++)
		for(j=1;j<nCols-1;j++)
		{
			mDistortionMesh->SetDisplacement(j,i,cosf(mlast/100.0+(i+j)/2)*5,sinf(mlast/100.0+(i+j)/2)*5,HGEDISP_NODE);
		}
}

void DrawScene()
{
	BeginScene(1);
	ImageToScreen(p_bg, 0, 0);
	mDistortionMesh->Render(20,20);
	EndScene();
}

extern "C"
int main(int argc, char* argv[])
{
	//初始化NGE分为VIDEO,AUDIO，这里是只初始化VIDEO，如果初始化所有用INIT_VIDEO|INIT_AUDIO,或者INIT_ALL
	NGE_Init(INIT_VIDEO);
	//初始化按键处理btn_down是按下响应,后面是弹起时的响应，0是让nge处理home消息(直接退出),填1就是让PSP系统处理
	//home消息,通常填1正常退出（1.50版的自制程序需要填0）
	InitInput(btn_down,NULL,1);
	//最后一个参数是psp swizzle优化，通常填1
	p_bg = image_load("images/demo0.jpg",DISPLAY_PIXEL_FORMAT_8888,1);
	if(p_bg == NULL) {
		printf("can not open file\n");
	}
	p_logo = image_load("images/nge2logo.png",DISPLAY_PIXEL_FORMAT_8888,1);
	if(p_logo == NULL) {
		printf("can not open file\n");
	}
	//初始化扭曲设置
	mDistortionMesh = new hgeDistortionMesh(nCols, nRows);
	mDistortionMesh->SetTexture(p_logo);
	mDistortionMesh->SetTextureRect(0,0,p_logo->w,p_logo->h);
	mDistortionMesh->Clear(0xFFFFFFFF);
	timer = timer_create();
	timer->start(timer);
	
	while ( !game_quit )
	{
		ShowFps();
		InputProc();
		Update();
		DrawScene();
		LimitFps(60);
	}
	image_free(p_logo);
	image_free(p_bg);
	SAFE_FREE(mDistortionMesh);
	delete mDistortionMesh;
	NGE_Quit();
	return 0;
}
