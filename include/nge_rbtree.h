#ifndef NGE_RBTREE_H_
#define NGE_RBTREE_H_
// red-black tree

#include "nge_common.h"

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
		RBT_STATUS_OK, /**< 成功*/
		RBT_STATUS_MEM_EXHAUSTED,/**< 空间不足*/
		RBT_STATUS_DUPLICATE_KEY,/**< 索引重复*/
		RBT_STATUS_KEY_NOT_FOUND/**< 索引无法找到*/
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

/**
 *向一棵红黑树中插入一个key, val对
 *@see RbtStatus
 *@param[in, out] rb 要插入的红黑树
 *@param[in] key 标识值的索引
 *@param[in] val 要插入的值
 *@return RbtStatus,操作成功即为RBT_STATUS_OK
 */
	NGE_API RbtStatus rbtInsert(rbtree_p rb,KeyType key, ValType val);

/**
 *从一棵红黑树中删除一个节点
 *@see RbtStatus
 *@param[in, out] rb 要删除的红黑树
 *@param[in] z 待删除节点
 *@return RbtStatus,操作成功即为RBT_STATUS_OK，否则请参考RbtStatus定义
 */
	NGE_API RbtStatus rbtErase(rbtree_p rb,NodeType * z);

/**
 *从一棵红黑树中找到一个索引对应的节点
 *@param[in] rb 要搜索的红黑树
 *@param[in] key 待搜索索引
 *@return NodeType*,搜索到的节点,若搜索失败,返回NULL
 */
	NGE_API NodeType* rbtFind(rbtree_p rb,KeyType key);

/**
 *从一棵红黑树中某个节点开始进行先序遍历
 *@param[in, out] rb 要遍历的红黑树
 *@param[in] p 要开始的节点
 *@param[in] callback 回调函数,对每一个节点进行调用
 */
	NGE_API void rbtInorder(rbtree_p rb,NodeType *p, travelcall callback);

/**
 *从一棵红黑树中删除某个节点的整个子树
 *@param[in, out] rb 要删除的红黑树
 *@param[in] p 删除的根节点
 */
	NGE_API void rbtDelete(rbtree_p rb,NodeType *p);

/**
 *创建一棵红黑树
 *@return rbtree_p,创建的红黑树
 */
	NGE_API rbtree_p createTree();

/**
 *销毁一棵红黑树
 *@param[in, out] rb 要销毁的红黑树
*/
	NGE_API void destoryTree(rbtree_p rb);
#ifdef __cplusplus
}
#endif

#endif
