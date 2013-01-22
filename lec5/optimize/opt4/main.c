/**
 *  学籍番号: 03-1230006
 *  氏名: 岩成達哉
 *      課題4: より早く収束するアルゴリズムを実装
 *          準ニュートン法に黄金分割法を組み込んで実装した
 *          初期値は ./a.out x0 x1 ... と入力する(デフォルトは0.0)
 */

#include <stdio.h>
#include <stdlib.h>
#include "optimize.h"
#include "func.h"

int main(int argc, char **argv)
{
    int i;
    const int dim = f_dimension();
    
    double *x = malloc(dim * sizeof(double));
    for (i = 0; i < dim; i++)
    {
        if (argc > i+1)
            x[i] = atof(argv[i+1]);
        else
            x[i] = 0;
    }
    
    f_print();  // 関数の表示
    
    const int iter = optimize(f_dimension(), x, f_gradient, f_quasi_hessian, f_value);
    
    printf("number of iterations = %d\n", iter);
    
    return 0;
}