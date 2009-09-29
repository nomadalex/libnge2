#ifndef WIN32

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspthreadman.h>
#include <pspaudiocodec.h> 
#include <pspmpeg.h>
#include "nge_define.h"

extern int NGE_main(int argc, char *argv[]);

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
	int cbid;
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

int main(int argc, char *argv[])
{
	pspDebugScreenInit();
	nge_psp_setup_callbacks();
	(void)NGE_main(argc, argv);
	/* Delay 0.5 seconds before returning to the OS. */
	sceKernelDelayThread(500000);
	sceKernelExitGame();
	return 0;
}
#endif