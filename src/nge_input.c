#include "nge_input.h"
#include "nge_common.h"
#include <stdlib.h>
#if defined(_PSP)
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>

//define in nge_main.c
extern int cbid;

#elif defined(__linux__)
#include <X11/Xlib.h>

#endif

static void btn_down_default(int keycode) { }
static void btn_up_default(int keycode) { }

static ButtonProc btn_down = btn_down_default;
static ButtonProc btn_up   = btn_up_default;

static int game_quit = 0;
static int need_swapxy = 0;

void SetSwapXY(int flag)
{
	if(flag)
		need_swapxy = 1;
}

#if defined(WIN32) || defined(__linux__)
static int touched = 0;
static int touch_mode = 0;

void EmulateTouchMove(int flag)
{
	if(flag == 1)
		touch_mode  = 1;
	else
		touch_mode  = 0;
}

static MouseMoveProc mouse_move_proc = NULL;
static MouseButtonProc mouse_btn_proc = NULL;

void InitMouse(MouseButtonProc mouse_btn,MouseMoveProc mouse_move)
{
	if(mouse_move != NULL)
		mouse_move_proc = mouse_move;
	if(mouse_btn != NULL)
		mouse_btn_proc = mouse_btn;
}
#endif

#if defined(IPHONEOS) || defined(WIN32) || defined(__linux__)
static TouchMoveProc touch_move_proc = NULL;
static TouchButtonProc touch_button_proc = NULL;

void InitTouch(TouchButtonProc touch_button,TouchMoveProc touch_move)
{
	if(touch_button != NULL)
		touch_button_proc = touch_button;
	if(touch_move != NULL)
		touch_move_proc = touch_move;
}
#endif

#if defined(_PSP) || defined(__linux__) || defined(WIN32)
static AnalogProc analog_proc = NULL;

void InitAnalog(AnalogProc analogproc)
{
	if(analogproc!=NULL)
		analog_proc = analogproc;
}
#endif

void InitInput(ButtonProc downproc,ButtonProc upproc,int doneflag)
{
#ifdef _PSP
	static int inited = 0;
	if(inited == 0){
		sceCtrlSetSamplingCycle(0);
		sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
		inited = 1;
	}
#endif
	if(downproc != NULL)
		btn_down  = downproc;
	if(upproc != NULL)
		btn_up = upproc;
	game_quit = doneflag;
}

#if defined(WIN32) || defined (__linux__)
//Ä£ÄâÒ¡¸Ë
#define ANALOG_LEFT  0
#define ANALOG_RIGHT 1
#define ANALOG_UP    2
#define ANALOG_DOWN  3
static char btn_analog[4] = {0};

uint8 GetAnalogX()
{
	if (btn_analog[ANALOG_LEFT]) return 0;
	if (btn_analog[ANALOG_RIGHT]) return 0xff;

	return 0x80;
}


uint8 GetAnalogY()
{
	if (btn_analog[ANALOG_UP]) return 0;
	if (btn_analog[ANALOG_DOWN]) return 0xff;

	return 0x80;
}

static int SetAnalog(int key,char flag)
{
	int ret = 0;
	switch(key)
	{
#if defined(WIN32)
		case SDLK_UP:
#elif defined(__linux__)
		case XK_Up:
#endif
			btn_analog[ANALOG_UP] = flag;
			ret = 1;
			break;
#if defined(WIN32)
		case SDLK_DOWN:
#elif defined(__linux__)
		case XK_Down:
#endif
			btn_analog[ANALOG_DOWN] = flag;
			ret = 1;
			break;
#if defined(WIN32)
		case SDLK_LEFT:
#elif defined(__linux__)
		case XK_Left:
#endif
			btn_analog[ANALOG_LEFT] = flag;
			ret = 1;
			break;
#if defined(WIN32)
		case SDLK_RIGHT:
#elif defined(__linux__)
		case XK_Right:
#endif
			btn_analog[ANALOG_RIGHT] = flag;
			ret = 1;
			break;
		default:
			break;
	}
	return ret;
}
#endif

#if defined(__linux__)
// backward declarations
// (in nge_graphics)
extern Display *g_dpy;
extern Window   g_win;
void FiniGrahics();

#define _DEF_INPUT_PROC(n) inline void _##n (XEvent *event)

// notice : event is XEvent
#define _INPUT_PROC(n) _##n (&event)

_DEF_INPUT_PROC(btn_down)
{
	unsigned char keysym = XLookupKeysym(&event->xkey, 0);
	btn_down(keysym);
}

_DEF_INPUT_PROC(btn_up)
{
	unsigned char keysym = XLookupKeysym(&event->xkey, 0);
	btn_up(keysym);
}

_DEF_INPUT_PROC(mouse_move)
{
	mouse_move_proc(event->xbutton.x, event->xbutton.y);
}

#define BUTTON_PROC(type)										\
	int button;													\
	if (event->xbutton.button == Button1)							\
		button = MOUSE_LBUTTON_##type;							\
	else if (event->xbutton.button == Button2)					\
		button = MOUSE_MBUTTON_##type;							\
	else if (event->xbutton.button == Button3)					\
		button = MOUSE_RBUTTON_##type;							\
	mouse_btn_proc(button, event->xbutton.x, event->xbutton.y)

static int touch_x, touch_y;

_DEF_INPUT_PROC(mouse_down)
{
	BUTTON_PROC(DOWN);
	if (touch_mode) {
		if (button == MOUSE_LBUTTON_DOWN) {
			touched = TRUE;
			touch_x = event->xbutton.x;
			touch_y = event->xbutton.y;
		}
		if (button == MOUSE_RBUTTON_DOWN) {
			if (touch_button_proc)
				touch_button_proc(button, button, event->xbutton.x, event->xbutton.y);
		}
	}
}

_DEF_INPUT_PROC(mouse_up)
{
	BUTTON_PROC(UP);
	if (touch_mode) {
		if (button == MOUSE_LBUTTON_UP) {
			if (touched) {
				touched = FALSE;
				if (touch_move_proc)
					touch_move_proc(button, touch_x, touch_y, event->xbutton.x, event->xbutton.y);
			}
		}
		if (button == MOUSE_RBUTTON_DOWN) {
			if (touch_button_proc)
				touch_button_proc(button, button, event->xbutton.x, event->xbutton.y);
		}
	}
}
#elif defined(_PSP)
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
int key_num = 14;
#endif

void InputProc()
{
#if defined(__linux__) || defined(WIN32)
	int ana_ret = 0;
#if defined(__linux__)
	static long mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | PointerMotionMask | StructureNotifyMask;
	static XEvent event;
	while (XCheckWindowEvent(g_dpy, g_win,
							 mask, &event)) {
		if (event.xany.window == g_win) {
			switch (event.type) {
			case DestroyNotify:
				FiniGrahics();
				exit(0);
				break;
			case MotionNotify:
				if (mouse_move_proc)
					_INPUT_PROC(mouse_move);
				break;
			case ButtonPress:
				if (mouse_btn_proc)
					_INPUT_PROC(mouse_down);
				break;
			case KeyPress:
				_INPUT_PROC(btn_down);
				ana_ret = SetAnalog(XLookupKeysym(&event.xkey, 0), TRUE);
				break;
			case KeyRelease:
				_INPUT_PROC(btn_up);
				ana_ret = SetAnalog(XLookupKeysym(&event.xkey, 0), FALSE);
				break;
			}
		}
	}
#elif defined(WIN32)
	int x,y,dx,dy,state,tmp;
	int mouse_btn_type = 0;
	static SDL_Event event;

	while( SDL_PollEvent( &event ) )
	{
		switch(event.type)
		{
		case SDL_QUIT:
			exit(0);
			break;
		case SDL_KEYDOWN:
			btn_down(event.key.keysym.sym);
			ana_ret = SetAnalog(event.key.keysym.sym,1);
			break;
		case SDL_KEYUP:
			btn_up(event.key.keysym.sym);
			ana_ret = SetAnalog(event.key.keysym.sym,0);
			break;
		case SDL_MOUSEMOTION:
			dx = event.button.x;
			dy = event.button.y;
			if(mouse_move_proc){
				mouse_move_proc(dx,dy);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			x = event.button.x;
			y = event.button.y;

			if(event.button.button == SDL_BUTTON_LEFT){
				mouse_btn_type = MOUSE_LBUTTON_DOWN;
				touched = 1;
			}
			else if(event.button.button == SDL_BUTTON_RIGHT)
				mouse_btn_type = MOUSE_RBUTTON_DOWN;
			else
				mouse_btn_type = MOUSE_MBUTTON_DOWN;

			if(mouse_btn_proc){
				mouse_btn_proc(mouse_btn_type,x,y);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			x = event.button.x;
			y = event.button.y;

			if(event.button.button == SDL_BUTTON_LEFT){
				mouse_btn_type = MOUSE_LBUTTON_UP;
				touched = 0;
			}
			else if(event.button.button == SDL_BUTTON_RIGHT)
				mouse_btn_type = MOUSE_RBUTTON_UP;
			else
				mouse_btn_type = MOUSE_MBUTTON_UP;
			if(mouse_btn_proc){
				mouse_btn_proc(mouse_btn_type,x,y);
			}
			break;
		}
	}
#endif
	if(analog_proc!=NULL&&ana_ret){
		analog_proc(GetAnalogX(),GetAnalogY());
	}
#elif defined(_PSP)
	static SceCtrlData pad;
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
#endif
}
