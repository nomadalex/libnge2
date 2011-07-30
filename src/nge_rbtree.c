#include "nge_debug_log.h"
#include "nge_rbtree.h"

#define SENTINEL &sentinel      // all leafs are sentinels

//static NodeType sentinel = { SENTINEL, SENTINEL, 0, BLACK, 0};

//static NodeType *root = SENTINEL; // root of red-black tree

/*
NodeType* rbtRoot()
{
	return root;
}
*/

rbtree_p createTree()
{
	rbtree_p ret = (rbtree_p)malloc(sizeof(rbtree_t));
	memset(ret,0,sizeof(rbtree_t));
	ret->sentinel.left  = &ret->sentinel;
	ret->sentinel.right = &ret->sentinel;
	ret->sentinel.parent = 0;
	ret->sentinel.color  = BLACK;
	ret->root = &ret->sentinel;
	return ret;
}


static void rotateLeft(rbtree_p rb,NodeType *x) {

    // rotate node x to left

    NodeType *y = x->right;

    // establish x->right link
    x->right = y->left;
    if (y->left != &rb->sentinel /*SENTINEL*/) y->left->parent = x;

    // establish y->parent link
    if (y != &rb->sentinel/**SENTINEL*/) y->parent = x->parent;
    if (x->parent) {
        if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
    } else {
        //root = y;
		rb->root = y;
    }

    // link x and y
    y->left = x;
    if (x != &rb->sentinel) x->parent = y;
}

static void rotateRight(rbtree_p rb,NodeType *x) {

    // rotate node x to right

    NodeType *y = x->left;

    // establish x->left link
    x->left = y->right;
    if (y->right != &rb->sentinel) y->right->parent = x;

    // establish y->parent link
    if (y != &rb->sentinel) y->parent = x->parent;
    if (x->parent) {
        if (x == x->parent->right)
            x->parent->right = y;
        else
            x->parent->left = y;
    } else {
        rb->root = y;
    }

    // link x and y
    y->right = x;
    if (x != &rb->sentinel) x->parent = y;
}

static void insertFixup(rbtree_p rb,NodeType *x) {

    // maintain red-black tree balance
    // after inserting node x

    // check red-black properties
    while (x != rb->root && x->parent->color == RED) {
        // we have a violation
        if (x->parent == x->parent->parent->left) {
            NodeType *y = x->parent->parent->right;
            if (y->color == RED) {

                // uncle is RED
                x->parent->color = BLACK;
                y->color = BLACK;
                x->parent->parent->color = RED;
                x = x->parent->parent;
            } else {

                // uncle is BLACK
                if (x == x->parent->right) {
                    // make x a left child
                    x = x->parent;
                    rotateLeft(rb,x);
                }

                // recolor and rotate
                x->parent->color = BLACK;
                x->parent->parent->color = RED;
                rotateRight(rb,x->parent->parent);
            }
        } else {

            // mirror image of above code
            NodeType *y = x->parent->parent->left;
            if (y->color == RED) {

                // uncle is RED
                x->parent->color = BLACK;
                y->color = BLACK;
                x->parent->parent->color = RED;
                x = x->parent->parent;
            } else {

                // uncle is BLACK
                if (x == x->parent->left) {
                    x = x->parent;
                    rotateRight(rb,x);
                }
                x->parent->color = BLACK;
                x->parent->parent->color = RED;
                rotateLeft(rb,x->parent->parent);
            }
        }
    }
    rb->root->color = BLACK;
}

// insert new node (no duplicates allowed)
RbtStatus rbtInsert(rbtree_p rb,KeyType key, ValType val) {
    NodeType *current, *parent, *x;

    // allocate node for data and insert in tree

    // find future parent
    current = rb->root;
    parent = 0;
    while (current != &rb->sentinel) {
        if (compEQ(key, current->key)) 
            return RBT_STATUS_DUPLICATE_KEY;
        parent = current;
        current = compLT(key, current->key) ?
            current->left : current->right;
    }

    // setup new node
    if ((x = (NodeType*)malloc (sizeof(*x))) == 0)
        return RBT_STATUS_MEM_EXHAUSTED;
    x->parent = parent;
    x->left = &rb->sentinel;
    x->right = &rb->sentinel;
    x->color = RED;
    x->key = key;
    x->val = val;

    // insert node in tree
    if(parent) {
        if(compLT(key, parent->key))
            parent->left = x;
        else
            parent->right = x;
    } else {
        rb->root = x;
    }

    insertFixup(rb,x);

    return RBT_STATUS_OK;
}

static void deleteFixup(rbtree_p rb,NodeType *x) {

    // maintain red-black tree balance
    // after deleting node x

    while (x != rb->root && x->color == BLACK) {
        if (x == x->parent->left) {
            NodeType *w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotateLeft (rb,x->parent);
                w = x->parent->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rotateRight (rb,w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rotateLeft (rb,x->parent);
                x = rb->root;
            }
        } else {
            NodeType *w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotateRight (rb,x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    rotateLeft (rb,w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rotateRight (rb,x->parent);
                x = rb->root;
            }
        }
    }
    x->color = BLACK;
}

// delete node
RbtStatus rbtErase(rbtree_p rb,NodeType * z) {
    NodeType *x, *y;

    if (z->left == &rb->sentinel || z->right == &rb->sentinel) {
        // y has a SENTINEL node as a child
        y = z;
    } else {
        // find tree successor with a SENTINEL node as a child
        y = z->right;
        while (y->left != &rb->sentinel) y = y->left;
    }

    // x is y's only child
    if (y->left != &rb->sentinel)
        x = y->left;
    else
        x = y->right;

    // remove y from the parent chain
    x->parent = y->parent;
    if (y->parent)
        if (y == y->parent->left)
            y->parent->left = x;
        else
            y->parent->right = x;
    else
        rb->root = x;

    if (y != z) {
        z->key = y->key;
        z->val = y->val;
    }


    if (y->color == BLACK)
        deleteFixup (rb,x);

    free (y);

    return RBT_STATUS_OK;
}

// find key
NodeType *rbtFind(rbtree_p rb,KeyType key) {
    NodeType *current;
    current = rb->root;
    while(current != &rb->sentinel) {
        if(compEQ(key, current->key)) {
            return current;
        } else {
            current = compLT (key, current->key) ?
                current->left : current->right;
        }
    }
    return NULL;
}

// in-order walk of tree
void rbtInorder(rbtree_p rb,NodeType *p, travelcall callback) {
    if (p == &rb->sentinel) return;
    rbtInorder(rb,p->left, callback);
    callback(rb,p);
    rbtInorder(rb,p->right, callback);
}

// delete nodes depth-first
void rbtDelete(rbtree_p rb,NodeType *p) {
    if (p == &rb->sentinel) return;
    rbtDelete(rb,p->left);
    rbtDelete(rb,p->right);
    free(p);
}

void destoryTree(rbtree_p rb)
{
	rbtDelete(rb,rb->root);
	if(rb){
		free(rb);
		rb = NULL;
	}
}
