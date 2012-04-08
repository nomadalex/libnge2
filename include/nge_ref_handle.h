/**
 * @file  nge_ref_handle.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/18 03:14:37
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _NGE_REF_HANDLER_H
#define _NGE_REF_HANDLER_H

#include "nge_common.h"

typedef struct ngeRefHandle_s
{
	void* ptr;
} *ngeRefHandle;

#define NGE_REF_CAST(type, ref) ((type)(ref->ptr))

typedef void (*fn_ngeDeleteCallback)(void* ptr);

#ifdef __cplusplus
extern "C" {
#endif

	NGE_API void ngeRef(ngeRefHandle handle);
	NGE_API void ngeUnref(ngeRefHandle handle);

	NGE_API ngeRefHandle ngeRefHandleCreate(fn_ngeDeleteCallback cb, void* ptr);
	NGE_API ngeRefHandle ngeRefHandleCopy(ngeRefHandle handle);
	NGE_API void ngeRefHandleRelease(ngeRefHandle handle);

	/* Normal handler, delete by free (stdlib.h) */
	NGE_API ngeRefHandle ngeCreateNormalHandle(void* ptr);
#ifdef __cplusplus
}
#endif

#endif /* _NGE_REF_HANDLER_H */
