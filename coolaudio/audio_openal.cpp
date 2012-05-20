/**
 * @file  audio_openal.cpp
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/22 15:18:25
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */
#include "nge_debug_log.h"
#include "audio_openal.h"

#ifndef _MSC_VER
#include <pthread.h>
#ifdef HAVE_PTHREAD_NP_H
#include <pthread_np.h>
#endif
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <list>

#include <android/log.h>
#define  LOG_TAG    "libcoolaudio"
#include "android_log.h"

typedef pthread_mutex_t CRITICAL_SECTION;
void EnterCriticalSection(CRITICAL_SECTION *cs);
void LeaveCriticalSection(CRITICAL_SECTION *cs);
void InitializeCriticalSection(CRITICAL_SECTION *cs);
void DeleteCriticalSection(CRITICAL_SECTION *cs);

void EnterCriticalSection(CRITICAL_SECTION *cs)
{
    int ret;
    ret = pthread_mutex_lock(cs);
	LOG_ASSERT(ret == 0);
}
void LeaveCriticalSection(CRITICAL_SECTION *cs)
{
    int ret;
    ret = pthread_mutex_unlock(cs);
	LOG_ASSERT(ret == 0);
}
void InitializeCriticalSection(CRITICAL_SECTION *cs)
{
    pthread_mutexattr_t attrib;
    int ret;

    ret = pthread_mutexattr_init(&attrib);
	LOG_ASSERT(ret == 0);

    ret = pthread_mutexattr_settype(&attrib, PTHREAD_MUTEX_RECURSIVE);
#ifdef HAVE_PTHREAD_NP_H
    if(ret != 0)
        ret = pthread_mutexattr_setkind_np(&attrib, PTHREAD_MUTEX_RECURSIVE);
#endif
	LOG_ASSERT(ret == 0);

    ret = pthread_mutex_init(cs, &attrib);
	LOG_ASSERT(ret == 0);

    pthread_mutexattr_destroy(&attrib);
}
void DeleteCriticalSection(CRITICAL_SECTION *cs)
{
    int ret;
    ret = pthread_mutex_destroy(cs);
	LOG_ASSERT(ret == 0);
}
#endif

BOOLEAN StartThread();
void StopThread();

extern "C" BOOLEAN InitAL() {
    ALCdevice *device = alcOpenDevice(NULL);
    if(!device)
    {
        alcGetError(NULL);

		LOGE("Audio Device open failed");
        return FALSE;
    }

    ALCcontext *context = alcCreateContext(device, NULL);
    if(!context || alcMakeContextCurrent(context) == ALC_FALSE)
    {
        if(context)
            alcDestroyContext(context);
        alcCloseDevice(device);

        LOGE("Audio Context setup failed");
        return FALSE;
    }
    alcGetError(device);

	if (StartThread() != TRUE)
		return FALSE;

	LOGI("Audio Context setup success");
    return TRUE;
}

extern "C" void DeInitAL() {
    ALCcontext *context = alcGetCurrentContext();
    ALCdevice *device = alcGetContextsDevice(context);

	StopThread();

    if(!context || !device)
    {
        alcGetError(device);
		return;
    }

    if(alcMakeContextCurrent(NULL) == ALC_FALSE)
    {
        alcGetError(NULL);
        return;
    }

    alcDestroyContext(context);
    alcCloseDevice(device);
    alcGetError(NULL);
}

void ALSleep(float duration) {
    if(duration < 0.0f)
    {
        LOGE("ALSleep Invalid duration");
        return;
    }

    ALuint seconds = (ALuint)duration;
    ALdouble rest = duration - (ALdouble)seconds;

    struct timespec t, remainingTime;
    t.tv_sec = (time_t)seconds;
    t.tv_nsec = (long)(rest*1000000000);

    while(nanosleep(&t, &remainingTime) < 0 && errno == EINTR)
        t = remainingTime;
}

static pthread_t thread;
static CRITICAL_SECTION cs;
static pthread_cond_t non_empty;
static bool need_run = true;
static std::list<IPlayer*> playerList;

static void* ThreadFunc(void* ptr) {
	while (need_run) {
		LockAudio();

		if (playerList.empty()) {
			pthread_cond_wait(&non_empty, &cs);
		}

		std::list<IPlayer*>::iterator iter = playerList.begin();
		for (;iter != playerList.end();) {
			if ((*iter)->op->CheckUpdate(*iter) != 0) {
				iter = playerList.erase(iter);
			}
			else {
				iter++;
			}
		}

		UnlockAudio();
		ALSleep(0.01f);
	}
	return NULL;
}

BOOLEAN StartThread() {
	need_run = true;
	InitializeCriticalSection(&cs);
    if(pthread_create(&thread, NULL, ThreadFunc, NULL) != 0)
		return FALSE;
	return TRUE;
}

void StopThread() {
	need_run = false;
	pthread_join(thread, NULL);
	DeleteCriticalSection(&cs);
}

extern "C" inline void LockAudio() {
	EnterCriticalSection(&cs);
}

extern "C" inline void UnlockAudio() {
	LeaveCriticalSection(&cs);
}

extern "C" void AddActivePlayer(IPlayer* player) {
	bool empty = false;
	LockAudio();
	if (playerList.empty())
		empty = true;

	playerList.push_back(player);

	if (empty)
		pthread_cond_signal(&non_empty);
	UnlockAudio();
}

extern "C" void RemoveActivePlayer(IPlayer* player) {
	LockAudio();
	playerList.remove(player);
	UnlockAudio();
}
