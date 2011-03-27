#include "nge_timer.h"

static uint32 TM_get_ticks(nge_timer* timer);
static uint32 TM_is_started(nge_timer* timer);
static uint32 TM_is_paused(nge_timer* timer);
static void TM_start(nge_timer* timer);
static void TM_stop(nge_timer* timer);
static void TM_pause(nge_timer* timer);
static void TM_unpause(nge_timer* timer);

//static uint32 nge_get_tick();


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


#if defined _PSP
#include <psprtc.h>
static u64 mTickFrequency = 0;
uint32 nge_get_tick()
{
	u64 ticks;
	if(mTickFrequency == 0)
		mTickFrequency = sceRtcGetTickResolution();
	sceRtcGetCurrentTick(&ticks);
	uint32 tick32 = ticks/1000;
	return tick32;
}
#else
#include "SDL.h"
uint32 nge_get_tick()
{
	return SDL_GetTicks();
}
#endif
