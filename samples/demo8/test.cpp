#include "libnge2.h"
/**
 * nge_test:Polygons
 */

//退出标识
int game_quit = 0;

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
		//按下三角键截图
		ScreenShot("screenshot.png");
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

int colors[]={
	MAKE_RGBA_8888(255,0,0,255),//红
	MAKE_RGBA_8888(255,128,0,255),//橙
	MAKE_RGBA_8888(255,255,0,255),//黄
	MAKE_RGBA_8888(0,255,0,255),//绿
	MAKE_RGBA_8888(0,128,255,255),//青
	MAKE_RGBA_8888(0,0,255,255),//蓝
	MAKE_RGBA_8888(195,0,255,255)//紫
};

//下面DrawPolygon画个用平行4边形
//  -------------------\
//   \                  \
//    \                  \
//     \------------------\
//
float Polygons_x1[]={
	200.0f,
	300.0f,
	400.0f,
	300.0f
};
float Polygons_y1[]={
	10.0f,
	10.0f,
	70.0f,
	70.0f
};
//下面用FillPolygon画个三角形
float Polygons_x2[]={
	200.0f,
	300.0f,
	400.0f
};
float Polygons_y2[]={
	80.0f,
	80.0f,
	160.0f
};
//
//下面用FillPolygonGrad画个渐进三角形
float Polygons_x3[]={
	400.0f,
	450.0f,
	350.0f
};
float Polygons_y3[]={
	160.0f,
	250.0f,
	250.0f
};
//颜色
int tri_color[]={
	MAKE_RGBA_8888(255,0,0,255),//红
	MAKE_RGBA_8888(255,255,0,255),//黄
	MAKE_RGBA_8888(0,0,255,255)//蓝
};

void DrawScene()
{

	BeginScene(1);
	DrawLine(0,0,480,272,colors[0],DISPLAY_PIXEL_FORMAT_8888);
	DrawRect(20,20,200,200,colors[1],DISPLAY_PIXEL_FORMAT_8888);
	FillRect(40,40,100,100,colors[2],DISPLAY_PIXEL_FORMAT_8888);
	DrawCircle(100,100,20,colors[3],DISPLAY_PIXEL_FORMAT_8888);
	FillCircle(150,150,20,colors[4],DISPLAY_PIXEL_FORMAT_8888);
	DrawEllipse(200,200,20,40,colors[5],DISPLAY_PIXEL_FORMAT_8888);
	FillEllipse(250,250,40,20,colors[6],DISPLAY_PIXEL_FORMAT_8888);
	DrawPolygon(Polygons_x1,Polygons_y1,4,colors[0],DISPLAY_PIXEL_FORMAT_8888);
	FillPolygon(Polygons_x2,Polygons_y2,3,colors[1],DISPLAY_PIXEL_FORMAT_8888);
	FillPolygonGrad(Polygons_x3,Polygons_y3,3,tri_color,DISPLAY_PIXEL_FORMAT_8888);
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
	//设置屏幕颜色为白色
	SetScreenColor(255,255,255,255);
	while ( !game_quit )
	{
		ShowFps();
		InputProc();
		DrawScene();
		//限制fps为60
		LimitFps(60);
	}
	NGE_Quit();
	return 0;
}



