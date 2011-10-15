/**
 * @file  audio_android.c
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2011/06/12 14:56:05
 *
 *  Copyright  2011  Kun Wang <ifreedom.cn@gmail.com>
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
#include "audio_interface.h"
#include <jni.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>

#define FALSE 0
#define TRUE 1

#define  LOG_TAG    "libcoolaudio"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define WARN_UN_IMP() LOGI("%s not implementation!\n", __FUNCTION__)

static JavaVM* javaVM = NULL;
static JNIEnv* env = NULL;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	javaVM = vm;
	return JNI_VERSION_1_2;
}

inline static JNIEnv* GetEnv()
{
	if (javaVM) (*javaVM)->GetEnv(javaVM, (void**)&env, JNI_VERSION_1_2);
	return env;
}

static jclass cLibCoolAudio = NULL;

#define CA_METHOD(method) mLibCoolAudio_##method
#define MAKE_CA_METHOD(name) static jmethodID CA_METHOD(name)
MAKE_CA_METHOD(LibCoolAudio);
MAKE_CA_METHOD(init);
MAKE_CA_METHOD(finalize);
MAKE_CA_METHOD(load);
MAKE_CA_METHOD(loadBuf);
MAKE_CA_METHOD(play);
MAKE_CA_METHOD(pause);
MAKE_CA_METHOD(resume);
MAKE_CA_METHOD(stop);
MAKE_CA_METHOD(volume);
MAKE_CA_METHOD(rewind);
MAKE_CA_METHOD(seek);
MAKE_CA_METHOD(iseof);
MAKE_CA_METHOD(ispaused);
#undef MAKE_CA_METHOD

/**
 * Cache LibCoolAudio class and it's method id's
 * And do this only once!
 */
inline void load_ca_methods()
{
	GetEnv();

	if (!cLibCoolAudio)
	{
		cLibCoolAudio = (*env)->FindClass(env, "org/libnge/nge2/LibCoolAudio");
		if (!cLibCoolAudio)
		{
			LOGE("org.libnge.nge2.LibCoolAudio class is not found.");
			return;
		}

		cLibCoolAudio = (*env)->NewGlobalRef(env, cLibCoolAudio);

		CA_METHOD(LibCoolAudio) = (*env)->GetMethodID(env, cLibCoolAudio, "<init>", "()V");

#define GET_CA_METHOD(method, signture) CA_METHOD(method) = (*env)->GetMethodID( \
			env, cLibCoolAudio, #method, signture)

		GET_CA_METHOD(init, "()I");
		GET_CA_METHOD(finalize, "()V");
		GET_CA_METHOD(load, "(Ljava/lang/String;)V");
		GET_CA_METHOD(loadBuf, "(Ljava/io/FileDescriptor;JJ)V");
		GET_CA_METHOD(play, "(I)V");
		GET_CA_METHOD(pause, "()V");
		GET_CA_METHOD(resume, "()V");
		GET_CA_METHOD(stop, "()V");
		GET_CA_METHOD(volume, "(I)I");
		GET_CA_METHOD(rewind, "()V");
		GET_CA_METHOD(seek, "(II)V");
		GET_CA_METHOD(iseof, "()Z");
		GET_CA_METHOD(ispaused, "()Z");
#undef GET_CA_METHOD
	}
}

void CoolAudioDefaultInit()
{
	load_ca_methods();
}

void CoolAudioDefaultFini()
{
	GetEnv();
	(*env)->DeleteGlobalRef(env, cLibCoolAudio);
}

jobject createFD(JNIEnv* env, int fd)
{
	jobject fdsc;
	jclass cls;
	jmethodID cons_mid;
	jfieldID field;
	cls = (*env)->FindClass(env, "java/io/FileDescriptor");
	cons_mid = (*env)->GetMethodID(env, cls, "<init>", "()V");
	fdsc = (*env)->NewObject(env, cls, cons_mid);
	field = (*env)->GetFieldID(env, cls, "fd", "I");
	(*env)->SetIntField(env, fdsc, field, fd);
	return fdsc;
}

typedef struct audio_media_player
{
	fd_load       load;
	fd_load_fp    load_fp;
	fd_load_buf   load_buf;
	fd_play       play;
	fd_playstop   playstop;
	fd_pause      pause;
	fd_stop       stop;
	fd_resume     resume;
	fd_volume     volume;
	fd_rewind     rewind;
	fd_seek       seek;
	fd_iseof      iseof;
	fd_ispaused   ispaused;
	fd_destroy    destroy;

// private:
	int fd;
	jobject obj;
} audio_media_player_t;

#define _METHOD(name) audio_##name
#define MAKE_METHOD(return_type,name,args) static return_type _METHOD(name) args
#define DEBUGME() //LOGI("this is in %s.\n", __FUNCTION__)

MAKE_METHOD(int, load, (audio_media_player_t* p, const char* filename))
{
	DEBUGME();
	GetEnv();

	jstring fn = (*env)->NewStringUTF(env, filename);
	(*env)->CallVoidMethod(env, p->obj, CA_METHOD(load), fn);
	(*env)->DeleteLocalRef(env, fn);
	return 0;
}

MAKE_METHOD(int, load_buf, (audio_media_player_t* p, const char* buf, int size))
{
	DEBUGME();
	GetEnv();

	int fd[2];
	if (pipe(fd) < 0) {
		printf("Can not open pipe!\n");
		return -1;
	}

	write(fd[1], (void*)buf, size);
	close(fd[1]);

	if (p->fd)
		close(p->fd);
	p->fd = fd[0];
	jclass jfd = createFD(env, fd[0]);
	(*env)->CallVoidMethod(env, p->obj, CA_METHOD(loadBuf), jfd, 0, size);
	(*env)->DeleteLocalRef(env, jfd);
	return 0;
}

MAKE_METHOD(int, load_fp, (audio_media_player_t* p, int handle, char closed_by_me))
{
	WARN_UN_IMP();
	return -1;
}

MAKE_METHOD(int, play, (audio_media_player_t* p, int times, int free_when_stop))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, p->obj, CA_METHOD(play), times);
	return 0;
}

MAKE_METHOD(int, playstop, (audio_media_player_t* p))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, p->obj, CA_METHOD(play), 1);
	return 0;
}

MAKE_METHOD(void, pause, (audio_media_player_t* p))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, p->obj, CA_METHOD(pause));
}

MAKE_METHOD(void, resume, (audio_media_player_t* p))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, p->obj, CA_METHOD(resume));
}

MAKE_METHOD(int, stop, (audio_media_player_t* p))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, p->obj, CA_METHOD(stop));
	if (p->fd) {
		close(p->fd);
		p->fd = 0;
	}
	return 0;
}

MAKE_METHOD(int, volume, (audio_media_player_t* p, int volume))
{
	DEBUGME();
	GetEnv();

	volume = (int)((float)volume/128*100);
	volume = (*env)->CallIntMethod(env, p->obj, CA_METHOD(volume), volume);
	return (int)((float)volume*1.28);
}

MAKE_METHOD(void, rewind, (audio_media_player_t* p))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, p->obj, CA_METHOD(rewind));
}

MAKE_METHOD(void, seek, (audio_media_player_t* p, int ms, int flag))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, p->obj, CA_METHOD(seek), ms, flag);
}

MAKE_METHOD(int, iseof, (audio_media_player_t* p))
{
	DEBUGME();
	GetEnv();

	if ((*env)->CallBooleanMethod(env, p->obj, CA_METHOD(iseof)))
		return TRUE;
	else
		return FALSE;
}

MAKE_METHOD(int, ispaused, (audio_media_player_t* p))
{
	DEBUGME();
	GetEnv();

	if ((*env)->CallBooleanMethod(env, p->obj, CA_METHOD(ispaused)))
		return TRUE;
	else
		return FALSE;
}

MAKE_METHOD(int, destroy, (audio_media_player_t* p))
{
	DEBUGME();
	GetEnv();

	(*env)->DeleteGlobalRef(env, p->obj);
	if (p->fd)
		close(p->fd);
	free(p);
	return 0;
}

MAKE_METHOD(audio_media_player_t*, init, ())
{
	audio_media_player_t* p = (audio_media_player_t*)malloc(sizeof(audio_media_player_t));

	DEBUGME();
	GetEnv();

#define LOAD_METHOD(name) p->name = (fd_##name) _METHOD(name)
	LOAD_METHOD(load);
	LOAD_METHOD(load_buf);
	LOAD_METHOD(load_fp);
	LOAD_METHOD(play);
	LOAD_METHOD(playstop);
	LOAD_METHOD(pause);
	LOAD_METHOD(stop);
	LOAD_METHOD(resume);
	LOAD_METHOD(volume);
	LOAD_METHOD(rewind);
	LOAD_METHOD(seek);
	LOAD_METHOD(iseof);
	LOAD_METHOD(ispaused);
	LOAD_METHOD(destroy);
#undef LOAD_METHOD

	p->obj = (*env)->NewObject(env, cLibCoolAudio, CA_METHOD(LibCoolAudio));
	p->obj = (*env)->NewGlobalRef(env, p->obj);

	int ret = (*env)->CallIntMethod(env, p->obj, CA_METHOD(init));
	if (ret) {
		LOGE("ERROR in init!\n");
		_METHOD(destroy)(p);
		return NULL;
	}

	p->fd = 0;

	return p;
}

#undef MAKE_METHOD

audio_play_p CreateWavPlayer()
{
	return (audio_play_p) _METHOD(init)();
}

audio_play_p CreateMp3Player()
{
	return (audio_play_p) _METHOD(init)();
}

audio_play_p CreateOggPlayer()
{
	return (audio_play_p) _METHOD(init)();
}
