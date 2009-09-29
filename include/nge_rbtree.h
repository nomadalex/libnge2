#ifndef NGE_RBTREE_H_
#define NGE_RBTREE_H_
// red-black tree
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//////////////////////
// supplied by user //
//////////////////////

typedef int KeyType;            // type of key

typedef struct {                // value related to key
    int stuff;
	void* node;
} ValType;

// how to compare keys
#define compLT(a,b) (a < b)
#define compEQ(a,b) (a == b)

/////////////////////////////////////
// implementation independent code //
/////////////////////////////////////

typedef enum {
		RBT_STATUS_OK,
		RBT_STATUS_MEM_EXHAUSTED,
		RBT_STATUS_DUPLICATE_KEY,
		RBT_STATUS_KEY_NOT_FOUND
} RbtStatus;

typedef enum { BLACK, RED } nodeColor;

typedef struct NodeTag {
    struct NodeTag *left;       // left child
    struct NodeTag *right;      // right child
    struct NodeTag *parent;     // parent
    nodeColor color;            // node color (BLACK, RED)
    KeyType key;                // key used for searching
    ValType val;                // data related to key
} NodeType;

typedef struct{
	NodeType* root;
	NodeType  sentinel;
}rbtree_t,*rbtree_p;


typedef void (*travelcall)(rbtree_p rb,NodeType *);


#ifdef __cplusplus
extern "C" {
#endif
	RbtStatus rbtInsert(rbtree_p rb,KeyType key, ValType val);
	RbtStatus rbtErase(rbtree_p rb,NodeType * z);
	NodeType* rbtFind(rbtree_p rb,KeyType key);
	void rbtInorder(rbtree_p rb,NodeType *p, travelcall callback);
	void rbtDelete(rbtree_p rb,NodeType *p);
	rbtree_p createTree();
	void destoryTree(rbtree_p rb);
#ifdef __cplusplus
}
#endif



#endif
