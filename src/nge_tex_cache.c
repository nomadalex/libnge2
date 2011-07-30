#include "nge_debug_log.h"
#include "nge_tex_cache.h"
#include "nge_rbtree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct tag_tex_node{
	int texid;
	struct tag_tex_node* next;
}tex_node_t,*tex_node_p;

static tex_node_p free_tex_list = NULL;
static tex_node_p free_tex_head = NULL;

static int node_size = 0;
static NodeType** deleted_node;
static int delete_count = 0;
static int delete_max = 0;
static rbtree_p rb;

void tex_cache_init (int size)
{
	int i = 0;
	free_tex_list = (tex_node_p)malloc(size*sizeof(tex_node_t));
	free_tex_head = free_tex_list;
	memset(free_tex_list,0,size*sizeof(tex_node_t));
	for(i = 0; i < size-1; i++){
		free_tex_list[i].next = &free_tex_list[i+1];
		free_tex_list[i].texid = i;
	}
	node_size = size;
	delete_max = node_size/4;
	deleted_node =(NodeType**)malloc((delete_max+1)*sizeof(NodeType*));
	rb = createTree();
}


void deleteNodeProc(rbtree_p tree, NodeType *p) {
	//删除一半
	if(delete_count < delete_max){
		deleted_node[delete_count] = p;
		delete_count++;
	}
}

/**
 * 从freelist中获取一个cache
 */
static tex_node_p get_free_node()
{
	tex_node_p ret = NULL;
	tex_node_p tex_node = NULL;
	int i = 0;
	if(free_tex_list == NULL){
		//没有剩余的cache,从树中释放一半的cache
		//root = rbtRoot();
		delete_count = 0;
		rbtInorder(rb,rb->root, deleteNodeProc);
		for(i =0;i<delete_count;i++){
			tex_node = (tex_node_p)(deleted_node[i]->val.node);
			tex_node->next =  free_tex_list;
			free_tex_list = tex_node;
			rbtErase(rb,deleted_node[i]);
		}
	}

	if(free_tex_list){
		ret = free_tex_list;
		free_tex_list = free_tex_list->next;
	}

	return ret;
}

int  tex_cache_add(int i,int texid)
{
	free_tex_list[i].texid = texid;
	return 1;
}

int  tex_cache_getid(int imgid,int* cacheid)
{
	KeyType key = imgid;
	RbtStatus status;
	NodeType *p;
	int ret = 0;

	if ((p = rbtFind(rb,key)) != NULL) {
		*cacheid = ((tex_node_p)(p->val.node))->texid;
		ret = 1;
	} else {
		ValType val;
		val.stuff = imgid;
		val.node  = get_free_node();
		key = imgid;
		status = rbtInsert(rb,key, val);
		if (status)
			printf("fail: status = %d\n", status);
		*cacheid  = ((tex_node_p)(val.node))->texid;
	}

	return ret;
}

int  tex_cache_free(int imgid)
{
	KeyType key = imgid;
	NodeType *p;
	tex_node_p tex_node;

	if ((p = rbtFind(rb,key)) != NULL) {
		tex_node = (tex_node_p)(p->val.node);
		tex_node->next =  free_tex_list;
		free_tex_list = tex_node;
		rbtErase(rb,p);
		return 1;
	}
	return 0;
}

void tex_cache_fini()
{
	if(free_tex_head){
		free(free_tex_head);
		free_tex_head = NULL;
	}
	if(deleted_node){
		free(deleted_node);
		deleted_node = NULL;
	}
	destoryTree(rb);
}
