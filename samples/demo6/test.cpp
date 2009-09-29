#include "libnge2.h"
/**
 * nge_gif:显示一张gif图片
 */
//退出标识
int game_quit = 0;
//背景图片
image_p p_bg = NULL;
//logo图片
image_p p_logo  = NULL;

enum{
	SCREEN_1X= 0,//原图显示
	SCREEN_FULL, //满屏显示
	SCREEN_2X    //二倍显示
};

gif_desc_p pgif = NULL;
int display_flag = SCREEN_1X;


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
		display_flag = SCREEN_2X;
		break;
	case PSP_BUTTON_CIRCLE:
		display_flag = SCREEN_FULL;
		break;
	case PSP_BUTTON_CROSS:
		display_flag = SCREEN_1X;
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



void DrawScene()
{
	BeginScene(1);
	ImageToScreen(p_bg,0,0);
	switch(display_flag)
	{
		case SCREEN_FULL:
			DrawGifAnimation(pgif,0,0,0,0,0,0,480,272);
			break;
		case SCREEN_1X:
			GifAnimationToScreen(pgif,0,0);
			break;
		case SCREEN_2X:
			RenderGifAnimation(pgif,0,0,0,0,0,0,2,2,0,pgif->gif_image_chains->pimage->mask);
			break;
		default:
			break;
	}
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
	if(p_bg == NULL)
		printf("can not open file\n");
	pgif = gif_animation_load("images/simple.gif",DISPLAY_PIXEL_FORMAT_5551,1);
	if(pgif == NULL)
		printf("can not open file\n");
	while ( !game_quit )
	{
		ShowFps();
		InputProc();
		DrawScene();
		LimitFps(60);
	}
	image_free(p_bg);
	image_free(p_logo);
	gif_animation_free(pgif);
	NGE_Quit();
	return 0;
}



