#ifndef NDEBUG

#include "nge_debug_log.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef _PSP
#include <psploadexec_kernel.h>
#endif

#if defined(__linux__) || defined(_PSP)
#define _vsnprintf  vsnprintf
#endif

#if !defined ANDROID && !defined IPHONES
FILE *g_logfile = NULL, *g_errorfile = NULL;

/**
 *  nge_printf
 */
inline void nge_printf (FILE** pFile, const char* filename, const char* pMessage, va_list	ArgPtr)
{
	char	Message[1024] = {0};
	char*   FirstLog = "-------------------------=NGE LOG=-------------------------\n";

	if(!(*pFile)){
		*pFile = fopen (filename, "a+b");
		fwrite (FirstLog, strlen (FirstLog), 1, *pFile);
	}
	_vsnprintf (Message, sizeof (Message), pMessage, ArgPtr);

#ifdef _DEBUG_STDOUT
	printf (Message);
#endif
	fwrite (Message, strlen (Message), 1, *pFile);
//	fclose (*pFile); -- notice: I don't close it, because system will close it when quit.
}
#endif

#ifdef ANDROID
#include <android/log.h>
#define  LOG_TAG "libnge2"
#define NGE_LOG(msg, argptr) __android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, msg, argptr)
#define NGE_ERR(msg, argptr) __android_log_vprint(ANDROID_LOG_ERROR, LOG_TAG, msg, argptr)
#elif defined IPHONES
// -- fixme
#define NGE_LOG(msg, argptr)
#define NGE_ERR(msg, argptr)
#else
#define NGE_LOG(msg, argptr) nge_printf(&g_logfile, "neg_log.txt", msg, argptr)
#define NGE_ERR(msg, argptr) nge_printf(&g_errorfile, "neg_error.txt", msg, argptr)
#endif

void nge_log(const char* pMessage, ...)
{
	va_list	ArgPtr;

	va_start (ArgPtr, pMessage);
	NGE_LOG(pMessage, ArgPtr);
	va_end (ArgPtr);
}

void nge_error(const char* pMessage, ...)
{
	va_list	ArgPtr;

	va_start (ArgPtr, pMessage);
	NGE_ERR(pMessage, ArgPtr);
	va_end (ArgPtr);
}

void nge_mem_log()
{
#ifdef MMGR
	m_dumpMemoryReport();
#endif
}

void nge_debug_quitgame()
{
#if defined(_PSP)
	sceKernelExitGame();
#endif
	exit(-1);
}
#endif
