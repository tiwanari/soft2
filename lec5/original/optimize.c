#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "optimize.h"

double calc_norm(const int dim, double v[])
{
  double tmp = 0;
  int i = 0;
  for (i = 0; i < dim; i++) {
    tmp += v[i] * v[i];
  }
  tmp = sqrt(tmp);
  return tmp;
}

int optimize(const double alpha, const int dim, double x[], 
	     void (*calc_grad)(const double [], double []))
{
  int i;

  double *g = malloc(dim * sizeof(double));

  int iter = 0;
  while (++iter < 10000) {

    (*calc_grad)(x, g);

    const double norm = calc_norm(dim, g);
    printf("%3d norm = %7.4f", iter, norm);
    for (i = 0; i < dim; i++) {
      printf(", x[%d] = %7.4f", i, x[i]);
    }
    putchar('\n');

    if (norm < 0.01) break;

    for (i = 0; i < dim; i++) {
      x[i] -= alpha * g[i];
    }
  }
  return iter;
}

