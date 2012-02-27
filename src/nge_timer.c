#include "nge_platform.h"
#include "nge_debug_log.h"
#include "nge_timer.h"
#include <stdlib.h>
#include <string.h>

#if defined NGE_LINUX || defined NGE_IPHONE || defined NGE_ANDROID
#define NGE_UNIX
#endif

static uint32 TM_get_ticks(nge_timer* timer);
static uint32 TM_is_started(nge_timer* timer);
static uint32 TM_is_paused(nge_timer* timer);
static void TM_start(nge_timer* timer);
static void TM_stop(nge_timer* timer);
static void TM_pause(nge_timer* timer);
static void TM_unpause(nge_timer* timer);

nge_timer* timer_create()
{
	nge_timer* timer = (nge_timer*)malloc(sizeof(nge_timer));
	memset(timer,0,sizeof(nge_timer));
	timer->get_ticks = TM_get_ticks;
	timer->is_started = TM_is_started;
	timer->is_paused = TM_is_paused;
	timer->start = TM_start;
	timer->stop = TM_stop;
	timer->pause = TM_pause;
	timer->unpause = TM_unpause;
	return timer;
}

void timer_free(nge_timer* timer)
{
	SAFE_FREE(timer);
}

static uint32 TM_get_ticks(nge_timer* timer)
{
	//If the timer is running
	if( timer->started == 1 )
	{
	//If the timer is paused
	if( timer->paused == 1 )
	{
		//Return the number of ticks when the the timer was paused
		return timer->pausedTicks;
	}
	else
	{
		//Return the current time minus the start time
		return nge_get_tick() - timer->startTicks;
	}
	}
   	return 0;
}
static uint32 TM_is_started(nge_timer* timer)
{
	return timer->started;
}
static uint32 TM_is_paused(nge_timer* timer)
{
	return timer->paused;
}
static void TM_start(nge_timer* timer)
{
	timer->started = 1;
	timer->paused = 0;
	timer->startTicks = nge_get_tick();
}
static void TM_stop(nge_timer* timer)
{
   timer->started = 0;
   timer->paused = 0;
}
static void TM_pause(nge_timer* timer)
{
	//If the timer is running and isn't already paused
	if( ( timer->started == 1 ) && ( timer->paused == 0 ) )
	{
	//Pause the timer
	timer->paused = 1;
	//Calculate the paused ticks
	timer->pausedTicks = nge_get_tick() - timer->startTicks;
	}
}
static void TM_unpause(nge_timer* timer)
{
	//If the timer is paused
	if( timer->paused == 1 )
	{
	//Unpause the timer
	timer->paused = 0;

	//Reset the starting ticks
	timer->startTicks = nge_get_tick() - timer->pausedTicks;

	//Reset the paused ticks
	timer->pausedTicks = 0;
	}
}

#if defined NGE_PSP
#include <psptypes.h>
#include <time.h>
#include <psprtc.h>
static u64 mTickFrequency = 0;
#elif defined NGE_WIN
#include <winbase.h>
#elif defined NGE_UNIX
#include <sys/time.h>
#endif

uint32 nge_get_tick()
{
#if defined NGE_UNIX
	static struct timeval start;
	static uint8 uninited = 1;
	uint32 ticks;
	struct timeval now;
	if (uninited) {
		uninited = 0;
		gettimeofday(&start, NULL);
	}

	gettimeofday(&now, NULL);
	ticks=(now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000;
	return(ticks);
#elif defined NGE_WIN
	DWORD now;
	now = GetTickCount();
	return now;
#elif defined NGE_PSP
	u64 ticks;
	uint32 tick32;

	if(mTickFrequency == 0)
		mTickFrequency = sceRtcGetTickResolution();
	sceRtcGetCurrentTick(&ticks);
	tick32 = ticks/1000;
	return tick32;
#endif
}
