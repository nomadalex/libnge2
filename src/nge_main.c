#include "nge_platform.h"
#include "nge_debug_log.h"
#include "nge_main.h"
#include "nge_app.h"
#include "nge_graphics.h"
#include "nge_input.h" // SDL.h is in it
#include "audio_interface.h"
#include <string.h>

extern int NGE_main(int argc, char *argv[]);

#ifdef NGE_PSP
#include <pspmoduleinfo.h>
#include <pspthreadman.h>
#include <pspdebug.h>
#include <psploadexec.h>

int cbid = 0;

PSP_MODULE_INFO("NGE APP", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(0);
PSP_HEAP_SIZE_KB(18*1024);

int nge_psp_exit_callback(int arg1, int arg2, void *common)
{
	sceKernelDelayThread(500000);
	sceKernelExitGame();
	return 0;
}

int nge_psp_callback_thread(SceSize args, void *argp)
{
	//int cbid;
	cbid = sceKernelCreateCallback("Exit Callback",nge_psp_exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
	return 0;
}

int nge_psp_setup_callbacks(void)
{
	int thid = 0;
	thid = sceKernelCreateThread("update_thread",nge_psp_callback_thread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
		sceKernelStartThread(thid, 0, 0);
	return thid;
}
#endif

#if !defined NGE_ANDROID
#undef main
int main(int argc, char *argv[])
{
	nge_app_t *app;
	int ret = 0;

#ifdef NGE_PSP
#ifndef NDEBUG
	pspDebugScreenInit();
#endif
	nge_psp_setup_callbacks();
#endif
	
	(void)NGE_main(argc, argv);
	app = nge_get_app();
	if (app) {
		app->init();
		while (1) {
			if (app->mainloop() == NGE_APP_QUIT)
				break;
		}
		ret = app->fini();
	}
	
#ifdef NGE_PSP
	/* Delay 0.5 seconds before returning to the OS. */
	sceKernelDelayThread(500000);
	sceKernelExitGame();
	return 0;
#endif
	return ret;
}
#endif

void NGE_SetScreenContext(const char* winname,int screen_width,int screen_height,int screen_bpp,int screen_full)
{
	screen_context_p screen = GetScreenContext();
	if(winname!=NULL)
		strncpy(screen->name,winname,256);
	screen->width = screen_width;
	screen->height = screen_height;
	screen->bpp = screen_bpp;
	screen->fullscreen = screen_full;

	screen->ori_width = screen_width;
	screen->ori_height = screen_height;
	screen->rate_h = screen->rate_w = 1.0f;
}

void NGE_SetNativeResolution(int width,int height)
{
	screen_context_p screen = GetScreenContext();
	screen->ori_width = width;
	screen->ori_height = height;
	screen->rate_w = 1.0f * width/screen->width;
	screen->rate_h = 1.0f * height/screen->height;
}

static int initFlags = 0;

void NGE_Init(int flags)
{
	if(initFlags==0){
#ifndef NGE_IPHONE
		if(flags&INIT_VIDEO)
			InitGrahics();
		if(flags&INIT_AUDIO)
			CoolAudioDefaultInit();
#endif
		initFlags = flags;
	}
}

void NGE_Quit()
{
	if(initFlags){
#ifndef NGE_IPHONE
		FiniInput();
		if(initFlags&INIT_VIDEO)
			FiniGrahics();
		if(initFlags&INIT_AUDIO)
			CoolAudioDefaultFini();
#endif
		initFlags = 0;

#if defined NGE_WIN
		SDL_Quit();
#endif

#ifdef MMGR
		m_dumpMemoryReport();
#endif
	}
}
