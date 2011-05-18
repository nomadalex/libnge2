#include "nge_debug_log.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#ifdef _PSP
#include <psploadexec_kernel.h>
#endif

#ifndef NDEBUG

#if defined(__linux__) || defined(_PSP)
#define _vsnprintf  vsnprintf
#endif

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

void nge_log(const char* pMessage, ...)
{
	va_list	ArgPtr;

	va_start (ArgPtr, pMessage);
	nge_printf(&g_logfile, "neg_log.txt", pMessage, ArgPtr);
	va_end (ArgPtr);
}

void nge_error(const char* pMessage, ...)
{
	va_list	ArgPtr;

	va_start (ArgPtr, pMessage);
	nge_printf(&g_errorfile, "neg_error.txt", pMessage, ArgPtr);
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
