/**
 *  学籍番号: 03-123006
 *  氏名: 岩成達哉
 *      課題1: 最適化の終了条件を厳しくする
 *          l.49での最適化の精度をl.14の定数によって指定しており，条件が0.01から0.0001となっている
 *          さらに，条件が厳しくなる分，l.38の試行回数をl.15の定数のように10000から1000000としている
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "optimize.h"

#define EPSILON 0.0001  // 誤差の閾値
#define TRY_NUM 1000000 // 試行回数の限界

/* ベクトルの大きさを計算する */
double calc_norm(const int dim, double v[])
{
    double tmp = 0;
    int i = 0;
    for (i = 0; i < dim; i++)
        tmp += v[i] * v[i];
    
    tmp = sqrt(tmp);
    return tmp;
}

/* 最適化問題を解く */
int optimize(const double alpha, const int dim, double x[],
             void (*calc_grad)(const double [], double [])) // 関数の値を得る
{
    int i;
    
    double *g = malloc(dim * sizeof(double));   // 次元数だけ確保
    
    int iter = 0;
    while (++iter < TRY_NUM)    // 条件を厳しくするために回数を増やす
    {
        
        (*calc_grad)(x, g); // 勾配の計算
        
        const double norm = calc_norm(dim, g);  // 大きさを計算
        
        printf("%3d norm = %7.4f", iter, norm);
        for (i = 0; i < dim; i++)
            printf(", x[%d] = %7.4f", i, x[i]);
        putchar('\n');
        
        if (norm < EPSILON) break; // 0.01をより厳しく
        
        for (i = 0; i < dim; i++)
            x[i] -= alpha * g[i];
    }
    
    free(g);
    
    return iter;
}