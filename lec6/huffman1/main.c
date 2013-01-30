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
#include "decode.h"

int main(int argc, char **argv)
{
    int i, flag = 0;
    
    for (i = 0; i < argc; i++)
    {
        if (argc > i + 1)
        {
            if (strcmp(argv[i], "-e") == 0)
            {
                encode(argv[i + 1]);
                flag = 1;
            }
            else if (strcmp(argv[i], "-d") == 0)
            {
                decode(argv[i + 1]);
                flag = 1;
            }
        }
    }
    
    if (!flag)
    {
        printf("Usage:\n");
        printf("\tEncode: -e filename\n");
        printf("\tDecode: -d filename\n");
    }

    return 0;
}