//kid ! do not modify this file if you do not know what to do! X:D
#ifndef	__NGE_DEBUG_LOG_H__
#define	__NGE_DEBUG_LOG_H__

#include "nge_common.h"

#ifdef MMGR
#include "nge_mmgr.h"
#endif

/**
 * debug log function
 */
#ifndef NDEBUG
#if defined NGE_PSP
#include <stdio.h>
#define printf pspDebugScreenPrintf
#endif

#ifdef __cplusplus
extern "C" {
#endif

void nge_log(const char* pMessage, ...);
void nge_error(const char* pMessage, ...);
void nge_debug_quitgame();
void nge_mem_log();
#define nge_line() nge_log("%s(line:%d): ",__FILE__,__LINE__)
#define nge_print nge_log

#ifdef __cplusplus
}
#endif
#else // defined NDEBUG
#ifndef NGE_NEED_PRINTF
#include <stdio.h>
#ifdef WIN32
#define printf()
#else
#define printf(...)
#endif
#endif

#ifdef WIN32
#pragma warning(disable :4002)
#define	nge_log()
#define nge_print nge_log
#define	nge_error()
#define nge_debug_quitgame();
#define nge_line()
#else
#define	nge_log(...)
#define nge_print nge_log
#define	nge_error(...)
#define nge_debug_quitgame(...);
#define nge_line(...)
#endif
#endif // _DEBUG_LOG

#endif // __NGELOG_H__
