#include <math.h>
#include "func.h"

/* 次元を返す */
int f_dimension()
{
    return 3;
}

/* 関数値を返す */
double f_value(const double x[])
{
    // cos(x) + sin(y) + cos(z)
    return cos(x[0]) + sin(x[1]) + cos(x[2]);
}

/* 勾配を計算する */
void f_gradient(const double x[], double g[])
{
    g[0] = -sin(x[0]);  // ∂f/∂x = -sin(x)
    g[1] =  cos(x[1]);  // ∂f/∂y =  cos(y)
    g[2] = -sin(x[2]);  // ∂f/∂z = -sin(z)
}