#ifndef NGE_TEX_CACHE_H_
#define NGE_TEX_CACHE_H_

#ifdef __cplusplus
extern "C" {
#endif

void tex_cache_init (int size);
int  tex_cache_getid(int imgid,int* cacheid);
int  tex_cache_free(int imgid);
void tex_cache_fini();
int  tex_cache_add(int i,int texid);

#ifdef __cplusplus
}
#endif

#endif

