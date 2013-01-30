/**
 *  学籍番号: 03-123006
 *  氏名: 岩成達哉
 *      
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include "encode.h"

#define NSYMBOLS 257
typedef unsigned char SYMBOL;    // NSYMBOLSに合わせる(256bit)

//#define DEBUG

static int count[NSYMBOLS] = {0};    // シンボルの出現回数

/* シンボルを表す構造体 */
typedef struct node
{
    SYMBOL symbol;
    int count;
    struct node *left;
    struct node *right;
} Node;

/**
 *  シンボル数をカウントする関数
 *  @param const char *filename 開くファイル名
 */
void count_symbols(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    SYMBOL c;
    
    if (fp == NULL)
    {
        fprintf(stderr, "error: cannot open %s\n", filename);
        exit(1);
    }
    
    // 読みだしてカウント
    while (fread(&c, sizeof(c), 1, fp) != 0)
        count[c]++;
    
    count[NSYMBOLS-1]++; // ファイルの終わり
    
    fclose(fp);
    
#ifdef DEBUG
    int i;
    for (i = 0; i < NSYMBOLS; i++)
        printf("Num of %c : %d\n", i, count[i]);
#endif
}

/**
 *  出現回数が最小のノードを取り出す関数
 *  @param int *n ノードの初期数
 *  @param Node *nodes[] ノードの配列
 *  @return Node* 出現回数が最小のノード
 */
Node* pop_min(int *n, Node *nodes[])
{
    int i, j = 0;
    
    // 出現回数が最小のノードの添字を探す
    for (i = 0; i < *n; i++)
    {
        if (nodes[i]->count < nodes[j]->count)
            j = i;
    }
    
    Node *node_min = nodes[j];  // 出現回数が最小のノード
    
    // 最小のノードを詰める
    for (i = j; i < (*n) - 1; i++)
        nodes[i] = nodes[i + 1];
    
    (*n)--; // 要素数が減ったので減らす
    
    return node_min;    // 出現回数が最小のノードを返す
}

/**
 *  符号の木を作る関数
 *  @return Node* 符号の木の根
 */
Node* build_tree()
{
    int i, n = 0;
    Node *nodes[NSYMBOLS];
    
    // 葉を作成する
    for (i = 0; i < NSYMBOLS; i++)
    {
        if (count[i] == 0) continue;    // 一度も出現していないとき
        nodes[n] = (Node*)malloc(sizeof(Node));
        nodes[n]->symbol = (SYMBOL)i;
        nodes[n]->count  = count[i];
        nodes[n]->left   = NULL;
        nodes[n]->right  = NULL;
        n++;    // 要素数をカウント
    }
    
    // 符号の木を作る
    while (n >= 2)
    {
        Node *node1 = pop_min(&n, nodes);   // 出現回数最小のノード
        Node *node2 = pop_min(&n, nodes);   // 出現回数最小のノード
        
        // 節を作成
        Node *new_node = (Node *)malloc(sizeof(Node));
        new_node->count = node1->count + node2->count;  // 出現回数を合計する
        new_node->left  = node1;
        new_node->right = node2;
        
        // 新しく作ったノードをリストに追加
        nodes[n++] = new_node;
    }
    
    return nodes[0];    // 根を返す
}

/**
 *  木をなぞって符号を当てる関数
 *  @param const int depth 探索の深さ
 *  @param const Node *np 現在のノード
 */
void traverse_tree(const int depth, const Node *np)
{
    static char code[NSYMBOLS] = {0};    // 符号
    
    assert(depth < NSYMBOLS);
    
    if (np->left == NULL)   // 葉にしか要素はない
    {
        // 符号語を表示
        code[depth] = '\0';
        printf("0x%02x -> %s\n", (unsigned int)np->symbol, code);
        return;
    }
    code[depth] = '0';    // 0を詰める
    traverse_tree(depth + 1, np->left);
    code[depth] = '1';    // 1を詰める
    traverse_tree(depth + 1, np->right);
}

/**
 *  エンコード関数
 *  @param const char *filename 開くファイル名
 *  @return int 成功
 */
int encode(const char *filename)
{
    count_symbols(filename);    // シンボル数のカウント
    Node *root = build_tree();  // 符号の木を作る
    traverse_tree(0, root);     // 符号を当てる
    
    return 1;
}
#endif