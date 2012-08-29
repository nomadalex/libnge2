/**
 * @file  nge_ref_handle.c
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/18 03:30:10
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */
#include "nge_ref_handle.h"
#include <stdlib.h>
#include <string.h>

typedef struct ngeRefHandleImpl
{
	void* ptr;
	fn_ngeDeleteCallback cb;
	int *ref;
} ngeRefHandleImpl;

ngeRefHandle ngeRefHandleCreate(fn_ngeDeleteCallback cb, void* ptr) {
	ngeRefHandleImpl* handle = (ngeRefHandleImpl*)malloc(sizeof(ngeRefHandleImpl));
	handle->ref = (int*)malloc(sizeof(int));

	*handle->ref = 1;
	handle->ptr = ptr;
	handle->cb = cb;

	return (ngeRefHandle)handle;
}

void ngeRefHandleRelease(ngeRefHandle handle) {
	ngeRefHandleImpl* h = (ngeRefHandleImpl*)handle;
	(*h->ref)--;
	if (*h->ref == 0) {
		h->cb(h->ptr);
		free(h->ref);
	}

	free(h);
}

ngeRefHandle ngeRefHandleCopy(ngeRefHandle handle) {
	ngeRefHandleImpl* h = (ngeRefHandleImpl*)handle;
	ngeRefHandleImpl* o;

	(*h->ref)++;

	o = (ngeRefHandleImpl*)malloc(sizeof(ngeRefHandleImpl));
	memcpy(o, h, sizeof(ngeRefHandleImpl));
	return (ngeRefHandle)o;
}

void ngeRef(ngeRefHandle handle) {
	ngeRefHandleImpl* h = (ngeRefHandleImpl*)handle;
	(*h->ref)++;
}

void ngeUnref(ngeRefHandle handle) {
	ngeRefHandleImpl* h = (ngeRefHandleImpl*)handle;
	(*h->ref)--;
}

static void Delete(void* ptr) {
	free(ptr);
}

ngeRefHandle ngeCreateNormalHandle(void* ptr) {
	return ngeRefHandleCreate(Delete, ptr);
}
