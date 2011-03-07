#ifndef NGE_MAIN_H_
#define NGE_MAIN_H_

#if defined WIN32 || defined IPHONEOS || defined __linux__
#include <SDL.h>
#elif defined _PSP
#define main	NGE_main
#define printf pspDebugScreenPrintf
#endif

#endif
