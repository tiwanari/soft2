/**
 *  学籍番号: 03-1230006
 *  氏名: 岩成達哉
 *      課題4: より早く収束するアルゴリズムを実装
 *          準ニュートン法に黄金分割法を組み込んで実装した
 */

#ifndef _OPTIMIZE_H_
#define _OPTIMIZE_H_

int optimize(const int, double [],
             void (*calc_grad)(const double [], double []),
             void (*calc_hessian)(const double [], const double [], double []),
             double (*get_value)(const double []));

#endif
