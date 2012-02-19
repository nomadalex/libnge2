#ifndef NGE_TIMER_H_
#define NGE_TIMER_H_

#include "nge_common.h"

struct tagTimer;

typedef uint32 (*fp_proc_int)(struct tagTimer*);
typedef void (*fp_proc_void)(struct tagTimer*);

typedef struct tagTimer{
	fp_proc_int		get_ticks; /**<  获取time tick */
	fp_proc_void	start;     /**<  开始timer */
	fp_proc_void	stop;      /**<  停止timer */
	fp_proc_void	pause;     /**<  暂停timer */
	fp_proc_void	unpause;   /**<  恢复timer */
	fp_proc_int		is_started;/**<  获取timer是否开始 */
	fp_proc_int		is_paused; /**<  获取timer是否暂停 */
	//privated
	uint32 startTicks;
    uint32 pausedTicks;
    uint8  paused;
    uint8  started;
}nge_timer;

#ifdef __cplusplus
extern "C"{
#endif

/**
 *创建一个timer
 *@return nge_timer*,返回timer的指针
 */
nge_timer* timer_create();

/**
 *释放一个timer
 *@param nge_timer* timer,待释放的timer指针
 *@return 无
 */
void timer_free(nge_timer* timer);

/**
 * 得到系统当前ticks
 *@return ticks
 */
uint32 nge_get_tick();

#ifdef __cplusplus
}
#endif

#endif
