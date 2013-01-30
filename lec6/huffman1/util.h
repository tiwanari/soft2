/**
 *  学籍番号: 03-123006
 *  氏名: 岩成達哉
 *      課題4: 圧縮ツール
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#define NSYMBOLS 257
typedef unsigned char SYMBOL;    // NSYMBOLSに合わせる(256bit)

//#define DEBUG

/* シンボルを表す構造体 */
typedef struct node
{
    int symbol;
    char code[NSYMBOLS];
    int count;
    struct node *left;
    struct node *right;
} Node;

/**
 *  拡張子を変更したファイル名を返す
 *  @param const char *infile 拡張子を変更したいファイル
 *  @param const char *extention 拡張子
 *  @return char* 変更したファイル名
 */
char* change_extention(const char *infile, const char *extention);

#endif