/**
 * @file  nge_utilskit.c
 * @author newcreat <newcreat@libnge.org>
 * @date 2012/08/31 14:56:05
 *
 *  Copyright  2012  newcreat <newcreat@libnge.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <jni.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include "libnge2.h"
#include "nge_utilskit.h"
#include "jni_android.h"
#define  LOG_TAG    "nge_utilskit"
static inline void LOGI(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	__android_log_vprint(ANDROID_LOG_INFO,LOG_TAG, fmt, args);
	va_end(args);
}
static inline void LOGE(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	__android_log_vprint(ANDROID_LOG_ERROR,LOG_TAG, fmt, args);
	va_end(args);
}

#define WARN_UN_IMP() LOGI("%s not implementation!\n", __FUNCTION__)

static JavaVM* javaVM = NULL;
static JNIEnv* env = NULL;
static jobject obj = NULL;
static screen_context_p screen = NULL;
static PayFeedBackCall pPayFeedBackCall = NULL;
static void* pUserData = NULL;

inline static JNIEnv* GetEnv()
{
	if (javaVM) (*javaVM)->GetEnv(javaVM, (void**)&env, JNI_VERSION_1_2);
	return env;
}

static jclass cLibUtilsKit = NULL;

#define CA_METHOD(method) mLibUtilsKit_##method
#define MAKE_CA_METHOD(name) static jmethodID CA_METHOD(name)
MAKE_CA_METHOD(LibUtilsKit);
//ad
MAKE_CA_METHOD(CreateAdHandle);
MAKE_CA_METHOD(SetAdPos);
MAKE_CA_METHOD(ShowAd);
MAKE_CA_METHOD(DestroyAdHandle);
//payment
MAKE_CA_METHOD(CreatePayHandle);
MAKE_CA_METHOD(Pay);
//share
MAKE_CA_METHOD(Share);
//more game
MAKE_CA_METHOD(MoreGame);
#undef MAKE_CA_METHOD

/**
 * Cache LibUtilsKit class and it's method id's
 * And do this only once!
 */
inline static void load_methods()
{
	javaVM  = GetJaveVM();
	GetEnv();
	cLibUtilsKit = (*env)->FindClass(env, "org/libnge/nge2/UtilsKit");
	if (!cLibUtilsKit)
	{
		LOGE("org.libnge.nge2.UtilsKit class is not found.");
		return;
	}
	cLibUtilsKit = (*env)->NewGlobalRef(env, cLibUtilsKit);
	CA_METHOD(LibUtilsKit) = (*env)->GetMethodID(env, cLibUtilsKit, "<init>", "()V");
	obj = (*env)->NewObject(env, cLibUtilsKit, CA_METHOD(LibUtilsKit));
	obj = (*env)->NewGlobalRef(env, obj);
#define GET_CA_METHOD(method, signture) CA_METHOD(method) = (*env)->GetMethodID( \
		env, cLibUtilsKit, #method, signture)
	GET_CA_METHOD(CreateAdHandle, "(Ljava/lang/String;Ljava/lang/String;)I");
	GET_CA_METHOD(SetAdPos, "(III)V");
	GET_CA_METHOD(ShowAd, "(II)V");
	GET_CA_METHOD(DestroyAdHandle, "(I)V");
	//payment
	GET_CA_METHOD(CreatePayHandle, "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
	GET_CA_METHOD(Pay, "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
	//share
	GET_CA_METHOD(Share, "(ILjava/lang/String;Ljava/lang/String;)I");
	//moregame
	GET_CA_METHOD(MoreGame, "(I)V");
#undef GET_CA_METHOD
}

int CreateAdHandle(const char* adKey,const char* adType)
{
	jstring jadKey,jadType;
	int     handle;
	GetEnv();
	jadKey  = (*env)->NewStringUTF(env, adKey);
	jadType = (*env)->NewStringUTF(env, adType);
	handle = (*env)->CallIntMethod(env, obj, CA_METHOD(CreateAdHandle), jadKey,jadType);	
	(*env)->DeleteLocalRef(env, jadKey);
	(*env)->DeleteLocalRef(env, jadType);
	return handle;
}

void SetAdPos(int adHandle,int x,int y)
{
	GetEnv();
	(*env)->CallVoidMethod(env, obj, CA_METHOD(SetAdPos),adHandle,x,y);
}
void ShowAd(int adHandle,int bShow)
{
	GetEnv();
	(*env)->CallVoidMethod(env, obj, CA_METHOD(ShowAd),adHandle,bShow);
}

void DestroyAdHandle(int adHandle)
{
	GetEnv();
	(*env)->CallVoidMethod(env, obj, CA_METHOD(DestroyAdHandle),adHandle);
}

//for payment
int CreatePayHandle(const char* payType,const char* partner,const char* seller,const char* privateRSA,const char* publicRSA)
{
	jstring jpayType,jpartner,jseller,jprivateRSA,jpublicRSA;
	int     handle;
	GetEnv();
	jpayType  = (*env)->NewStringUTF(env, payType);
	jpartner  = (*env)->NewStringUTF(env, partner);
	jseller   = (*env)->NewStringUTF(env, seller);
	jprivateRSA  = (*env)->NewStringUTF(env, privateRSA);
	jpublicRSA   = (*env)->NewStringUTF(env, publicRSA);
	
	handle = (*env)->CallIntMethod(env, obj, CA_METHOD(CreatePayHandle), jpayType,jpartner,jseller,jprivateRSA,jpublicRSA);	
	(*env)->DeleteLocalRef(env, jpayType);
	(*env)->DeleteLocalRef(env, jpartner);
	(*env)->DeleteLocalRef(env, jseller);
	(*env)->DeleteLocalRef(env, jprivateRSA);
	(*env)->DeleteLocalRef(env, jpublicRSA);
	return handle;
}

int Pay(int payHandle,const char* title,const char* body,const char* total,PayFeedBackCall pCall,void* userData)
{
	jstring jtitle,jbody,jtotal;
	int     handle;
	GetEnv();
	jtitle  = (*env)->NewStringUTF(env, title);
	jbody   = (*env)->NewStringUTF(env, body);
	jtotal  = (*env)->NewStringUTF(env, total);
	
	handle = (*env)->CallIntMethod(env, obj, CA_METHOD(Pay),payHandle, jtitle,jbody,jtotal);	
	(*env)->DeleteLocalRef(env, jtitle);
	(*env)->DeleteLocalRef(env, jbody);
	(*env)->DeleteLocalRef(env, jtotal);
	pPayFeedBackCall = pCall;
	pUserData        = userData;
	return handle;
}

// for share
int Share(int shareHandle,const char* str,const char* imgName)
{
	jstring jstr,jimgName;
	int     handle;
	GetEnv();
	jstr  = (*env)->NewStringUTF(env, str);
	jimgName   = (*env)->NewStringUTF(env, imgName);

	handle = (*env)->CallIntMethod(env, obj, CA_METHOD(Share),shareHandle, jstr,jimgName);	
	(*env)->DeleteLocalRef(env, jstr);
	(*env)->DeleteLocalRef(env, jimgName);
	return handle;
}

void Java_org_libnge_nge2_UtilsKit_nativePayFeedBack(JNIEnv* env,jobject thiz,jint error,jstring errorStr)
{
	char* error_str = NULL;/*jstringTostring(env,errorStr);*/
	int   error_id  = error;
	if(pPayFeedBackCall!=NULL)
		pPayFeedBackCall(error_id,error_str,pUserData);
	if(error_str)
		free(error_str);
}


void MoreGame(int type)
{
	GetEnv();
	(*env)->CallVoidMethod(env, obj, CA_METHOD(MoreGame),type);	
}

void UtilsKitDefaultInit()
{
	load_methods();
	screen = GetScreenContext();
}

void UtilsKitDefaultFini()
{
	GetEnv();
	(*env)->DeleteGlobalRef(env, cLibUtilsKit);
}

