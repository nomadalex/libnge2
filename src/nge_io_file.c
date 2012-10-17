#include "nge_platform.h"
#include "nge_debug_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nge_io_file.h"
#include "nge_io_file_internal.h"

typedef struct ngeVFBlock {
	int start;
	ngeVF* f[10];
	struct ngeVFBlock *next, *prev;
	uint8_t len;
} ngeVFBlock;

static ngeVFBlock firstBlock = { 1, {NULL}, NULL, NULL, 0 };
static ngeVFBlock *currentBlock = &firstBlock;
static int nextHandle = 0;

int ngeVFAdd(ngeVF* f) {
	int idx;

	ngeVFBlock *b = currentBlock;
	while (1) { /* find last */
		if (b->next == NULL)
			break;
		b = b->next;
	}

	nextHandle++;
	if (b->len == 0) { /* empty, so reset start */
		b->start = nextHandle;
	}

	idx = nextHandle - b->start;

	if (idx > 9) { /* full */
		ngeVFBlock* next = (ngeVFBlock*)malloc(sizeof(ngeVFBlock));
		memset(next, 0, sizeof(ngeVFBlock));
		next->start = nextHandle;
		next->len = 0;
		next->next = NULL;
		next->prev = b;
		b->next = next;
		b = next;
		idx = 0;
	}

	b->f[idx] = f;
	b->len++;

	currentBlock = b;
	return nextHandle;
}

#define GET_BLOCK_AND_VF(b, f, handle)			\
	b = currentBlock;							\
												\
	if (handle < b->start) {					\
		while (b && handle < b->start)			\
			b = b->prev;						\
	}											\
	else if (handle >= b->start+10) {			\
		while (b && handle >= b->start+10)		\
			b = b->next;						\
	}											\
												\
	if (b == NULL ||							\
		handle < b->start ||					\
		handle >= b->start+10) {				\
		f = NULL;								\
	}											\
	else {										\
		currentBlock = b;						\
		f = b->f[handle - b->start];			\
	}

#define GET_VF(f, handle)						\
	do {										\
		ngeVFBlock* b;							\
		GET_BLOCK_AND_VF(b, f, handle);			\
	} while(0)

#define TEST_F(f, ret)							\
	if (f == NULL)								\
		return ret

#define TEST_OP(f, func, ret)					\
	if (f->op->func == NULL)					\
		return ret

int io_fclose(int handle)
{
	ngeVF* f;
	int ret;
	ngeVFBlock* b;
	GET_BLOCK_AND_VF(b, f, handle);

	TEST_F(f, -1);

	if (f != NULL) {
		ret = f->op->Close(f);

		b->f[handle - b->start] = NULL;
		b->len--;
		if (b->len == 0 && b->prev != NULL) {
			currentBlock = b->prev;

			b->prev->next = b->next;
			if (b->next != NULL)
				b->next->prev = b->prev;

			free(b);
		}
	}

	return ret;
}

int io_fread(void* buf,int count,int size,int handle)
{
	ngeVF* f;
	GET_VF(f, handle);

	TEST_F(f, -1);
	TEST_OP(f, Read, -1);
	return f->op->Read(buf, size, count, f);
}

int io_fwrite(void* buffer,int count,int size,int handle)
{
	ngeVF* f;
	GET_VF(f, handle);

	TEST_F(f, -1);
	TEST_OP(f, Write, -1);
	return f->op->Write(buffer, size, count, f);
}

int io_fseek(int handle,int offset,int flag)
{
	ngeVF* f;
	GET_VF(f, handle);

	TEST_F(f, -1);
	TEST_OP(f, Seek, -1);
	return f->op->Seek(f, offset, flag);
}

int io_ftell(int handle)
{
	ngeVF* f;
	GET_VF(f, handle);

	TEST_F(f, -1);
	TEST_OP(f, Tell, -1);
	return f->op->Tell(f);
}

int io_fsize(int handle)
{
	ngeVF* f;
	GET_VF(f, handle);

	TEST_F(f, -1);
	TEST_OP(f, Size, -1);
	return 	f->op->Size(f);
}

/* local disk operation */
typedef struct {
	ngeVFOperation* op;
#ifdef NGE_PSP
	int handle;
#else
	FILE* handle;
#endif
} ngeVFDisk;

static int Close(ngeVF* f) {
	ngeVFDisk* df = (ngeVFDisk*)f;

#if defined NGE_PSP
	if (df->handle >= 0)
		return sceIoClose(df->handle);
#else
	if (df->handle != 0)
		return fclose(df->handle);
#endif

	free(df);
	return -1;
}

static int Read(void *buf, size_t size, size_t n, ngeVF* f) {
	ngeVFDisk* df = (ngeVFDisk*)f;

#if defined NGE_PSP
	return sceIoRead(df->handle, buf, size*n) / size;
#else
	return fread(buf, size, n,df->handle);
#endif
}

static int Write(const void *buf, size_t size, size_t n, ngeVF* f) {
	ngeVFDisk* df = (ngeVFDisk*)f;

#if defined NGE_PSP
	return sceIoWrite(df->handle, buf, size*n) / size;
#else
	return fwrite(buf, size, n, df->handle);
#endif
}

static int Seek(ngeVF *f, int offset, int whence) {
	ngeVFDisk* df = (ngeVFDisk*)f;

#if defined NGE_PSP
	int ret = sceIoLseek(df->handle, offset, whence);
	if(ret >= 0)
		return 0;
	else
		return -1;
#else
	return fseek(df->handle, offset, whence);
#endif
}

static int Tell(ngeVF *f) {
	ngeVFDisk* df = (ngeVFDisk*)f;

#if defined NGE_PSP
	int ret = sceIoLseek(df->handle, 0, IO_SEEK_CUR);
	return ret;
#else
	return ftell(df->handle);
#endif
}

static int Eof(ngeVF *f) {
	ngeVFDisk* df = (ngeVFDisk*)f;
	int size,cur;
#if defined NGE_PSP
	int handle = df->handle;
	cur = sceIoLseek(handle, 0, IO_SEEK_CUR);
	size = sceIoLseek(handle, 0, IO_SEEK_END);
	sceIoLseek(handle, cur, IO_SEEK_SET);
#else
	FILE* handle = df->handle;
	cur = ftell(handle);
	fseek(handle,0,IO_SEEK_END);
	size = ftell(handle);
	fseek(handle,cur,IO_SEEK_SET);
#endif
	if (size == cur)
		return 1;
	return 0;
}

static int Size(ngeVF* f) {
	ngeVFDisk* df = (ngeVFDisk*)f;
 	int size,cur;

#if defined NGE_PSP
	int handle = df->handle;
	cur = sceIoLseek(handle, 0, IO_SEEK_CUR);
	size = sceIoLseek(handle, 0, IO_SEEK_END);
	sceIoLseek(handle, cur, IO_SEEK_SET);
	return size;
#else
	FILE* handle = df->handle;
	cur = ftell(handle);
	fseek(handle,0,IO_SEEK_END);
	size = ftell(handle);
	fseek(handle,cur,IO_SEEK_SET);
#endif

	return size;
}

static ngeVFOperation disk_operations = {
	Close,
	Read,
	Write,
	Seek,
	Tell,
	Eof,
	Size
};

int io_fopen(const char* fname,int flag)
{
	ngeVFDisk* f;
#if defined NGE_PSP
	int handle = 0;
	if(flag == IO_RDONLY)
		handle = sceIoOpen(fname, PSP_O_RDONLY, 0777);
	else
		handle = sceIoOpen(fname, PSP_O_RDWR|PSP_O_CREAT, 0777);
	handle = handle <=0 ? 0: handle;
	if (handle <= 0)
		return 0;
#else
	char io_array[][4]={"rb","wb","ab"};
	FILE* handle;
	if(flag > 3){
		flag = 0;
	}
	handle = fopen(fname,io_array[flag]);
	if (handle == NULL)
		return 0;
#endif

	f = (ngeVFDisk*)malloc(sizeof(ngeVFDisk));
	f->handle = handle;
	f->op = &disk_operations;

	return ngeVFAdd((ngeVF*)f);
}
