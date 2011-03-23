#include "nge_debug_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef NDEBUG

#if defined(_PSP)
#define printf pspDebugScreenPrintf
#endif

#ifdef __linux__
#define _vsnprintf  vsnprintf // linux need it
#endif

FILE *g_logfile = NULL, *g_errorfile = NULL;

/**
 *  nge_printf
 */
void nge_printf (FILE** pFile, const char* filename, const char* pMessage, ...)
{
	va_list	ArgPtr;
	char	Message[1024] = {0};
	char*   FirstLog = "-------------------------=NGE LOG=-------------------------\n";

	if(!(*pFile)){
		*pFile = fopen (filename, "a+b");
		fwrite (FirstLog, strlen (FirstLog), 1, *pFile);
	}
	va_start (ArgPtr, pMessage);
	_vsnprintf (Message, sizeof (Message), pMessage, ArgPtr);
	va_end (ArgPtr);

#ifdef _DEBUG_STDOUT
	printf (Message);
#endif
	fwrite (Message, strlen (Message), 1, *pFile);
//	fclose (*pFile); -- notice: I don't close it, because system will close it when quit.
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
