/**
 *  学籍番号: 03-1230006
 *  氏名: 岩成達哉
 *      課題4: より早く収束するアルゴリズムを実装
 *          準ニュートン法に黄金分割法を組み込んで実装した
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
    x[i] = 0;
  

  const int iter = optimize(f_dimension(), x, f_gradient, f_quasi_hessian, f_value);

  printf("number of iterations = %d\n", iter);

  return 0;
}