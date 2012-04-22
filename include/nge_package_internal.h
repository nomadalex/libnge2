/**
 * @file  nge_package_internal.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/18 07:09:54
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _NGE_PACKAGE_INTERNAL_H
#define _NGE_PACKAGE_INTERNAL_H

#define NGE_MAX_PACKAGE_FORMAT 10

typedef struct ngePackageOperation ngePackageOperation;

typedef struct ngePackageFormat_s {
	char* extension;
	ngePackageOperation* op;
} ngePackageFormat;

struct ngePackageOperation {
	ngePackage* (*Open)(const char* fname,int option);
	void (*Close)(ngePackage* pkg);
	int (*OpenFile)(ngePackage* pkg, const char* fname,int flag);
};

struct ngePackage
{
	ngePackageOperation* op;
};

#ifdef __cplusplus
extern "C" {
#endif

	/**
	   注册新的包文件格式
	 * @param ngePackageFormat*, 包文件格式
	 * @return int, 返回0成功，返回-1失败
	 */
	NGE_API int ngePackageRegister(ngePackageFormat* format);

#ifdef __cplusplus
}
#endif

#endif /* _NGE_PACKAGE_INTERNAL_H */
