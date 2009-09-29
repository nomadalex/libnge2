#include "libnge2.h"
#include "cAnimation.h"
/**
 * nge_test:最简单的nge程序:显示一张图片
 */

//退出标识
int game_quit = 0;
//背景图片
image_p p_bg = NULL;







extern "C"
int main(int argc, char* argv[])
{
	//初始化NGE分为VIDEO,AUDIO，这里是只初始化VIDEO，如果初始化所有用INIT_VIDEO|INIT_AUDIO,或者INIT_ALL
	NGE_Init(INIT_VIDEO);
	
	//最后一个参数是psp swizzle优化，通常填1
	p_bg = image_load("images/demo4_bg.png",DISPLAY_PIXEL_FORMAT_8888,1);
	if(p_bg == NULL)
		printf("can not open file!\n");
	CAnimation* ani = new CAnimation("images/demo4_man.png");
	while ( !game_quit )
	{
		ShowFps();
		ani->Input();
		ani->Update();
		//draw target
		BeginScene(1);
			ImageToScreen(p_bg,0,0);
			ani->Show();
		EndScene();
		LimitFps(60);
	}
	image_free(p_bg);
	NGE_Quit();
	delete ani;
	return 0;
}



