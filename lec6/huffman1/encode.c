/**
 *  学籍番号: 03-123006
 *  氏名: 岩成達哉
 *      課題4: 圧縮ツール
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include "encode.h"
#include "util.h"

static int count[NSYMBOLS] = {0};    // シンボルの出現回数

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
        nodes[n]->symbol = i;
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
void traverse_tree(const int depth, const Node *np, FILE *fp)
{
    static char code[NSYMBOLS] = {0};    // 符号
    
    assert(depth < NSYMBOLS);
    
    if (np->left == NULL)   // 葉にしか要素はない
    {
        // 符号語を表示
        code[depth] = '\0';
        strcpy((char *)(np->code), code);   // コードを保存
  //      if (np->symbol != NSYMBOLS - 1)     // EOFは登録しない
            fprintf(fp, "%d %s\n", np->symbol, code);
#ifdef DEBUG
        printf("0x%02x -> %s\n", np->symbol, code);
#endif
        return;
    }
    code[depth] = '0';    // 0を詰める
    traverse_tree(depth + 1, np->left, fp);
    code[depth] = '1';    // 1を詰める
    traverse_tree(depth + 1, np->right, fp);
}

/**
 *  シンボルの等号判定
 *  @param const SYMBOL symbol1 比較対象1
 *  @param const SYMBOL symbol2 比較対象2
 *  @return int 真or偽
 */
int is_equal(const SYMBOL symbol1, const SYMBOL symbol2)
{
    return (symbol1 == symbol2);
}

/**
 *  シンボルを木から探す関数
 *  @param char **code 符号を返すための格納先
 *  @param const int symbol 探す対象
 *  @param const Node *np 現在のノード
 *  @return int 見つかったとき
 */
int search_tree_t(char **code, const int symbol, const Node *np)
{
    if (np->left == NULL)   // 葉にしか要素はない
    {
        if (is_equal(np->symbol, symbol))   // シンボルと一致するか
        {
            *code = (char *)np->code;   // 一致するなら格納
            return 1;
        }
#ifdef DEBUG
        printf("0x%02x -> %s\n", (unsigned int)np->symbol, np->code);
#endif
        return 0;
    }
    // 左へ
    if (search_tree_t(code, symbol, np->left))  // 見つかったら終了
        return 1;
    
    // 右へ
    if (search_tree_t(code, symbol, np->right)) // 見つかったら終了
        return 1;
    
    return 0;
}

/**
 *  シンボルを木から探す関数のラッパー
 *  @param const int symbol 探す対象
 *  @param const Node *root 木の根
 *  @return char* 符号語
 */
char* search_tree(const int symbol, const Node *root)
{
    char *code;
    
    search_tree_t(&code, symbol, root);
    
    return code;
}

/**
 *  実際に圧縮ファイルを作る関数
 *
 **/
void compress(const Node *root, const char *infile)
{
    FILE *ifp;
    FILE *ofp;
    char *p;
    char *outfile;
    SYMBOL c;
    char writeBits = 0;
    int nbit = 0;
    int flag = 0;
    
    // 出力ファイル名の作成
    outfile = change_extention(infile, ".cmpd");
    if ((ifp = fopen(infile, "rb")) == NULL
        || (ofp = fopen(outfile, "wb")) == NULL)
    {
        fprintf(stderr, "error: cannot open %s\n", infile);
        exit(1);
    }
    free(outfile);
    
    // 読みだして変換
    while (1)
    {
        int i;

        if ((flag = fread(&c, sizeof(c), 1, ifp)) == 1)
            p = search_tree(c, root);   // シンボルに対応する符号語を抽出
        else
            p = search_tree(NSYMBOLS - 1, root);   // EOFに対応する符号語を抽出
            
        for (i = 0; p[i] != '\0'; i++)
        {
            writeBits = (writeBits << 1) + (p[i] - '0');
            if (++nbit == 8)
            {
                fwrite(&writeBits, sizeof(char), 1, ofp);
                nbit = writeBits = 0;
            }
        }
        
        // EOFを書き込んだら抜ける
        if (!flag)
            break;
    }

    // 1byteに満たないなら書き込んでしまう
    if (nbit != 0)
    {
        while (nbit != 8)
        {
            writeBits = writeBits << 1; // 0を詰める
            nbit++;
        }
        fwrite(&writeBits, sizeof(char), 1, ofp);
    }
    
    fclose(ifp);
    fclose(ofp);
}

/**
 *  エンコード関数
 *  @param const char *filename 開くファイル名
 *  @return int 成功
 */
int encode(const char *filename)
{
    FILE *fp;
    char *mapfile;
    
    count_symbols(filename);    // シンボル数のカウント
    Node *root = build_tree();  // 符号の木を作る
    
    mapfile = change_extention(filename, ".maps");
    if ((fp = fopen(mapfile, "w")) == NULL)
    {
        fprintf(stderr, "Cannot Open Mapping File\n");
        exit(1);
    }
    free(mapfile);
    fprintf(fp, "%s\n", filename);  // ファイル名を入れておく
    traverse_tree(0, root, fp);     // 符号を当てる
    fclose(fp);
    
    compress(root, filename);   // 圧縮
    
    printf("finished\n");
    return 1;
}
