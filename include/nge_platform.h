/**
 * @file  nge_platform.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2011/08/07 09:11:47
 *
 *  Copyright  2011  Kun Wang <ifreedom.cn@gmail.com>
 */

#ifndef _NGE_PLATFORM_H
#define _NGE_PLATFORM_H

#if defined _PSP
#define NGE_PSP

#elif defined TARGET_OS_IPHONE || defined TARGET_IPHONE_SIMULATOR
#define NGE_IPHONE

#elif defined __ANDROID_API__ || defined ANDROID
#define NGE_ANDROID

#elif defined _WIN32
#define NGE_WIN

#elif defined __APPLE__
#define NGE_MAC

/*
  put linux to last one, because some other platform
  is build on top of linux, like android
*/
#elif defined __linux__
#define NGE_LINUX

#endif

#if defined NGE_PSP
#define NGE_PLATFORM_CONSOLE
#endif

#if defined NGE_IPHONE || defined NGE_ANDROID
#define NGE_PLATFORM_MOBILE
#endif

#if defined NGE_WIN || defined NGE_LINUX || defined NGE_MAC
#define NGE_PLATFORM_DESKTOP
#endif

#endif /* _NGE_PLATFORM_H */
