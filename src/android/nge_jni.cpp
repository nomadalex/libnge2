/*
 * Copyright (C) 2011 TOPOC
 */
#include <string.h>
#include <jni.h>
#include <stdio.h>
#include <unistd.h>
#include <android/log.h>
#include <GLES/gl.h>

#include "nge_app.h"
#include "libnge2.h"

#define JNIAPI extern "C"
// #define DEBUG_HERE() nge_print("%s %s %d\n",__FUNCTION__,  __FILE__, __LINE__)

char *main_argv[2] = { 0, 0};

static nge_app_t *s_app = NULL;

static unsigned char sPaused = 0;
static unsigned char sResume = 0;

static screen_context_p screen;

JNIAPI void Java_org_libnge_nge2_NGE2_nativeSetContext(JNIEnv* env,
													   jobject thiz,jint w,jint h)
{
	NGE_SetNativeResolution(w,h);
	screen = GetScreenContext();
}

//jstring to char*
char* jstringTostring(JNIEnv* env, jstring jstr)
{
	char* rtn = NULL;
	jclass clsstring = env->FindClass("java/lang/String");
	jstring strencode = env->NewStringUTF("utf-8");
	jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray barr= (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
	jsize alen = env->GetArrayLength(barr);
	jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
	if (alen > 0)
	{
		rtn = (char*)malloc(alen + 1);
		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}
	env->ReleaseByteArrayElements(barr, ba, 0);
	return rtn;
}

JNIAPI void Java_org_libnge_nge2_NGE2_nativeSetPackname(JNIEnv* env,
														jobject thiz,jstring packname)
{
	main_argv[0] = jstringTostring(env, packname);
}

JNIAPI void Java_org_libnge_nge2_NGE2_nativeCreate(JNIEnv* env,
												   jobject thiz)
{
	main(1, main_argv);
	s_app = nge_get_app();
	if (!s_app)
		nge_error("no app!");
}

extern "C" void nge_graphics_reset(void);
JNIAPI void Java_org_libnge_nge2_NGE2_nativeResetContext(JNIEnv* env,
														 jobject thiz )
{
	if(sResume) {
		nge_print("nge2 reset graphics.\n");
		nge_graphics_reset();
		sResume = 0;
	}
}

extern "C" int main(int argc, char *argv[]);

JNIAPI void Java_org_libnge_nge2_NGE2_nativeInitialize(JNIEnv* env,
													   jobject thiz )
{
	s_app->init();
	nge_print("nge2 init normaly.\n");
}

JNIAPI int Java_org_libnge_nge2_NGE2_nativeUpdate(JNIEnv* env,
												  jobject thiz )
{
	if(sPaused)
		return NGE_APP_NORMAL;

	return s_app->mainloop();
}

JNIAPI void  Java_org_libnge_nge2_NGE2_nativeFinalize(JNIEnv* env,
													  jobject thiz )
{
	s_app->fini();
	nge_print("nge2 finished normaly.\n");
}

JNIAPI void Java_org_libnge_nge2_NGE2_nativePause(JNIEnv* env,
												  jobject thiz )
{
	nge_print("Paused.\n");
	sPaused = 1;
}

JNIAPI void Java_org_libnge_nge2_NGE2_nativeStop(JNIEnv* env,
												  jobject thiz )
{
	nge_print("Stoped.\n");
}

JNIAPI void Java_org_libnge_nge2_NGE2_nativeResume(JNIEnv* env,
												   jobject thiz )
{
	nge_print("Resume.\n");
	if (sPaused) {
		sPaused = 0;
		sResume = 1;
	}
}

// decl from nge_input.c
extern MouseMoveProc mouse_move_proc;
extern MouseButtonProc mouse_btn_proc;

JNIAPI void Java_org_libnge_nge2_NGE2_nativeTouch(JNIEnv* env,
												  jobject thiz , jint action, jint x, jint y)
{
	switch (action)
	{
	case 0: // ACTION_DOWN
		if (mouse_btn_proc)
			mouse_btn_proc(MOUSE_LBUTTON_DOWN,int(x*screen->rate_w),int(y*screen->rate_h));
		break;

	case 1: // ACTION_UP:
		if (mouse_btn_proc)
			mouse_btn_proc(MOUSE_LBUTTON_UP,int(x*screen->rate_w),int(y*screen->rate_h));
		break;
	case 2: // ACTION_MOVE:
		if (mouse_move_proc)
			mouse_move_proc(int(x*screen->rate_w),int(y*screen->rate_h));
		break;
	default:
		break;
	}
}
