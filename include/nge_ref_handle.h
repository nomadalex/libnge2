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

/**
 *对一个句柄进行一次引用,会使其引用计数+1
 *@param[in, out] handle 要引用的句柄
 */
	NGE_API void ngeRef(ngeRefHandle handle);

/**
 *对一个句柄解除一次引用,会使其引用计数-1
 *@param[in, out] handle 要解除引用的句柄
 */
	NGE_API void ngeUnref(ngeRefHandle handle);

/**
 *新建一个句柄,初始引用计数为1
 *@param[in] cb 回调函数,在句柄被销毁时调用,不必要有释放空间的行为
 *@param[in] ptr 被句柄指向对象的指针
 *@return handle,新建的句柄
 */
	NGE_API ngeRefHandle ngeRefHandleCreate(fn_ngeDeleteCallback cb, void* ptr);

/**
 *复制一个句柄,会使其引用计数+1,新复制的句柄会重新分配空间
 *@param[in, out] handle 被复制的句柄
 *@return handle,复制得到的句柄
 */
	NGE_API ngeRefHandle ngeRefHandleCopy(ngeRefHandle handle);

/**
 *释放一个句柄
 *@param[in] handle 被释放的句柄
 */
	NGE_API void ngeRefHandleRelease(ngeRefHandle handle);

/**
 *创建一个普通句柄,删除函数为stdlib.h中的free()函数,其他同ngeRefHandleCreate
 *@param[in] ptr 被句柄指向对象的指针
 *@return handle,新建的句柄
 */
	NGE_API ngeRefHandle ngeCreateNormalHandle(void* ptr);
#ifdef __cplusplus
}
#endif

#endif /* _NGE_REF_HANDLER_H */
