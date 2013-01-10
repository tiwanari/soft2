/**
 * ファイルから初期配置パターンを入力する
 * 学籍番号: 03-123006
 * 氏名: 岩成達哉
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define HEIGHT (50)
#define WIDTH (70)

#define INPUT_FILE "init.txt"   // 入力用ファイル
#define OUTPUT_FILE "cells.txt" // 出力用ファイル

#define BUFSIZE (1000)

int cell[HEIGHT][WIDTH];        // フィールド
int cell_next[HEIGHT][WIDTH];   // 次のフィールド

/* フィールドをファイルへ書き出す関数 */
void print_cells(FILE *fp)
{
    int i, j;
    
    fprintf(fp, "----------\n");
    
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            char c = (cell[i][j] == 1) ? '#' : ' ';
            fputc(c, fp);
        }
        fputc('\n', fp);
    }
    fflush(fp);
    
    sleep(1);
}

/* フィールドの初期化をする関数 */
int init_cells()
{
    int i, j;
    
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            cell[i][j] = rand() % 2;
        }
    }
}

/* フィールドをファイルによって初期化をする関数 */
void init_cells_by_file(char* filename)
{
    char buf[BUFSIZE];
    FILE *fp;
    int i, j;
    
    if ((fp = fopen(filename, "r")) == NULL) {
        printf("error: can't open %s\n", filename);
        exit(1);
    }
    
    for (i = 0; i < HEIGHT && fgets(buf, BUFSIZE, fp) != NULL; i++) {
        size_t len = strlen(buf) - 1;
        for (j = 0; j < WIDTH && j < len; j++) {
            cell[i][j] = (buf[j] == '#') ? 1 : 0;
        }
    }
    fclose(fp);
}

/* フィールドの更新 */
void update_cells()
{
    int i, j, k, l;
    
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            int n = 0;
            for (k = i - 1; k <= i + 1; k++) {
                if (k < 0 || k >= HEIGHT) continue;     // フィールド外
                for (l = j - 1; l <= j + 1; l++) {
                    if (k == i && l == j) continue;     // そのセル自身
                    if (l < 0 || l >= WIDTH) continue;  // フィールド外
                    n += cell[k][l];    // 8近傍セル数のカウント
                }
            }
            // 新しくセルを作成するかどうか
            if (cell[i][j] == 0) {
                if (n == 3)
                    cell_next[i][j] = 1;
                else
                    cell_next[i][j] = 0;
            } else {
                if (n == 2 || n == 3)
                    cell_next[i][j] = 1;
                else
                    cell_next[i][j] = 0;
            }
        }
    }
    
    // 代入
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            cell[i][j] = cell_next[i][j];
        }
    }
}


int main()
{
    FILE *fp;
    char filename[BUFSIZE];
    int gen = 1;
    
    // 書き出し用ファイルを開く
    if ((fp = fopen(OUTPUT_FILE, "w")) == NULL) {
        fprintf(stderr, "error: cannot open a file.\n");
        return 1;
    }
    
    printf("Init File Name: ");
    fgets(filename, BUFSIZE, stdin);
    filename[strlen(filename) - 1] = '\0'; // 改行を消す
    
    //init_cells();
    
    init_cells_by_file(filename);   // ファイルから初期配置パターンを決める
    print_cells(fp);
    
    // ずっと繰り返す
    while (1) {
        printf("gen = %d\n", gen++);
        update_cells();
        print_cells(fp);
    }
    
    fclose(fp);
}
