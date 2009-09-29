#include "hge/hgeparticle.h"
#include "libnge2.h"

/**
 * nge_test:nge程序:显示例子
 */
//退出标识
int game_quit = 0;
//背景图片
image_p p_logo = NULL;
image_p p_par = NULL;
hgeParticleSystem*	mParticleSys;
sprite_p mParticle = NULL;

nge_timer* timer;
//last tick;	
int mlast = 0;
//dt秒为单位
float dt = 0;


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
	//注意,传给；粒子系统update的是秒.所以需要除1000
	float dt = (float)((timer->get_ticks(timer)-mlast)/1000.0);
	mlast = timer->get_ticks(timer);
	mParticleSys->Update(dt);
}

void DrawScene()
{
	BeginScene(1);
	RenderQuad(p_logo,0,0,0,0,(480-p_logo->w)/2,(272-p_logo->h)/2,1,1,0,MAKE_RGBA_8888(255,255,255,40));
	mParticleSys->Render();
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
	p_logo = image_load("images/nge2logo.png",DISPLAY_PIXEL_FORMAT_8888,1);
	if(p_logo == NULL)
		printf("can not open file\n");
	p_par = image_load("par/particles.png",DISPLAY_PIXEL_FORMAT_8888,1);
	if(p_par == NULL)
		printf("can not open file\n");
	//设置sprite子图
	mParticle = (sprite_p)malloc(sizeof(sprite_t));
	memset(mParticle,0,sizeof(sprite_t));
	mParticle->sprite_image = p_par;
	mParticle->sprite_clip.left= 0.0f;
	mParticle->sprite_clip.top= 0.0f;
	mParticle->sprite_clip.right= 32.0f;
	mParticle->sprite_clip.bottom = 32.0f;
	mParticle->sprite_center.x = 16.0f; 
	mParticle->sprite_center.y = 16.0f;

	mParticleSys = new hgeParticleSystem("par/particle1.psi", mParticle);
	mParticleSys->MoveTo(480.0f/2, 272.0f/2,0);
	mParticleSys->Fire();
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
	image_free(p_par);
	SAFE_FREE(mParticle);
	delete mParticleSys;
	NGE_Quit();
	return 0;
}



