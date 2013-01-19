#ifndef _OPTIMIZE_H_
#define _OPTIMIZE_H_

int optimize(const double, const int, double [],
             void (*calc_grad)(const double [], double []),
             double (*get_value)(const double []));

#endif
