/*
 * ライフゲームで周期を持つまでの長さが大きい配置（10セル）を求める
 * 今回はフィールドの一行を変数のbitで表すことでデータを圧縮
 * 学籍番号: 03-123006
 * 氏名: 岩成達哉
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define HEIGHT (70)     // フィールドの高さ
#define WIDTH (50)      // フィールドの幅
#define MAX_ELE (10000) // 保存しておく要素数
#define MAX_CELL (10)   // 初期におけるセルの最大
#define OUTPUT_FILE "cells.txt" // 出力先のファイル名
#define TRYUNI (10000)    // 試行回数

int CENTER_HEIGHT = (HEIGHT / 2) - 1;   // フィールドの高さの中央
int CENTER_WIDTH = (WIDTH / 2) - 1;     // フィールドの幅の中央

typedef unsigned long long ROW;   // WIDTHによって変更する(今回は8bytes = 32bits)

int g_cur = 0;                // 現在の経過ターン
ROW g_cell[MAX_ELE][HEIGHT];  // 記録用

int g_max = 0;                // 最大
ROW g_longest[HEIGHT];        // 最大のパターン

ROW INIT_CELL[HEIGHT] = {0};   // 初期化用

/* 関数のプロトタイプ宣言 START */
void print_cells(ROW cell[]);
double genRand();
double gaussian(double myu, double sgm);
void gen_pattern(double s, int num, ROW pattern[]);
void update_cells();
int check_cycle();
int find_span(double sgm);
/* 関数のプロトタイプ宣言 END */


/**
 * セルの表示をする関数
 * @param ROW cell[] 表示したいフィールド
 */
void print_cells(ROW cell[])
{
    int i, j;
    
    for (i = 0; i < HEIGHT; ++i) {
        for (j = WIDTH - 1; j >= 0 ; --j) {
            char c = ((cell[i] >> j) & 1) ? '#' : '-'; // bit演算
            printf("%c", c);
        }
        printf("\n");
    }
    printf("\n");
    fflush(stdin);  // 書き出させる
    
    //sleep(1);
}

/**
 * 乱数を作る関数
 * @return double 乱数
 */
double genRand()
{
    return rand() / (1.0 + RAND_MAX);
}

/** 
 * 正規分布を返す関数
 * @param double myu 平均
 * @param double sgm 分散
 * @return double 正規分布による値
 */
double gaussian(double myu, double sgm)
{
    double u_0 = genRand();
    double u_1 = genRand();
    double v_0 = myu + sgm * sqrt(-2 * log(1 - u_0)) * cos(2 * M_PI * u_1);
    double v_1 = myu + sgm * sqrt(-2 * log(1 - u_0)) * sin(2 * M_PI * u_1);
    
    return v_0; // 1つ目のデータを返す
}

/**
 * 中心に正規分布でパターンを作る関数
 * @param double s 分散
 * @param int num 配置するセルの数
 * @param ROW pattern[] 初期化するfield
 */
void gen_pattern(double s, int num, ROW pattern[])
{
    int i;
    int x, y;
    double sgm = s;
    
    g_cur = 0;    // 初期化
    memcpy(pattern, INIT_CELL, sizeof(ROW) * HEIGHT);   // 初期化
    
    for (i = 0; i < num; i++) {
        x = (int)gaussian(CENTER_WIDTH, sgm);
        y = (int)gaussian(CENTER_HEIGHT, sgm);
        
        // 境界条件の判定
        if (x < 0) x = 0;
        if (x >= WIDTH) x = WIDTH - 1;
        if (y < 0) y = 0;
        if (y >= HEIGHT) y = HEIGHT - 1;
        
        pattern[y] |= 1 << x;
    }
}


/**
 * 次の状態を作る関数
 */
void update_cells()
{
    ROW cell_next[HEIGHT] = {};
    int i, j, k, l;
    
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            int n = 0;
            for (k = i - 1; k <= i + 1; k++) {
                if (k < 0 || k >= HEIGHT) continue; // フィールド外
                for (l = j - 1; l <= j + 1; l++) {
                    if (k == i && l == j) continue;     // 同じマス
                    if (l < 0 || l >= WIDTH) continue;  // フィールド外
                    n += (g_cell[g_cur][k] >> l) & 1;       // あるなら1が立っている
                }
            }
            
            // 元々の位置にあったかどうかを判定
            if (((g_cell[g_cur][i] >> j) & 1) == 0) {
                if (n == 3)
                    cell_next[i] |= 1 << j;
                else
                    cell_next[i] &= ~(1 << j);
            }
            else {
                if (n == 2 || n == 3)
                    cell_next[i] |= 1 << j;
                else
                    cell_next[i] &= ~(1 << j);
            }
        }
    }
    // 結果を格納
    memcpy(g_cell[++g_cur], cell_next, sizeof(ROW) * HEIGHT);
}

/**
 * 周期性のチェックをする関数
 */
int check_cycle()
{
    int i, j;
    
    // 現在までのすべてでチェック
    for (i = 0; i < g_cur; i++) {
        if (i == MAX_ELE || memcmp(g_cell[i], g_cell[g_cur], sizeof(ROW) * HEIGHT) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * 周期性を持つまでの時間を求める関数
 */
int find_span(double sgm)
{
    FILE *fp;
    int res;
    
    // 周期を持つまでチェック
    do {
        update_cells();
    } while ( (res = check_cycle()) == -1 );
    
    // 最大数が更新された時
    if (g_max < res)
    {
        // 結果の表示
        g_max = res;
        memcpy(g_longest, g_cell[(g_cur = 0)], sizeof(ROW) * HEIGHT);
        printf("--- Updated! Sgm = %f Span = %d ---\n", sgm, g_max);
        print_cells(g_longest);
        printf("\n");
        
        // ファイルへ書き込み(バイナリ)
        if ((fp = fopen(OUTPUT_FILE, "wb")) == NULL) {
            printf("File Open Error\n");
            exit(1);
        }
        fwrite(&g_max, sizeof(g_max), 1, fp);
        fwrite(g_longest, sizeof(g_longest), 1, fp);
        fclose(fp);
    }
    // 周期を持つまでの値を返す
    return res;
}

int main()
{
    FILE *fp;
    double i;   // 分散
    double sum, ave; // 分散ごとの周期の大きさの平均を見る
    int max, tmp;
    int j, k;
    
    // これまでの最大を読み込む
    if ((fp = fopen(OUTPUT_FILE, "rb")) == NULL) {
        printf("File Open Error\n");
        return 1;
    }
    else {
        fread(&g_max, sizeof(g_max), 1, fp);
        fread(g_longest, sizeof(g_longest), 1, fp);
        fclose(fp);
        // これまでの最大を表示
        printf("init max = %d, \n", g_max);
        print_cells(g_longest);
    }

    srand((unsigned int)time(NULL));    // 乱数列の初期化
    
    // 試行
    for (i = 1.000; i < 2.100; i += 0.100) {
        printf("Sgm = %f\n", i);
        fflush(stdin);
        ave = 0.0; max = 0;
        for (j = 0; j < TRYUNI; j++) {
            sum = 0.0;
            for (k = 5; k < MAX_CELL; k++) {
                gen_pattern(i, k, g_cell[0]);
                tmp = find_span(i);
                sum += tmp / (MAX_CELL - 5);
                if (max < tmp) max = tmp;
            }
            ave += sum;
        }
        printf("    Ave Span = %f, Max Span = %d\n", ave /= TRYUNI, max);
        fflush(stdin);
    }

    return 0;
}
