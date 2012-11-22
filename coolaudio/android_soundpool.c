/**
 * @file  android_soundpool.c
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/11/14 13:57:04
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */
#include "audio_interface.h"
#include "android_soundpool.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <jni.h>
#include "nge_android_jni.h"
#include "audio_android.h"
#include "nge_graphics.h"
#include "nge_timer.h"
#include "nge_io_file.h"
#include "wav.h"

static screen_context_p screen = NULL;

static JNIEnv* env;

#define GetEnv() env = nge_GetEnv()
#define WARN_UN_IMP() LOGI("%s not implementation!\n", __FUNCTION__)

static jclass cSPManager = NULL;
static jobject managerObj = NULL;

#define SP_METHOD(method) mSP_##method

#define DECL_SP_METHOD(method) static jmethodID SP_METHOD(method)

#define GET_SP_COR(name) SP_METHOD(name) = (*env)->GetMethodID( \
		env, cSPManager, "<init>", "()V")

#define GET_SP_METHOD(method, signture) SP_METHOD(method) = (*env)->GetMethodID( \
		env, cSPManager, #method, signture)


DECL_SP_METHOD(cor);
DECL_SP_METHOD(create);
DECL_SP_METHOD(delete);
DECL_SP_METHOD(loadFd);
DECL_SP_METHOD(load);
DECL_SP_METHOD(play);
DECL_SP_METHOD(pause);
DECL_SP_METHOD(resume);
DECL_SP_METHOD(stop);
DECL_SP_METHOD(setVolume);
DECL_SP_METHOD(isPaused);


void load_SP()
{
	GetEnv();

	screen = GetScreenContext();

	cSPManager = (*env)->FindClass(env, "org/libnge/nge2/SoundpoolManager");
	if (!cSPManager)
	{
		LOGE("org.libnge.nge2.SoundpoolManager class is not found.");
		return;
	}

	cSPManager = (*env)->NewGlobalRef(env, cSPManager);

	GET_SP_COR(cor);

	GET_SP_METHOD(create, "()I");
	GET_SP_METHOD(delete, "(I)V");
	GET_SP_METHOD(loadFd, "(ILjava/io/FileDescriptor;I)I");
	GET_SP_METHOD(load, "(ILjava/lang/String;)I");
	GET_SP_METHOD(play, "(II)V");
	GET_SP_METHOD(pause, "(I)V");
	GET_SP_METHOD(resume, "(I)V");
	GET_SP_METHOD(stop, "(I)V");
	GET_SP_METHOD(setVolume, "(IF)F");
	GET_SP_METHOD(isPaused, "(I)Z");

	managerObj = (*env)->NewObject(env, cSPManager, SP_METHOD(cor));
	managerObj = (*env)->NewGlobalRef(env, managerObj);
}

void release_SP()
{
	GetEnv();

	(*env)->DeleteGlobalRef(env, managerObj);
	(*env)->DeleteGlobalRef(env, cSPManager);
}

typedef struct audio_soundpool
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
	int id;
	int fd;
	float length;
	nge_timer* timer;
} audio_soundpool_t;


#define THIS_DEF audio_soundpool_t* p
#define DECL_MALLOC_THIS() audio_soundpool_t* p = (audio_soundpool_t*)malloc(sizeof(audio_soundpool_t))

#define _METHOD(name) audio_##name
#define MAKE_METHOD(return_type,name,args) static return_type _METHOD(name) args
#define SET_METHOD(name) p->name = (fd_##name) _METHOD(name)

#define DEBUGME()
/* #define DEBUGME() LOGI("this is in %s id : %d.\n", __FUNCTION__, p->id) */

MAKE_METHOD(int, load, (THIS_DEF, const char* filename))
{
	int fd;
	int pos;
	uint8_t* buf;
	wav_info_t info;

	int ret = 0;
	jstring fn;
	char fullname[256]={0};

	DEBUGME();
	GetEnv();

	if (p->fd) {
		close(p->fd);
		p->fd = 0;
	}

	fd = io_fopen(filename, IO_RDONLY);
	if (fd == 0)
		return -1;

	buf = (uint8_t*)malloc(MIN_WAV_HEADER_SIZE);
	io_fread(buf, MIN_WAV_HEADER_SIZE, 1, fd);
	io_fclose(fd);

	pos = 0;
	if (parse_wav_hdr(buf, MIN_WAV_HEADER_SIZE, &pos, &info) == -1) {
		free(buf);
		return -1;
	}
	free(buf);
	p->length = ((float)info.size) / ((float)info.bps / 8.0f * info.rate / 1000.0f * info.channels);

	/* LOGI("load %s, length %f.\n", filename, p->length); */

	sprintf(fullname,"%s/%s",screen->pathname,filename);
	fn = (*env)->NewStringUTF(env, fullname);

	ret = (*env)->CallIntMethod(env, managerObj, SP_METHOD(load), p->id, fn);
	(*env)->DeleteLocalRef(env, fn);
	return ret;
}

MAKE_METHOD(int, load_buf, (THIS_DEF, const char* buf, int size))
{
	int ret = 0;
	int fd[2];
	jclass jfd;

	int pos;
	wav_info_t info;

	DEBUGME();
	GetEnv();

	pos = 0;
	if (parse_wav_hdr(buf, size, &pos, &info) == -1) {
		return -1;
	}
	p->length = ((float)info.size) / ((float)info.bps / 8.0f * info.rate / 1000.0f * info.channels);

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
	ret = (*env)->CallIntMethod(env, managerObj, SP_METHOD(loadFd), p->id, jfd, size);
	(*env)->DeleteLocalRef(env, jfd);
	return ret;
}

MAKE_METHOD(int, load_fp, (THIS_DEF, int handle, char closed_by_me))
{
	WARN_UN_IMP();
	return -1;
}

MAKE_METHOD(int, play, (THIS_DEF, int times, int free_when_stop))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, managerObj, SP_METHOD(play), p->id, times);

	p->timer->start(p->timer);

	return 0;
}

MAKE_METHOD(int, playstop, (THIS_DEF))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, managerObj, SP_METHOD(play), p->id, 1);
	return 0;
}

MAKE_METHOD(void, pause, (THIS_DEF))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, managerObj, SP_METHOD(pause), p->id);

	p->timer->pause(p->timer);
}

MAKE_METHOD(void, resume, (THIS_DEF))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, managerObj, SP_METHOD(resume), p->id);

	p->timer->unpause(p->timer);
}

MAKE_METHOD(int, stop, (THIS_DEF))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, managerObj, SP_METHOD(stop), p->id);

	if (p->fd) {
		close(p->fd);
		p->fd = 0;
	}

	p->timer->stop(p->timer);

	return 0;
}

MAKE_METHOD(int, volume, (THIS_DEF, int volume))
{
	float v, retv;
	DEBUGME();
	GetEnv();

	v = (float)volume/128;
	retv = (*env)->CallFloatMethod( \
		env, managerObj, SP_METHOD(setVolume), p->id, v);

	return (int)(retv*128);
}

MAKE_METHOD(void, rewind, (THIS_DEF))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, managerObj, SP_METHOD(stop), p->id);
}

MAKE_METHOD(void, seek, (THIS_DEF, int ms, int flag))
{
	WARN_UN_IMP();
}

MAKE_METHOD(int, iseof, (THIS_DEF))
{
	uint32_t time;

	/* DEBUGME(); */

	if (p->timer->is_started(p->timer) == 0) return FALSE;

	time = p->timer->get_ticks(p->timer);

	if ((float)time > p->length) {
		GetEnv();
		(*env)->CallVoidMethod(env, managerObj, SP_METHOD(stop), p->id);

		return TRUE;
	}

	return FALSE;
}

MAKE_METHOD(int, ispaused, (THIS_DEF))
{
	DEBUGME();
	GetEnv();

	if ((*env)->CallBooleanMethod(env, managerObj, SP_METHOD(isPaused), p->id))
		return TRUE;
	else
		return FALSE;
}

MAKE_METHOD(int, destroy, (THIS_DEF))
{
	DEBUGME();
	GetEnv();

	(*env)->CallVoidMethod(env, managerObj, SP_METHOD(delete), p->id);

	nge_timer_free(p->timer);

	if (p->fd)
		close(p->fd);
	free(p);
	return 0;
}

audio_play_p android_soundpool_create()
{
	DECL_MALLOC_THIS();

	GetEnv();

	SET_METHOD(load);
	SET_METHOD(load_buf);
	SET_METHOD(load_fp);
	SET_METHOD(play);
	SET_METHOD(playstop);
	SET_METHOD(pause);
	SET_METHOD(stop);
	SET_METHOD(resume);
	SET_METHOD(volume);
	SET_METHOD(rewind);
	SET_METHOD(seek);
	SET_METHOD(iseof);
	SET_METHOD(ispaused);
	SET_METHOD(destroy);

	p->id = (*env)->CallIntMethod(env, managerObj, SP_METHOD(create));
	p->fd = 0;
	p->timer = nge_timer_create();

	DEBUGME();

	return (audio_play_p)p;
}
