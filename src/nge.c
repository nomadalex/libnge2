/**
 * @file  nge.c
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/02/12 06:42:41
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */
#include "nge_debug_log.h"
#include "nge.h"
#include "nge_graphics.h"
#include "nge_input.h"
#include "audio_interface.h"
#include <string.h>

#ifdef NGE_PSP
int psp_exit_callback_id = 0;
void NGE_RegisterPSPExitCallback(int cb_id) {
	psp_exit_callback_id = cb_id;
}
#endif

NotifyCallback _notifyCallback = NULL;
void* _notifyCookie = NULL;

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

char *NGE_OP_Path = NULL;
void NGE_SetOPMoviePath(const char* path) {
	NGE_OP_Path = path;
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

#ifdef MMGR
		m_dumpMemoryReport();
#endif
	}

	_notifyCallback = NULL;
	_notifyCookie = NULL;
}

void NGE_RegisterNotifyCallback(NotifyCallback cb, void* pCookie)
{
	_notifyCallback = cb;
	_notifyCookie = pCookie;
}

#ifdef NGE_WIN
/* just for export dll symbol */
void _______fake_CreateMp3Player() {
	CreateMp3Player();
}
void _______fake_CreateWavPlayer() {
	CreateWavPlayer();
}
void _______fake_CreateOggPlayer() {
	CreateOggPlayer();
}
#endif
