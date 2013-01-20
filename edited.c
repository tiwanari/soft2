#include <stdio.h>

typedef struct _node {
    int key;
    struct _node *parent;
    struct _node *left;
    struct _node *right;
#define AVL_BALANCED   0
#define AVL_LEFT       1
#define AVL_RIGHT      2
    int balance;
} NODE;

static NODE *root;

NODE *search(int key)
{
    NODE *node = root;

    node = root->left;
    while (node != NULL) {
        if (key == node->key) return node;
        if (key < node->key) node = node->left;
        else                 node = node->right;
    }
    return NULL;
}

void rebalance(NODE *node, int direct)
{
    NODE *prev, *next;

    if (node == NULL) return;
    if (node->balance == AVL_LEFT  && direct == AVL_RIGHT ||
        node->balance == AVL_RIGHT && direct == AVL_LEFT) {
        node->balance = AVL_BALANCED;
    } else if (node->balance == AVL_BALANCED) {
        node->balance = direct;
        if ((prev = node->parent) != NULL)
            rebalance(prev, node->key < prev->key ? 
                      AVL_LEFT : AVL_RIGHT);
    } else /* if (node->balance == direct) */ {
        prev = node->parent;
        if (node->left->balance == direct) {
            next = (direct == AVL_LEFT) ? node->left : node->right;

            node->parent = next;
            next->parent = prev;
            if (direct == AVL_LEFT) {
                node->left  = next->right;
                next->right = node;
            } else {
                node->right = next->left;
                next->left  = node;
            }
            node->balance = AVL_BALANCED;
            next->balance = AVL_BALANCED;
        } else {
            NODE *dene;
            if (direct == AVL_LEFT) {
                dene = node->left;
                next = dene->right;
            } else {
                dene = node->right;
                next = dene->left;
            }

            node->parent = next;
            dene->parent = next;
            next->parent = prev;
            if (direct == AVL_LEFT) {
                node->left  = next->right;
                next->right = node;
                dene->right = next->left;
                next->left  = dene;
                dene->balance = (next->balance == AVL_RIGHT) ?
                                AVL_LEFT : AVL_BALANCED;
                node->balance = (next->balance == AVL_LEFT) ?
                                AVL_RIGHT : AVL_BALANCED;
            } else {
                node->right = next->left;
                next->left  = node;
                dene->left  = next->right;
                next->right = dene;
                dene->balance = (next->balance == AVL_LEFT) ?
                                AVL_RIGHT : AVL_BALANCED;
                node->balance = (next->balance == AVL_RIGHT) ?
                                AVL_LEFT : AVL_BALANCED;
            }
            next->balance = AVL_BALANCED;
        }
        if (prev->parent == NULL)
            prev->left = prev->right = next;
        else if (node->key < prev->key) prev->left  = next;
        else                            prev->right = next;
    }
}

NODE *insert(int key)
{
    NODE **node = &root;
    NODE *prev  = NULL;

    while (*node != NULL) {
        if (key == (*node)->key) return NULL;
        prev = *node;
        if (key < (*node)->key) node = &((*node)->left);
        else                    node = &((*node)->right);
    }
    if ((*node = (NODE *)malloc(sizeof(NODE))) == NULL) return NULL;
    (*node)->key  = key;
    (*node)->parent = prev;
    (*node)->left = (*node)->right = NULL;
    (*node)->balance = 0;
    if (prev != NULL)
        rebalance(prev, key < prev->key ? AVL_LEFT : AVL_RIGHT);
    return (*node);
}

int delete(int key)
{
    NODE *prev, *node;

    prev = root;
    node = root->left;
    while (node != NULL) {
        if (key == node->key) {
            if      (node->left  == NULL) node = node->right;
            else if (node->right == NULL) node = node->left;
            else {
                NODE *last = node;
                NODE *next = last->right;
                while (next->left != NULL) {
                    last = next;
                    next = next->left;
                }
                last->left = next->right;
                next->left = node->left;
                next->right = node->right;
                node = next;
            }
            if (key < prev->key) prev->left  = node;
            else                 prev->right = node;
            free(node);
            return 0;
        }
        prev = node;
        if (key < node->key) node = node->left;
        else                 node = node->right;
    }
    return -1;
}
