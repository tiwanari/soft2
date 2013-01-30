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
#include "decode.h"
#include "util.h"

/**
 *  符号の木を作り直す関数
 *  @param const int depth 深さ
 *  @param Node *np 現在のノード
 *  @param const int symbol 挿入したいシンボル
 *  @param const char *code 符号語
 */
void add_tree_t(const int depth, Node *np, const int symbol, const char *code)
{
    // 終わりに来たら
    if (code[depth] == '\0')
    {
        np->symbol = symbol;    // シンボルの設定
        strcpy((char *)(np->code), code);   // 符号語の設定
        return;
    }
    
    // 要素を作りながらたどっていく
    if (code[depth] == '0') // 左へ
    {
        if (np->left == NULL)   // 無いときは作る
        {
            np->left = (Node *)malloc(sizeof(Node));
            (np->left)->left = (np->left)->right = NULL;
        }
        add_tree_t(depth + 1, np->left, symbol, code);
    }
    else if (code[depth] == '1')    // 右へ
    {
        if (np->right == NULL)  // 無いときは作る
        {
            np->right = (Node *)malloc(sizeof(Node));
            (np->right)->left = (np->right)->right = NULL;
        }
        add_tree_t(depth + 1, np->right, symbol, code);
    }
}

/**
 *  符号の木を作り直す関数のラッパー
 *  @param Node *root 木の根
 *  @param const SYMBOL symbol 挿入したいシンボル
 */
void add_tree(Node *root, const int symbol, const char *code)
{
    add_tree_t(0, root, symbol, code);
}

/**
 *  符号の木を作る関数
 *  @param const char *filename 圧縮されたファイル
 *  @param char *outfile マッピングを示したファイル
 *  @return Node* 符号の木の根
 */
Node* read_mapping(const char *filename, char *outfile)
{
    FILE *fp;
    char code[NSYMBOLS];
    int symbol;
    Node *root = (Node *)malloc(sizeof(Node));
    char *mapfile;
    
    root->left = root->right = NULL;    // 根を作って初期化
    
    // マッピングを保存するファイルを作成
    mapfile = change_extention(filename, ".maps");
    if ((fp = fopen(mapfile, "rb")) == NULL)
    {
        fprintf(stderr, "Error: cannot open %s\n", mapfile);
        exit(1);
    }
    free(mapfile);
    
    fscanf(fp, "%s", outfile);  // 1行目は圧縮前のファイル名
    
    // 符号の木を作成
    while (fscanf(fp, "%d %s", &symbol, code) != -1)
        add_tree(root, symbol, code);
    
    fclose(fp);
    
    return root;    // 符号の木の根を返す
}

/**
 *  解凍する関数
 *  @param const Node *root 符号の木の根
 *  @param const char *infile 圧縮されたファイル
 *  @param const char *outfile 解凍後のファイル
 */
void decompress(const Node *root, const char *infile, const char *outfile)
{
    FILE *ifp;
    FILE *ofp;
    int nbit = 0;
    Node *np = (Node *)root;    // 根で初期化
    char c;
    
    // ファイルを開く
    if ((ifp = fopen(infile, "rb")) == NULL
        || (ofp = fopen(outfile, "wb")) == NULL)
    {
        fprintf(stderr, "error: cannot open %s\n", infile);
        exit(1);
    }
    
    // 読みだして変換
    while (fread(&c, sizeof(c), 1, ifp) == 1)
    {
        nbit = 0;   // 現在詰めているbitの位置
        while (nbit < 8)    // cのbit数の限界(8bit)まで探す
        {
            if ((c >> (7 - nbit)) & 1)  // 右へ
                np = np->right;
            else    // 左へ
                np = np->left;
            
            // 葉まで来たらそのシンボルが書き込む値
            if (np->left == NULL)
            {
                if (np->symbol == NSYMBOLS - 1) // EOFなら抜ける
                    break;
                fwrite(&(np->symbol), sizeof(SYMBOL), 1, ofp);  // 書き出し
                np = (Node *)root;  // 根に戻す
            }
            
            nbit++; // bit数をカウントアップ
        }
    }
    
    fclose(ifp);
    fclose(ofp);
}

/**
 *  デコード関数
 *  @param const char *filename 開くファイル名
 *  @return int 成功or失敗
 */
int decode(const char *filename)
{
    char *outfile = (char *)malloc(sizeof(char) * (strlen(filename) + 10));
    Node *root;
    
    if (strstr(filename, ".cmpd") == NULL)
    {
        printf("This file is not a compressed file.\n");
        return 0;
    }
    
    root = read_mapping(filename, outfile); // マッピングを読み出し
    
    decompress(root, filename, outfile);    // 解凍
    free(outfile);
    
    printf("finished\n");
    
    return 1;
}
