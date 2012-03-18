/**
 * @file  nge_package_zip.c
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/18 07:57:30
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */
#include "nge_package.h"
#include "nge_package_internal.h"
#include "nge_package_zip.h"
#include <stdlib.h>
#include "unzip.h"

typedef struct ngePackageZip {
	ngePackageOperation* op;
	unzFile file;
} ngePackageZip;

static ngePackage* Open(const char* fname,int option);

static void Close(ngePackage* pkg) {
	ngePackageZip* zip = (ngePackageZip*)pkg;
	unzClose(zip->file);
	free(zip);
}

static int OpenFile(const char* fname,int flag) {
	return 0;
}

static ngePackageOperation zipOperation = {
	Open,
	Close,
	OpenFile
};

ngePackage* Open(const char* fname,int option) {
	ngePackageZip* zip = (ngePackageZip*)malloc(sizeof(ngePackageZip));
	zip->file = unzOpen(fname);
	zip->op = &zipOperation;

	if (zip->file == NULL) {
		free(zip);
		return NULL;
	}
	return (ngePackage*)zip;
}

static ngePackageFormat zipPackage = {
	"zip",
	&zipOperation,
};

void ngePackageZipInit() {
	ngePackageRegister(&zipPackage);
}
