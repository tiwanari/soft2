/*
 * 物理シミュレーション
 * Vector用ヘッダファイル
 * 学籍番号: 03-123006
 * 氏名: 岩成達哉
 */

#ifndef _VECTOR_H_
#define _VECTOR_H_

/* 2次元ベクトル */
typedef struct _vector
{
    double x;
    double y;
} Vector2D;

/**
 * 大きさを計算する
 * @param const Vector2D v 大きさを計算するベクトル
 * @return double 大きさ
 */
double norm(const Vector2D v);

/**
 * 和を計算する
 * @param const Vector2D v1 1つ目のベクトル
 * @param const Vector2D v2 2つ目のベクトル
 * @return Vector2D 計算結果
 */
Vector2D add(const Vector2D v1, const Vector2D v2);


/**
 * 差を計算する
 * @param const Vector2D v1 引かれるベクトル
 * @param const Vector2D v2 引くベクトル
 * @return Vector2D 計算結果
 */
Vector2D sub(const Vector2D v1, const Vector2D v2);


/**
 * 内積を計算する
 * @param const Vector2D v1 1つ目のベクトル
 * @param const Vector2D v2 2つ目のベクトル
 * @return double 大きさ
 */
double inner_product(const Vector2D v1, const Vector2D v2);

/**
 * 成分を定数倍する
 * @param const Vector2D v 成分を定数倍するベクトル
 * @param double k 定数
 * @return Vector2D 定数倍したベクトル
 */
Vector2D scalar_mul(const Vector2D v, double k);

/**
 * 成分を表示する
 * @param const Vector2D v 成分を表示するベクトル
 */
void show_component(const Vector2D v);
#endif