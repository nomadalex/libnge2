/**
 * @file  nge_package_zip.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/18 07:57:43
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _NGE_PACKAGE_ZIP_H
#define _NGE_PACKAGE_ZIP_H

#include "nge_common.h"
#include "nge_package.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *初始化zip处理
 */
	NGE_API void ngePackageZipInit();

/**
 *为一个zip压缩包设置密码
 *@param[in, out] pkg 待设置的zip包
 *@param[in] passwd 要设置的密码
 */
	NGE_API void ngePackageZipSetPassword(ngePackage* pkg, const char* passwd);

#ifdef __cplusplus
}
#endif

#endif /* _NGE_PACKAGE_ZIP_H */
