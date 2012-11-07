/**
 * @file  nge_android_jni.c
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/10/21 14:43:03
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */
#include <stdlib.h>
#include <jni.h>
#include "nge_android_jni.h"

JavaVM* javaVM = NULL;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	(void)reserved;
	javaVM = vm;
	return JNI_VERSION_1_2;
}

JNIEnv* nge_GetEnv()
{
	JNIEnv* env = NULL;
	if (javaVM) (*javaVM)->GetEnv(javaVM, (void**)&env, JNI_VERSION_1_2);
	return env;
}
