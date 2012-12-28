/*
 * 物理シミュレーション
 *  2次元Vector
 * 学籍番号: 03-123006
 * 氏名: 岩成達哉
 */

#include <stdio.h>
#include <math.h>
#include "vector2D.h"

/**
 * 大きさを計算する
 * @param Vector2D v 大きさを計算するベクトル
 * @return double 大きさ
 */
double norm(Vector2D v)
{
    return sqrt(v.x * v.x + v.y * v.y);
}

/**
 * 和を計算する
 * @param Vector2D v1 1つ目のベクトル
 * @param Vector2D v2 2つ目のベクトル
 * @return Vector2D 計算結果
 */
Vector2D add(Vector2D v1, Vector2D v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    
    return v1;
}

/**
 * 差を計算する
 * @param Vector2D v1 引かれるベクトル
 * @param Vector2D v2 引くベクトル
 * @return Vector2D 計算結果
 */
Vector2D sub(Vector2D v1, Vector2D v2)
{
    v1.x -= v2.x;
    v1.y -= v2.y;
    
    return v1;
}

/**
 * 内積を計算する
 * @param Vector2D v1 1つ目のベクトル
 * @param Vector2D v2 2つ目のベクトル
 * @return double 大きさ
 */
double inner_product(Vector2D v1, Vector2D v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

/**
 * 成分を定数倍する
 * @param Vector2D v 成分を定数倍するベクトル
 * @param double k 定数
 * @return Vector2D 定数倍したベクトル
 */
Vector2D scalar_mul(Vector2D v, double k)
{
    v.x *= k;
    v.y *= k;
    
    return v;
}

/**
  * 成分を表示する
  * @param Vector2D v 成分を表示するベクトル
  */
void show_component(Vector2D v)
{
    printf("(%6.2f, %6.2f)", v.x, v.y);
}