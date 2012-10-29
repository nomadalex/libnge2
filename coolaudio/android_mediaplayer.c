/**
 * @file  android_mediaplayer.c
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/10/21 14:50:00
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <jni.h>
#include "nge_android_jni.h"
#include "audio_android.h"

static JNIEnv* env;

#define GetEnv() env = nge_GetEnv()
#define WARN_UN_IMP() LOGI("%s not implementation!\n", __FUNCTION__)

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

void load_CA()
{
	GetEnv();

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

void release_CA()
{
	GetEnv();

	(*env)->DeleteGlobalRef(env, cLibCoolAudio);
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

/* private: */
	int fd;
	jobject obj;
} audio_media_player_t;

#define _METHOD(name) audio_##name
#define MAKE_METHOD(return_type,name,args) static return_type _METHOD(name) args
#define DEBUGME() /*LOGI("this is in %s.\n", __FUNCTION__)*/

MAKE_METHOD(int, load, (audio_media_player_t* p, const char* filename))
{
	jstring fn;

	DEBUGME();
	GetEnv();

	fn = (*env)->NewStringUTF(env, filename);
	(*env)->CallVoidMethod(env, p->obj, CA_METHOD(load), fn);
	(*env)->DeleteLocalRef(env, fn);
	return 0;
}

MAKE_METHOD(int, load_buf, (audio_media_player_t* p, const char* buf, int size))
{
	int fd[2];
	jclass jfd;

	DEBUGME();
	GetEnv();

	if (pipe(fd) < 0) {
		printf("Can not open pipe!\n");
		return -1;
	}

	write(fd[1], (void*)buf, size);
	close(fd[1]);

	if (p->fd)
		close(p->fd);
	p->fd = fd[0];
	jfd = createFD(env, fd[0]);
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

audio_media_player_t* android_mediaplayer_create()
{
	audio_media_player_t* p = (audio_media_player_t*)malloc(sizeof(audio_media_player_t));
	int ret;

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

	ret = (*env)->CallIntMethod(env, p->obj, CA_METHOD(init));
	if (ret) {
		LOGE("ERROR in init!\n");
		_METHOD(destroy)(p);
		return NULL;
	}

	p->fd = 0;

	return p;
}
