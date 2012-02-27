#ifndef NGE_TEX_CACHE_H_
#define NGE_TEX_CACHE_H_

#include "nge_common.h"

#ifdef __cplusplus
extern "C" {
#endif

	NGE_API void tex_cache_init (int size);
	NGE_API int  tex_cache_getid(int imgid,int* cacheid);
	NGE_API int  tex_cache_free(int imgid);
	NGE_API void tex_cache_fini();
	NGE_API int  tex_cache_add(int i,int texid);
	NGE_API void tex_cache_clear();

#ifdef __cplusplus
}
#endif

#endif

