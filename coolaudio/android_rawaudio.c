/**
 * @file  android_rawaudio.c
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/10/24 15:17:06
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */
#include <stdint.h>
#include <jni.h>
#include "nge_android_jni.h"
#include "audio_android.h"
#include "android_rawaudio.h"

void Java_org_libnge_nge2_RawAudio_PlaybackListener_nativeNotify(JNIEnv* env, jobject thiz, jint cb, jint cookie) {
	((ngeRA_notify_fn)cb)((void*)cookie);
}

static JNIEnv* env;

#define GetEnv() env = nge_GetEnv()

static jclass cRawAudio = NULL;

#define RA_METHOD(method) mRawAudio_##method
#define DECL_RA_METHOD(method) static jmethodID RA_METHOD(method)

DECL_RA_METHOD(RawAudio);

DECL_RA_METHOD(getMinBufferSize);

DECL_RA_METHOD(init);
DECL_RA_METHOD(release);

DECL_RA_METHOD(setListener);
DECL_RA_METHOD(clearListener);

DECL_RA_METHOD(play);
DECL_RA_METHOD(stop);
DECL_RA_METHOD(pause);

DECL_RA_METHOD(getPlayState);
DECL_RA_METHOD(reloadStaticData);
DECL_RA_METHOD(setLoopPoints);

DECL_RA_METHOD(setPosition);
DECL_RA_METHOD(getPosition);

DECL_RA_METHOD(write);

DECL_RA_METHOD(setVolume);
DECL_RA_METHOD(getVolume);

DECL_RA_METHOD(setMarker);
DECL_RA_METHOD(getMarker);

#undef DECL_RA_METHOD

void ngeRA_init()
{
	GetEnv();

	cRawAudio = (*env)->FindClass(env, "org/libnge/nge2/RawAudio");
	if (!cRawAudio)
	{
		LOGE("org.libnge.nge2.RawAudio class is not found.");
		return;
	}

	cRawAudio = (*env)->NewGlobalRef(env, cRawAudio);

#define GET_COR_METHOD(name, signture) RA_METHOD(name) = \
		(*env)->GetMethodID(env, cRawAudio, "<init>", signture)

#define GET_STATIC_METHOD(method, signture) RA_METHOD(method) = \
		(*env)->GetStaticMethodID(env, cRawAudio, #method, signture)

#define GET_METHOD(method, signture) RA_METHOD(method) = \
		(*env)->GetMethodID(env, cRawAudio, #method, signture)

	GET_COR_METHOD(RawAudio, "()V");

	GET_STATIC_METHOD(getMinBufferSize, "(III)I");

	GET_METHOD(init, "(IIIII)V");
	GET_METHOD(release, "()V");

	GET_METHOD(setListener, "(II)V");
	GET_METHOD(clearListener, "()V");

	GET_METHOD(play, "()V");
	GET_METHOD(stop, "()V");
	GET_METHOD(pause, "()V");

	GET_METHOD(getPlayState, "()I");
	GET_METHOD(reloadStaticData, "()I");
	GET_METHOD(setLoopPoints, "(III)I");

	GET_METHOD(getPosition, "()I");
	GET_METHOD(setPosition, "(I)I");

	GET_METHOD(write, "([BII)I");

	GET_METHOD(setVolume, "(F)I");
	GET_METHOD(getVolume, "()F");

	GET_METHOD(setMarker, "(I)I");
	GET_METHOD(getMarker, "()I");
}

void ngeRA_release()
{
	GetEnv();

	(*env)->DeleteGlobalRef(env, cRawAudio);
}

int ngeRA_getMinBufferSize(int sampleRateInHz, int channelConfig, int audioFormat)
{
	GetEnv();

	return (*env)->CallStaticIntMethod(env, cRawAudio, RA_METHOD(getMinBufferSize),
									   sampleRateInHz, channelConfig, audioFormat);
}

void* ngeRA_new(int sampleRateInHz, int channelConfig, int audioFormat, int bufferSizeInBytes, int mode)
{
	jobject obj;

	GetEnv();

	obj = (*env)->NewObject(env, cRawAudio, RA_METHOD(RawAudio));

	(*env)->CallNonvirtualVoidMethod(env, obj, cRawAudio, RA_METHOD(init), sampleRateInHz, channelConfig, audioFormat, bufferSizeInBytes, mode);

	obj = (*env)->NewGlobalRef(env, obj);

	return (void*)obj;
}

void ngeRA_delete(void* audio)
{
	GetEnv();

	(*env)->CallNonvirtualVoidMethod(env, (jobject)audio, cRawAudio, RA_METHOD(release));
	(*env)->DeleteGlobalRef(env, (jobject)audio);
}

void ngeRA_play(void* audio)
{
	GetEnv();

	(*env)->CallNonvirtualVoidMethod(env, (jobject)audio, cRawAudio, RA_METHOD(play));
}
void ngeRA_stop(void* audio)
{
	GetEnv();

	(*env)->CallNonvirtualVoidMethod(env, (jobject)audio, cRawAudio, RA_METHOD(stop));
}
void ngeRA_pause(void* audio)
{
	GetEnv();

	(*env)->CallNonvirtualVoidMethod(env, (jobject)audio, cRawAudio, RA_METHOD(pause));
}

int ngeRA_getPlayState(void* audio)
{
	GetEnv();

	return (*env)->CallNonvirtualIntMethod(env, (jobject)audio, cRawAudio, RA_METHOD(getPlayState));
}

int ngeRA_reloadStaticData(void* audio)
{
	GetEnv();

	return (*env)->CallNonvirtualIntMethod(env, (jobject)audio, cRawAudio, RA_METHOD(reloadStaticData));
}

int ngeRA_setLoopPoints(void* audio, int startInFrames, int endInFrames, int loopCount)
{
	GetEnv();

	return (*env)->CallNonvirtualIntMethod(env, (jobject)audio, cRawAudio, RA_METHOD(setLoopPoints), startInFrames, endInFrames, loopCount);
}

/* byteArray is java object byte[] */
int ngeRA_writeArray(void* audio, void* byteArray, int offsetInBytes, int sizeInBytes)
{
	GetEnv();

	return (*env)->CallNonvirtualIntMethod(env, (jobject)audio, cRawAudio, RA_METHOD(write), (jarray)byteArray, offsetInBytes, sizeInBytes);
}
int ngeRA_write(void* audio, void* pbuffer, int offsetInBytes, int sizeInBytes)
{
	int i=0;
	jarray buffer;
	uint8_t* pData = (uint8_t*)pbuffer;
	uint8_t* p;

	GetEnv();

    buffer = (*env)->NewByteArray(env, sizeInBytes-offsetInBytes);
	p = (uint8_t*)((*env)->GetPrimitiveArrayCritical(env, buffer, NULL));

	for (i=offsetInBytes; i<sizeInBytes; i++, p++)
	{
		*p = pData[i];
	}

	(*env)->ReleasePrimitiveArrayCritical(env, buffer, p, 0);

	return (*env)->CallNonvirtualIntMethod(env, (jobject)audio, cRawAudio, RA_METHOD(write), buffer, 0, sizeInBytes-offsetInBytes);
}

int ngeRA_setVolume(void* audio, float volume)
{
	GetEnv();

	return (*env)->CallNonvirtualIntMethod(env, (jobject)audio, cRawAudio, RA_METHOD(setVolume), volume);
}
float ngeRA_getVolume(void* audio)
{
	GetEnv();

	return (*env)->CallNonvirtualFloatMethod(env, (jobject)audio, cRawAudio, RA_METHOD(getVolume));
}

int ngeRA_setMarker(void* audio, int markerInFrames)
{
	GetEnv();

	return (*env)->CallNonvirtualIntMethod(env, (jobject)audio, cRawAudio, RA_METHOD(setMarker), markerInFrames);
}
int ngeRA_getMarker(void* audio)
{
	GetEnv();

	return (*env)->CallNonvirtualIntMethod(env, (jobject)audio, cRawAudio, RA_METHOD(getMarker));
}

int ngeRA_setPosition(void* audio, int frames)
{
	GetEnv();

	return (*env)->CallNonvirtualIntMethod(env, (jobject)audio, cRawAudio, RA_METHOD(setPosition), frames);
}
int ngeRA_getPosition(void* audio)
{
	GetEnv();

	return (*env)->CallNonvirtualIntMethod(env, (jobject)audio, cRawAudio, RA_METHOD(getPosition));
}

void ngeRA_setListener(void* audio, ngeRA_notify_fn cbPtr, void* pCookie)
{
	GetEnv();

	(*env)->CallNonvirtualVoidMethod(env, (jobject)audio, cRawAudio, RA_METHOD(setListener), (int)cbPtr, (int)pCookie);
}
void ngeRA_clearListener(void* audio)
{
	GetEnv();

	(*env)->CallNonvirtualVoidMethod(env, (jobject)audio, cRawAudio, RA_METHOD(clearListener));
}
