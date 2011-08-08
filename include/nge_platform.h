/**
 * @file  nge_platform.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2011/08/07 09:11:47
 *
 *  Copyright  2011  Kun Wang <ifreedom.cn@gmail.com>
 */

#ifndef _NGE_PLATFORM_H
#define _NGE_PLATFORM_H

#if defined WIN32
#define NGE_WIN

#elif defined _PSP
#define NGE_PSP

#elif defined __linux__
#if !defined IPHONEOS && !defined ANDROID
#define NGE_LINUX

#elif defined IPHONEOS
#define NGE_IPHONE

#elif defined ANDROID
#define NGE_ANDROID
#endif
#endif

#endif /* _NGE_PLATFORM_H */
