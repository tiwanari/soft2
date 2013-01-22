#include <math.h>
#include "func.h"

int f_dimension()
{
  return 2;
}

double f_value(const double x[])
{
  return (x[0] - 3) * (x[0] - 3)  * (x[0] - 3)  * (x[0] - 3) + (x[1] - 2) * (x[1] - 2);
}

void f_gradient(const double x[], double g[])
{
  g[0] = 4 * (x[0] - 3) * (x[0] - 3) * (x[0] - 3);
  g[1] = 2 * (x[1] - 2);
}

