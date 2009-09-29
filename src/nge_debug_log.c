#include <windows.h>
#include "nge_debug_log.h"

CONSOLE_SCREEN_BUFFER_INFO   csbiInfo; 
HANDLE   hStdout;
WORD   SetRedText()//切换成红色显示 
{   
	unsigned short   wOldColorAttrs;   
	hStdout=GetStdHandle(STD_OUTPUT_HANDLE);//控制台窗口句柄   
  

//hStdout   =   GetStdHandle(STD_OUTPUT_HANDLE);     
//   Save   the   current   text   colors.     
if   (!   GetConsoleScreenBufferInfo(hStdout,   &csbiInfo))     
{  // MessageBox(NULL,   "GetConsoleScreenBufferInfo",   "Console   Error",   MB_OK);     
return   0;   
}   
wOldColorAttrs   =   csbiInfo.wAttributes;     
if(!SetConsoleTextAttribute(hStdout,   
   FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE))   
{   //MessageBox(NULL,   "SetConsoleTextAttribute",   "Console   Error",   MB_OK);   
return   0;   
}   
return   wOldColorAttrs;   
//     SetConsoleTextAttribute(hStdout,   wOldColorAttrs);   
}   
void   RestoreText(WORD   wOldColorAttrs)   
{   //HANDLE   hStdout;   
	//hStdout   =   GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hStdout,   wOldColorAttrs);   
}






#ifdef _DEBUG_LOG

#if defined WIN32 || defined IPHONEOS
//win32
#else
#define printf pspDebugScreenPrintf
#define _vsnprintf  vsnprintf
#endif
static int first_run = 0;
/**
 *  nge_print
 */
void nge_log (const char* pMessage, ...)
{
	char	Message[1024] = {0};
	char  FileInfo[256] = {0};
	va_list	ArgPtr;
	FILE*	pFile;
	
	static char FirstLog[128]={"-------------------------=NGE LOG=-------------------------\n"};
	SetRedText();
	if(first_run == 0){
		pFile = fopen ("nge_log.txt", "a+b");
		fwrite (FirstLog, strlen (FirstLog), 1, pFile);
		fclose (pFile);
		first_run = 1;
	}
	va_start (ArgPtr, pMessage);
	_vsnprintf (Message, sizeof (Message), pMessage, ArgPtr);
	va_end (ArgPtr);
	
#ifdef _DEBUG_STDOUT
	printf (Message);
#endif
	pFile = fopen ("nge_log.txt", "a+b");

	fwrite (Message, strlen (Message), 1, pFile);

	fclose (pFile);
}

/**
 *  nge_error
 */
void nge_error (const char* pMessage, ...)
{
	char	Message[1024] = {0};
	char  FileInfo[256] = {0};
	va_list	ArgPtr;
	FILE*	pFile;
	static char FirstLog[128]={"-------------------------=NGE LOG=-------------------------\n"};
	if(first_run == 0){
		pFile = fopen ("nge_error.txt", "a+b");
		fwrite (FirstLog, strlen (FirstLog), 1, pFile);
		fclose (pFile);
		first_run = 1;
	}
	va_start (ArgPtr, pMessage);
	_vsnprintf (Message, sizeof (Message), pMessage, ArgPtr);
	va_end (ArgPtr);
#ifdef _DEBUG_STDOUT
	printf (Message);
#endif
	pFile = fopen ("nge_error.txt", "a+b");

	fwrite (Message, strlen (Message), 1, pFile);

	fclose (pFile);
}

void nge_mem_log()
{
#if defined WIN32 || defined IPHONEOS
	#ifdef MMGR
		m_dumpMemoryReport();
	#endif
#endif
}

void nge_debug_quitgame()
{
#if defined WIN32 || defined IPHONEOS
	exit(-1);
#else	
	sceKernelExitGame();
	exit(-1);
#endif			
}
#endif
