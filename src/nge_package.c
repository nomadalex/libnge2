/**
 * @file  nge_package.c
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/18 07:18:54
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */
#include "nge_package.h"
#include "nge_package_internal.h"
#include <string.h>

static ngePackageFormat* formats[NGE_MAX_PACKAGE_FORMAT];
static uint8 len = 0;

int ngePackageRegister(ngePackageFormat* format) {
	if (len > NGE_MAX_PACKAGE_FORMAT)
		return -1;

	formats[len] = format;
	len++;
	return 0;
}

ngePackage* ngeLoadPackage(const char* fname, int option) {
	ngePackageFormat *format = NULL;
	char* p, ext[10];
	int i;

	p = strrchr(fname, '.');
	if (p == NULL)
		return NULL;

	strcpy(ext, p+1);
	for (i=0; i<len; i++) {
		if (strcmp(ext, formats[i]->extension) == 0) {
			format = formats[i];
			break;
		}
	}

	if (format == NULL)
		return NULL;

	return format->op->Open(fname, option);
}

void ngeClosePackage(ngePackage* pkg) {
	pkg->op->Close(pkg);
}

int ngeOpenInPackage(ngePackage* pkg, const char* fname, int flag) {
	return pkg->op->OpenFile(pkg, fname, flag);
}

ngeRefHandle ngeCreatePackageHandle(ngePackage* pkg) {
	return ngeRefHandleCreate((fn_ngeDeleteCallback)ngeClosePackage, (void*)pkg);
}
