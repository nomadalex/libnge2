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
#include "nge_io_file_internal.h"
#include "nge_package_zip.h"
#include <stdlib.h>
#include <string.h>
#include "unzip.h"

typedef struct ngePackageZip {
	ngePackageOperation* op;
	char *fname, *passwd;
} ngePackageZip;

static ngePackage* Open(const char* fname,int option);
static void Close(ngePackage* pkg);
static int OpenFile(ngePackage* pkg, const char* fname,int flag);

static ngePackageOperation zipOperation = {
	Open,
	Close,
	OpenFile
};

ngePackage* Open(const char* fname,int option) {
	ngePackageZip* zip = (ngePackageZip*)malloc(sizeof(ngePackageZip));
	int len = strlen(fname);

	zip->fname = (char*)malloc(len+1);
	strcpy(zip->fname, fname);
	zip->passwd = NULL;

	zip->op = &zipOperation;
	return (ngePackage*)zip;
}

void Close(ngePackage* pkg) {
	ngePackageZip* zip = (ngePackageZip*)pkg;

	free(zip->fname);
	if (zip->passwd != NULL)
		free(zip->passwd);

	free(zip);
}

void ngePackageZipSetPassword(ngePackage* pkg, const char* passwd) {
	ngePackageZip* zip = (ngePackageZip*)pkg;
	int len = strlen(passwd);

	zip->passwd = (char*)malloc(len+1);
	strcpy(zip->passwd, passwd);
}

static ngePackageFormat zipPackage = {
	"zip",
	&zipOperation,
};

void ngePackageZipInit() {
	ngePackageRegister(&zipPackage);
}

/* zip file operations */
typedef struct {
	ngeVFOperation* op;
	unzFile file;
} ngeVFZip;

static int FileClose(ngeVF* f) {
	ngeVFZip* zip = (ngeVFZip*)f;
	unzCloseCurrentFile(zip->file);
	unzClose(zip->file);
	free(zip);
	return 0;
}

static int Read(void *buf, size_t size, size_t n, ngeVF* f) {
	ngeVFZip* zip = (ngeVFZip*)f;

	return unzReadCurrentFile(zip->file, buf, size*n);
}

static int Tell(ngeVF *f) {
	ngeVFZip* zip = (ngeVFZip*)f;

	return unztell(zip->file);
}

static int Eof(ngeVF *f) {
	ngeVFZip* zip = (ngeVFZip*)f;

	return unzeof(zip->file);
}

static int Size(ngeVF* f) {
	ngeVFZip* zip = (ngeVFZip*)f;
	unz_file_info info;

	if(unzGetCurrentFileInfo(zip->file, &info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK)
		return -1;

	return info.uncompressed_size;
}

static ngeVFOperation zipFileOperation = {
	FileClose,
	Read,
	NULL,
	NULL,
	Tell,
	Eof,
	Size
};

int OpenFile(ngePackage* pkg, const char* fname,int flag) {
	ngePackageZip* zip = (ngePackageZip*)pkg;
	ngeVFZip* f;
	unzFile file;

	if (flag != IO_RDONLY)
		return 0;

	file = unzOpen(zip->fname);
	if (file == NULL)
		return 0;

	if(unzLocateFile(file, fname, 0) != UNZ_OK)
		goto fail;

	if (zip->passwd == NULL) {
		if (unzOpenCurrentFile(file) != UNZ_OK)
			goto fail;
	}
	else {
		if (unzOpenCurrentFilePassword(file, zip->passwd) != UNZ_OK)
			goto fail;
	}

	f = (ngeVFZip*)malloc(sizeof(ngeVFZip));
	f->file = file;

	f->op = &zipFileOperation;
	return ngeVFAdd((ngeVF*)f);

fail:
	unzClose(file);
	return 0;
}
