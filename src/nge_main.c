#include "nge_main.h"
#include "nge_graphics.h"
#include "nge_input.h" // SDL.h is in it

#ifdef _PSP
#include <pspmoduleinfo.h>
#include <pspthreadman.h>
#include <pspdebug.h>
#include <psploadexec.h>

extern int NGE_main(int argc, char *argv[]);
int cbid = 0;

PSP_MODULE_INFO("NGE APP", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(0);
PSP_HEAP_SIZE_KB(18*1024);
//PSP_MODULE_INFO("Blit Sample", 0, 1, 1);
//PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

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

#undef main
int main(int argc, char *argv[])
{
#ifndef NDEBUG
	pspDebugScreenInit();
#endif
	nge_psp_setup_callbacks();
	(void)NGE_main(argc, argv);
	/* Delay 0.5 seconds before returning to the OS. */
	sceKernelDelayThread(500000);
	sceKernelExitGame();
	return 0;
}
#endif

static int initFlags = 0;

void NGE_SetScreenContext(const char* winname,int screen_width,int screen_height,int screen_bpp,int screen_full)
{
	screen_context_p screen = GetScreenContext();
	if(winname!=NULL)
		strncpy(screen->name,winname,256);
	screen->width = screen_width;
	screen->height = screen_height;
	screen->bpp = screen_bpp;
	screen->fullscreen = screen_full;
}

void NGE_Init(int flags)
{
	if(initFlags==0){
#ifndef IPHONEOS
		if(flags&INIT_VIDEO)
			InitGrahics();
#endif
		initFlags = flags;
	}
}

void NGE_Quit()
{
	if(initFlags){
#ifndef IPHONEOS
		FiniInput();
		if(initFlags&INIT_VIDEO)
			FiniGrahics();
#endif

#if defined WIN32 || defined __linux__// linux and win32
		SDL_Quit();
#endif

#ifdef MMGR
		m_dumpMemoryReport();
#endif
	}
}
