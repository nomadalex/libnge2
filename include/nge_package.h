/**
 * @file  nge_package.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/18 07:08:31
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _NGE_PACKAGE_H
#define _NGE_PACKAGE_H

#include "nge_common.h"
#include "nge_ref_handle.h"
#include "nge_io_file.h"

typedef struct ngePackage ngePackage;

#ifdef __cplusplus
extern "C" {
#endif

/**
 *载入打包文件
 *@param const char*,文件名
 *@param int, 一些选项（可选）
 *@return ngePackage*, 返回包文件句柄，失败返回NULL
 */
	NGE_API ngePackage* ngeLoadPackage(const char* fname, int option);
	NGE_API ngeRefHandle ngeCreatePackageHandle(ngePackage* pkg);

	NGE_API void ngeClosePackage(ngePackage* pkg);

/**
 *打开包内文件
 *@param ngePackage*, 包句柄
 *@param const char*,文件名
 *@param int,打开标志(IO_RDONLY,IO_WRONLY,IO_APPEND的一种)
 *@return int,文件句柄,读写均用此文件句柄进行操作
 */
	NGE_API int ngeOpenInPackage(ngePackage* pkg, const char* fname, int flag);

#ifdef __cplusplus
}
#endif

#endif /* _NGE_PACKAGE_H */
