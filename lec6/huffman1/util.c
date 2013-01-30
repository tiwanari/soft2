/**
 *  学籍番号: 03-123006
 *  氏名: 岩成達哉
 *      課題4: 圧縮ツール
 */

#include <string.h>
#include <stdlib.h>
#include "util.h"

char* change_extention(const char *infile, const char *extention)
{
    char *outfile = (char *)malloc(sizeof(char) * (strlen(infile) + strlen(extention) + 1));
    char *p;
    
    // 出力ファイル名の作成
    strcpy(outfile, infile);   // コピー
    if ((p = strrchr(outfile, '.')) != NULL) // 拡張子以下を消す
        *p = '\0';
    strcat(outfile, extention);    // 新しく拡張子をつける
    
    return outfile;
}