/**
 * @file  audio_android.c
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2011/06/12 14:56:05
 *
 *  Copyright  2011  Kun Wang <ifreedom.cn@gmail.com>
 *
 */
#include <jni.h>
#include "nge_android_jni.h"
#include "audio_android.h"

#include "android_mediaplayer.h"
#include "android_wav.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static JNIEnv* env;

#define GetEnv() env = nge_GetEnv()

void CoolAudioDefaultInit()
{
	load_wav();
	load_CA();
}

void CoolAudioDefaultFini()
{
	release_wav();
	release_CA();
}

jobject createFD(JNIEnv* env, int fd)
{
	jobject fdsc;
	jclass cls;
	jmethodID cons_mid;
	jfieldID field;

	GetEnv();

	cls = (*env)->FindClass(env, "java/io/FileDescriptor");
	cons_mid = (*env)->GetMethodID(env, cls, "<init>", "()V");
	fdsc = (*env)->NewObject(env, cls, cons_mid);
	field = (*env)->GetFieldID(env, cls, "fd", "I");
	(*env)->SetIntField(env, fdsc, field, fd);
	return fdsc;
}

audio_play_p CreateWavPlayer()
{
	LOGI("Create wav player\n");
	return (audio_play_p) android_wav_create();
}

audio_play_p CreateMp3Player()
{
	return (audio_play_p) android_mediaplayer_create();
}

audio_play_p CreateOggPlayer()
{
	return (audio_play_p) android_mediaplayer_create();
}
