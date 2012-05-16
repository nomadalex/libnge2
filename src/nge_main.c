#include "nge_platform.h"
#include "nge_app.h"
#include "nge.h"

#include <stdio.h>

#ifdef NGE_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

extern int NGE_main(int argc, char *argv[]);

#ifdef NGE_PSP
#include <pspmoduleinfo.h>
#include <pspthreadman.h>
#include <pspdebug.h>
#include <psploadexec.h>

int cbid = 0;

PSP_MODULE_INFO("NGE APP", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);
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
	NGE_RegisterPSPExitCallback(cbid);
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

#ifdef NGE_WIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int iCmdShow)
#else
int main(int argc, char *argv[])
#endif
{
	nge_app_t *app;
	int ret = 0;

#ifdef NGE_WIN
	static int argc = 0;
	static char* argv[] = { NULL };
#ifdef _DEBUG_STDOUT
    AllocConsole();
    freopen("CONOUT$","w+t",stdout);
    freopen("CONIN$","r+t",stdin);
#endif
#endif

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
