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

extern "C"
{
	static nge_app_t *s_app = NULL;

	static unsigned char sPaused = 0;
	static unsigned char sResume = 0;
	static char sPackName[256]={0};
	static screen_context_p screen;

	void Java_org_libnge_nge2_NGE2_nativeSetContext(JNIEnv* env,
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

	void Java_org_libnge_nge2_NGE2_nativeSetWorkPath(JNIEnv* env,
			jobject thiz,jstring packname)
	{
			char fullname[512]={0};
			char* name = jstringTostring(env, packname);
			sprintf(fullname,"/data/data/%s/resource/",name);
			if(name)
				free(name);
			chdir(fullname);
			nge_print("nge2 set workpath:%s.\n",fullname);
	}

	extern void nge_graphics_reset(void);
	void Java_org_libnge_nge2_NGE2_nativeResetContext(JNIEnv* env,
													  jobject thiz )
	{
		nge_graphics_reset();
	}

	extern int NGE_main(int argc, char *argv[]);

	void Java_org_libnge_nge2_NGE2_nativeInitialize(JNIEnv* env,
			jobject thiz )
	{
		NGE_main(0, 0);
		s_app = nge_get_app();
		s_app->init();
		nge_print("nge2 init normaly.\n");
	}

	int Java_org_libnge_nge2_NGE2_nativeUpdate(JNIEnv* env,
			jobject thiz )
	{
		if(sPaused)
			return NGE_APP_NORMAL;
		if(sResume) {
			sResume = 0;
		}

		return s_app->mainloop();
	}

	void  Java_org_libnge_nge2_NGE2_nativeFinalize(JNIEnv* env,
			jobject thiz )
	{
		s_app->fini();
		nge_print("nge2 finished normaly.\n");
	}

	void Java_org_libnge_nge2_NGE2_nativePause(JNIEnv* env,
			jobject thiz )
	{
		// do nothing for now
		nge_print("Paused.\n");
		sPaused = 1;
	}

	void Java_org_libnge_nge2_NGE2_nativeResume(JNIEnv* env,
			jobject thiz )
	{
		nge_print("Resume.\n");
		sPaused = 0;
		sResume = 1;
	}

	// decl from nge_input.c
	extern MouseMoveProc mouse_move_proc;
	extern MouseButtonProc mouse_btn_proc;

	void Java_org_libnge_nge2_NGE2_nativeTouch(JNIEnv* env,
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

}
