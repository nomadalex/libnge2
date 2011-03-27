#include "nge_input.h"

#if defined(_PSP)
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>

//define in nge_main.c
extern int cbid;

#else // win32 and iphone and linux
SDL_Joystick* joystick;
SDL_Event event;
//win32Ä£ÄâÒ¡¸Ë
#define WIN_ANALOG_LEFT  0
#define WIN_ANALOG_RIGHT 1
#define WIN_ANALOG_UP    2
#define WIN_ANALOG_DOWN  3
static char btn_analog[4] = {0};

#endif

static void btn_down_default(int keycode);
static void btn_up_default(int keycode);

static ButtonProc btn_down = btn_down_default;
static ButtonProc btn_up   = btn_up_default;
static AnalogProc analog_proc = NULL;
static MouseMoveProc mouse_move_proc = NULL;
static MouseButtonProc mouse_btn_proc = NULL;

static TouchMoveProc touch_move_proc = NULL;
static TouchButtonProc touch_button_proc = NULL;

static int game_quit = 0;
static int need_swapxy = 0;
static int touched = 0;
static int touch_mode = 0;

void EmulateTouchMove(int flag)
{
	if(flag == 1)
		touch_mode  = 1;
	else
		touch_mode  = 0;
}

void btn_down_default(int keycode)
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
	case PSP_BUTTON_HOME:
		break;
    }
}

void SetSwapXY(int flag)
{
#ifdef IPHONEOS
	if(flag)
		need_swapxy = 1;
#endif
}

void btn_up_default(int keycode)
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
	case PSP_BUTTON_HOME:
		break;
    }
}

#if defined WIN32 ||  defined IPHONEOS || defined __linux__
uint8 GetAnalogX()
{
	if (btn_analog[WIN_ANALOG_LEFT]) return 0;
	if (btn_analog[WIN_ANALOG_RIGHT]) return 0xff;

	return 0x80;
}


uint8 GetAnalogY()
{
	if (btn_analog[WIN_ANALOG_UP]) return 0;
	if (btn_analog[WIN_ANALOG_DOWN]) return 0xff;

	return 0x80;
}
static int SetAnalog(int key,char flag)
{
	int ret = 0;
	switch(key)
	{
		case SDLK_UP:
			btn_analog[WIN_ANALOG_UP] = flag;
			ret = 1;
			break;
		case SDLK_DOWN:
			btn_analog[WIN_ANALOG_DOWN] = flag;
			ret = 1;
			break;
		case SDLK_LEFT:
			btn_analog[WIN_ANALOG_LEFT] = flag;
			ret = 1;
			break;
		case SDLK_RIGHT:
			btn_analog[WIN_ANALOG_RIGHT] = flag;
			ret = 1;
			break;
		default:
			break;
	}
	return ret;
}

void InitMouse(MouseButtonProc mouse_btn,MouseMoveProc mouse_move)
{
	if(mouse_move != NULL)
		mouse_move_proc = mouse_move;
	if(mouse_btn != NULL)
		mouse_btn_proc = mouse_btn;
	touched = 0;
}

void InitTouch(TouchButtonProc touch_button,TouchMoveProc touch_move)
{
	if(touch_button != NULL)
		touch_button_proc = touch_button;
	if(touch_move != NULL)
		touch_move_proc = touch_move;
}

void InitAnalog(AnalogProc analogproc)
{
	if(analogproc!=NULL)
		analog_proc = analogproc;
}
void InitInput(ButtonProc downproc,ButtonProc upproc,int doneflag)
{
	static int inited = 0;
	if(inited == 0){
		if(SDL_NumJoysticks()) {
			joystick = SDL_JoystickOpen(0);
		}
		inited = 1;
	}
	if(downproc != NULL)
		btn_down  = downproc;
	if(upproc != NULL)
		btn_up = upproc;
	game_quit = doneflag;
}

void InputProc()
{
	int ret = 0,x,y,dx,dy,state,tmp;
	int mouse_btn_type = 0;

	while( SDL_PollEvent( &event ) )
    {
		switch(event.type)
		{
		case SDL_QUIT:
			exit(0);
			break;
		case SDL_KEYDOWN:
			btn_down(event.key.keysym.sym);
			ret = SetAnalog(event.key.keysym.sym,1);
			break;
		case SDL_KEYUP:
			btn_up(event.key.keysym.sym);
			ret = SetAnalog(event.key.keysym.sym,0);
			break;
		case SDL_JOYBUTTONDOWN:
			btn_down(event.jbutton.button);
			break;
		case SDL_JOYBUTTONUP:
			btn_up(event.jbutton.button);
			break;
		case SDL_MOUSEMOTION:
			if(touch_move_proc){
				#ifdef IPHONEOS
				SDL_SelectMouse(event.motion.which);        /* select 'mouse' (touch) that moved */
				ret = SDL_GetMouseState(event.motion.which,&x, &y);  /* get its location */
				SDL_GetRelativeMouseState(event.motion.which,&dx, &dy);        /* find how much the mouse moved */
				#endif
				if(need_swapxy){

					tmp = dx;
					dx = dy;
					dy = 320-tmp;
				}
				touch_move_proc(event.motion.which,x,y,dx,dy);
			}
			if(mouse_move_proc){
				x = event.button.x;
				y = event.button.y;
				if(need_swapxy){
					tmp = x;
					x = y;
					y = 320-tmp;
				}
				if((touched&&touch_mode == 1)||(touch_mode == 0))
					mouse_move_proc(x,y);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if(event.button.button == SDL_BUTTON_LEFT){
				mouse_btn_type = MOUSE_LBUTTON_DOWN;
				touched = 1;
			}
			else if(event.button.button == SDL_BUTTON_RIGHT)
				mouse_btn_type = MOUSE_RBUTTON_DOWN;
			else
				mouse_btn_type = MOUSE_MBUTTON_DOWN;
			if(mouse_btn_proc){
				x = event.button.x;
				y = event.button.y;
				if(need_swapxy){
					tmp = x;
					x = y;
					y = 320-tmp;
				}
				mouse_btn_proc(mouse_btn_type,x,y);
			}
			if(touch_button_proc){
				#ifdef IPHONEOS
				SDL_SelectMouse(event.button.which);        /* select 'mouse' (touch) that moved */
				state = SDL_GetMouseState(event.button.which,&x, &y);  /* get its location */
				#endif
				if(need_swapxy){
					tmp = x;
					x = y;
					y = 320-tmp;
				}
				if (state & SDL_BUTTON_LMASK) {
					touch_button_proc(event.button.which,MOUSE_LBUTTON_DOWN,x,y);
				}
				//SDL_GetRelativeMouseState(event.motion.which,&dx, &dy);        /* find how much the mouse moved */
			 }
			break;
		case SDL_MOUSEBUTTONUP:
			if(event.button.button == SDL_BUTTON_LEFT){
				mouse_btn_type = MOUSE_LBUTTON_UP;
				touched = 0;
			}
			else if(event.button.button == SDL_BUTTON_RIGHT)
				mouse_btn_type = MOUSE_RBUTTON_UP;
			else
				mouse_btn_type = MOUSE_MBUTTON_UP;
			if(mouse_btn_proc){
				x = event.button.x;
				y = event.button.y;
				if(need_swapxy){
					tmp = x;
					x = y;
					y = 320-tmp;
				}
				mouse_btn_proc(mouse_btn_type,x,y);
			}

			if(touch_button_proc){
				#ifdef IPHONEOS
				SDL_SelectMouse(event.button.which);        /* select 'mouse' (touch) that moved */
				state = SDL_GetMouseState(event.button.which,&x, &y);  /* get its location */
				#endif
				if(need_swapxy){
					tmp = x;
					x = y;
					y = 320-tmp;
				}
				if (state & SDL_BUTTON_LMASK) {
					touch_button_proc(event.button.which,MOUSE_LBUTTON_UP,x,y);
				}
					//SDL_GetRelativeMouseState(event.motion.which,&dx, &dy);        /* find how much the mouse moved */
			}
			break;
		}

	}
	if(analog_proc!=NULL&&ret){
		analog_proc(GetAnalogX(),GetAnalogY());
	}
}
#else
//for psp

typedef struct {
	uint32 press;
	uint32 held;
	uint32 pspcode;
	uint32 mapcode;
}key_states;

key_states nge_keymap[]={
	{0,0,PSP_CTRL_TRIANGLE,0},//0
	{0,0,PSP_CTRL_CIRCLE,1},//1
	{0,0,PSP_CTRL_CROSS,2},//2
	{0,0,PSP_CTRL_SQUARE,3},//3
	{0,0,PSP_CTRL_LTRIGGER,4},//4
	{0,0,PSP_CTRL_RTRIGGER,5},//5
	{0,0,PSP_CTRL_DOWN,6},//6
	{0,0,PSP_CTRL_LEFT,7},//7
	{0,0,PSP_CTRL_UP,8},//8
	{0,0,PSP_CTRL_RIGHT,9},//9
	{0,0,PSP_CTRL_SELECT,10},//10
	{0,0,PSP_CTRL_START,11},//11
	{0,0,PSP_CTRL_HOME,12},//12
	{0,0,PSP_CTRL_HOLD,13}//13
};
int key_num = 14;//sizeof(nge_keymap)/sizeof(key_states);

SceCtrlData pad;

void InitAnalog(AnalogProc analogproc)
{
	if(analogproc!=NULL)
		analog_proc = analogproc;
}

void InitMouse(MouseButtonProc mouse_btn,MouseMoveProc mouse_move)
{

}

void InitTouch(TouchButtonProc touch_button,TouchMoveProc touch_move)
{

}

void InitInput(ButtonProc downproc,ButtonProc upproc,int doneflag)
{
	static int inited = 0;
	if(inited == 0){
		sceCtrlSetSamplingCycle(0);
		sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
		inited = 1;
	}
	if(downproc != NULL)
		btn_down  = downproc;
	if(upproc != NULL)
		btn_up = upproc;
	game_quit = doneflag;
}
void InputProc()
{
	uint32 Buttons;
	int i;
	static int suspended = 0;

	sceCtrlPeekBufferPositive(&pad, 1);
	Buttons = pad.Buttons;
	if(analog_proc!=NULL){
		analog_proc(pad.Lx,pad.Ly);
	}
	if(Buttons != 0){
		if((PSP_CTRL_HOME&Buttons)&&(!game_quit||game_quit==2)){
			if(!game_quit){
				sceKernelExitGame();
				exit(0);
			}
			else{
				if(!suspended){
					sceKernelSuspendThread(cbid);
					suspended = 1;
				}
			}
		}
		for(i = 0;i<key_num;i++){
			if((nge_keymap[i].pspcode&Buttons)==0){
				if(nge_keymap[i].held == 1){
						btn_up(nge_keymap[i].mapcode);
				}
				nge_keymap[i].press = 0;
				nge_keymap[i].held  = 0;;
			}
			else{
				//btn_up(nge_keymap[i].mapcode);
				if(nge_keymap[i].held==0){
					nge_keymap[i].held = 1;
					nge_keymap[i].press =1;
					btn_down(nge_keymap[i].mapcode);
				}
				else{
					nge_keymap[i].held = 1;
					nge_keymap[i].press =1;
				}
			}
		}
	}
	else{
			for(i = 0;i<key_num;i++){
					if(nge_keymap[i].held == 1){
						btn_up(nge_keymap[i].mapcode);
					}
					nge_keymap[i].press = 0;
					nge_keymap[i].held  = 0;
			}
	}
}
#endif
