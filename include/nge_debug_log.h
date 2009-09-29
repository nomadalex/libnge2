//kid ! do not modify this file if you do not know what to do! X:D
#ifndef	__NGE_DEBUG_LOG_H__
#define	__NGE_DEBUG_LOG_H__
#include "nge_define.h" 
/**
 * debug log function
 */
#ifdef _DEBUG_LOG
#ifdef __cplusplus
extern "C"{
#endif
	void	nge_log(const char* pMessage, ...);
	void	nge_error(const char* pMessage, ...);
	void    nge_debug_quitgame();
	void    nge_mem_log();
#define nge_line() nge_log("%s(line:%d): ",__FILE__,__LINE__)
#define nge_print nge_log
#ifdef __cplusplus
}
#endif

#else // _DEBUG_LOG
#ifdef WIN32
#pragma warning(disable :4002)
#define	nge_log() 
#define nge_print nge_log
#define	nge_error()
#define nge_debug_quitgame();
#define nge_line()
#elif defined IPHONEOS
//iphone
#define	nge_log(...) 
#define nge_print nge_log
#define	nge_error(...)
#define nge_line()
#define nge_debug_quitgame();
#else
//psp
#define	nge_log(...) 
#define nge_print nge_log
#define	nge_error(...)
#define nge_line()
#define nge_debug_quitgame();
#endif 
#endif // _DEBUG_LOG

#endif // __NGELOG_H__
