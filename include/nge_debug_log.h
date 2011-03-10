//kid ! do not modify this file if you do not know what to do! X:D
#ifndef	__NGE_DEBUG_LOG_H__
#define	__NGE_DEBUG_LOG_H__
#include "nge_define.h"
/**
 * debug log function
 */
#ifndef NDEBUG
#ifdef __cplusplus
extern "C"{
#endif
  extern FILE *g_logfile, *g_errorfile;
  void nge_printf (FILE** pFile, const char* filename, const char* pMessage, ...);
#define nge_log(pMessage, ...) nge_printf(&g_logfile, "neg_log.txt", pMessage, __VA_ARGS__)
#define nge_error(pMessage, ...) nge_printf(&g_errorfile, "neg_error.txt", pMessage, __VA_ARGS__)
  void nge_debug_quitgame();
  void nge_mem_log();
#define nge_line() nge_log("%s(line:%d): ",__FILE__,__LINE__)
#define nge_print(msg) nge_printf(&g_logfile, "neg_log.txt", msg)
#ifdef __cplusplus
}
#endif
#else // defined NDEBUG
#ifdef WIN32
#pragma warning(disable :4002)
#endif

#define	nge_log()
#define nge_print nge_log
#define	nge_error()
#define nge_debug_quitgame();
#define nge_line()
#endif // _DEBUG_LOG

#endif // __NGELOG_H__
