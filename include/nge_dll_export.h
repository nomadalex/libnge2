/**
 * @file  nge_dll_export.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/04 05:53:38
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _NGE_DLL_EXPORT_H
#define _NGE_DLL_EXPORT_H

#ifdef NGE_LIBTYPE_STATIC
#define NGE_API
#else

#if defined WIN32

#ifdef NGE2_EXPORTS
#define NGE_API __declspec(dllexport)
#else
#define NGE_API __declspec(dllimport)
#endif

#else
#define NGE_API
#endif
#endif

#endif /* _NGE_DLL_EXPORT_H */
