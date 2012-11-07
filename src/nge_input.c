#include "nge_common.h"
#include "nge_platform.h"
#include "nge_debug_log.h"
#include "nge_graphics.h"
#if defined NGE_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#endif

#include "nge_input.h"
#include <stdlib.h>

#if defined NGE_PSP
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>

//define in nge.c
extern int psp_exit_callback_id;
static int game_quit = 0;


#elif defined NGE_LINUX
#include <X11/Xlib.h>
// backward declarations
// (in nge_graphics)
extern Display *g_dpy;
extern Window   g_win;
#endif

#ifdef NGE_INPUT_BUTTON_SUPPORT
static void btn_down_default(int keycode) { }
static void btn_up_default(int keycode) { }

static ButtonProc btn_down = btn_down_default;
static ButtonProc btn_up   = btn_up_default;

void InitInput(ButtonProc downproc,ButtonProc upproc,int doneflag)
{
#ifdef NGE_PSP
	static int inited = 0;
	if(inited == 0){
		sceCtrlSetSamplingCycle(0);
		sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
		inited = 1;
	}
#elif defined NGE_LINUX
	long mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | PointerMotionMask | StructureNotifyMask;
	XSelectInput(g_dpy, g_win, mask);
#endif
	if(downproc != NULL)
		btn_down  = downproc;
	if(upproc != NULL)
		btn_up = upproc;
#ifdef NGE_PSP
	game_quit = doneflag;
#endif
}
#endif

static int need_swapxy = 0;

void SetSwapXY(int flag)
{
	if(flag)
		need_swapxy = 1;
}

#ifdef NGE_INPUT_MOUSE_SUPPORT
MouseMoveProc mouse_move_proc = NULL;
MouseButtonProc mouse_btn_proc = NULL;

static touch_mode = 0;
static istouched  = 0;
static screen_context_p screen = NULL;

void EmulateTouchMove(int flag)
{
	if(flag == 1)
		touch_mode  = 1;
	else
		touch_mode  = 0;
}


void InitMouse(MouseButtonProc mouse_btn,MouseMoveProc mouse_move)
{
	if(mouse_move != NULL)
		mouse_move_proc = mouse_move;
	if(mouse_btn != NULL)
		mouse_btn_proc = mouse_btn;
	screen = GetScreenContext();
}
#endif

#ifdef NGE_INPUT_ANALOG_SUPPORT
static AnalogProc analog_proc = NULL;

void InitAnalog(AnalogProc analogproc)
{
	if(analogproc!=NULL)
		analog_proc = analogproc;
}

//Ä£ÄâÒ¡¸Ë
#if defined NGE_WIN || defined NGE_LINUX
#define ANALOG_LEFT  0
#define ANALOG_RIGHT 1
#define ANALOG_UP    2
#define ANALOG_DOWN  3
static char btn_analog[4] = {0};

static uint8_t GetAnalogX()
{
	if (btn_analog[ANALOG_LEFT]) return 0;
	if (btn_analog[ANALOG_RIGHT]) return 0xff;

	return 0x80;
}

static uint8_t GetAnalogY()
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
#if defined NGE_WIN
	case VK_UP:
#elif defined(NGE_LINUX)
	case XK_Up:
#endif
		btn_analog[ANALOG_UP] = flag;
		ret = 1;
		break;
#if defined(NGE_WIN)
	case VK_DOWN:
#elif defined(NGE_LINUX)
	case XK_Down:
#endif
		btn_analog[ANALOG_DOWN] = flag;
		ret = 1;
		break;
#if defined(NGE_WIN)
	case VK_LEFT:
#elif defined(NGE_LINUX)
	case XK_Left:
#endif
		btn_analog[ANALOG_LEFT] = flag;
		ret = 1;
		break;
#if defined(NGE_WIN)
	case VK_RIGHT:
#elif defined(NGE_LINUX)
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
#endif

#if defined(NGE_LINUX)
void FiniGraphics();

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
	if (event->xbutton.button == Button1)						\
		button = MOUSE_LBUTTON_##type;							\
	else if (event->xbutton.button == Button2)					\
		button = MOUSE_MBUTTON_##type;							\
	else if (event->xbutton.button == Button3)					\
		button = MOUSE_RBUTTON_##type;							\
	else														\
		return;													\
	mouse_btn_proc(button, event->xbutton.x, event->xbutton.y)

_DEF_INPUT_PROC(mouse_down)
{
	BUTTON_PROC(DOWN);
}

_DEF_INPUT_PROC(mouse_up)
{
	BUTTON_PROC(UP);
}
#elif defined(NGE_PSP)
typedef struct {
	uint32_t press;
	uint32_t held;
	uint32_t pspcode;
	uint32_t mapcode;
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

#if defined NGE_WIN || defined NGE_LINUX
static int ana_ret = 0;
#endif

#ifdef NGE_WIN

#define MOUSE_BUTTON_HANDLE(type)					\
	int mouse_btn_type = 0;							\
	int xPos = GET_X_LPARAM(lParam);				\
	int yPos = GET_Y_LPARAM(lParam);				\
													\
	switch (wParam) {								\
	case VK_LBUTTON:								\
		mouse_btn_type = MOUSE_LBUTTON_##type;		\
		break;										\
													\
	case VK_RBUTTON:								\
		mouse_btn_type = MOUSE_RBUTTON_##type;		\
		break;										\
													\
	case VK_MBUTTON:								\
		mouse_btn_type = MOUSE_MBUTTON_##type;		\
		break;										\
	}												\
	if (mouse_btn_proc != NULL)						\
		mouse_btn_proc(mouse_btn_type, 1.0f*xPos*screen->ori_width/screen->width, 1.0f*yPos*screen->ori_height/screen->height);	\
	return 0

int nge_win_mouse_btn_down_handle(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	MOUSE_BUTTON_HANDLE(DOWN);
}

int nge_win_mouse_btn_up_handle(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	MOUSE_BUTTON_HANDLE(UP);
}

int nge_win_mouse_move_handle(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int xPos = GET_X_LPARAM(lParam);
	int yPos = GET_Y_LPARAM(lParam);
	if ((mouse_move_proc != NULL) && ((istouched&&touch_mode == 1)||(touch_mode == 0)))
		mouse_move_proc(1.0f*xPos*screen->ori_width/screen->width, 1.0f*yPos*screen->ori_height/screen->height);
	return 0;
}

int nge_win_key_down_handle(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	btn_down(wParam);
	ana_ret = SetAnalog(wParam,1);
	return 0;
}

int nge_win_key_up_handle(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	btn_up(wParam);
	ana_ret = SetAnalog(wParam,0);
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		return 0;

	case WM_CLOSE:
		PostQuitMessage( 0 );
		return 0;

	case WM_DESTROY:
		PostQuitMessage( 0 );
		return 0;

	case WM_KEYDOWN:
		return nge_win_key_down_handle(hWnd, wParam, lParam);

	case WM_KEYUP:
		return nge_win_key_up_handle(hWnd, wParam, lParam);

	case WM_LBUTTONDOWN:
		 istouched = 1;
		 return nge_win_mouse_btn_down_handle(hWnd, VK_LBUTTON, lParam);
	case WM_RBUTTONDOWN:
		 return nge_win_mouse_btn_down_handle(hWnd, VK_RBUTTON, lParam);
	case WM_MBUTTONDOWN:
		 return nge_win_mouse_btn_down_handle(hWnd, VK_MBUTTON, lParam);

	case WM_LBUTTONUP:
		 istouched = 0;
		 return nge_win_mouse_btn_up_handle(hWnd, VK_LBUTTON, lParam);
	case WM_RBUTTONUP:
		 return nge_win_mouse_btn_up_handle(hWnd, VK_RBUTTON, lParam);
	case WM_MBUTTONUP:
		 return nge_win_mouse_btn_up_handle(hWnd, VK_MBUTTON, lParam);

	case WM_MOUSEMOVE:
		return nge_win_mouse_move_handle(hWnd, wParam, lParam);

	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}
}
#endif

void InputProc()
{
#if defined(NGE_LINUX)
	static long mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | PointerMotionMask | StructureNotifyMask;
	static XEvent event;
#elif defined NGE_WIN
	MSG msg;
#endif

#if defined NGE_WIN || defined NGE_LINUX
	ana_ret = 0;
#endif

#if defined(NGE_LINUX)
	while (XCheckWindowEvent(g_dpy, g_win,
							 mask, &event)) {
		if (event.xany.window == g_win) {
			switch (event.type) {
			case DestroyNotify:
				FiniGraphics();
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
#elif defined(NGE_WIN)
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if ( msg.message == WM_QUIT )
		{
			exit(0);
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
#endif

#if defined NGE_WIN || defined NGE_LINUX
	if(analog_proc!=NULL&&ana_ret){
		analog_proc(GetAnalogX(),GetAnalogY());
	}
#endif

#if defined(NGE_PSP)
	static SceCtrlData pad;
	uint32_t Buttons;
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
					sceKernelSuspendThread(psp_exit_callback_id);
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
