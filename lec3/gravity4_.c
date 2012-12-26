/*
 * 物理シミュレーション
 *  課題4: 精度を上げる
 * 学籍番号: 03-123006
 * 氏名: 岩成達哉
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vector2D.h"

#define WIDTH 75    // 幅
#define HEIGHT 50   // 高さ
#define EPSILON 1   // 融合する距離の基準
#define DELETED -100.0  // 消去されたかどうか

const double G = 1.0;   // 万有引力定数

/* 星 */
typedef struct _star
{
    double m;
    double r;
    Vector2D p;
    Vector2D v;
} Star;

// 2つおく
Star stars[] = {
    { 1.0, 0.5, {-10.0, 0.0}, {0.0, 0.0} },
    { 1.0, 1.0, { 10.0, 0.0}, {0.2, 0.0} },
    /*
    { 1.0, {-5.0, 0.0}, {0.0,  0.10}, 1.0 },
    { 1.0, { 5.0, 0.0}, {0.0, -0.10}, 1.0 },
     */
};

// 星の数
int nstars = sizeof(stars) / sizeof(Star);

/* 星の表示をする関数 */
void plot_stars(FILE *fp, double t)
{
    int i;
    char space[WIDTH][HEIGHT];    // 表示用
    
    memset(space, ' ', sizeof(space));    // 初期化
    // 星を表示
    for (i = 0; i < nstars; i++) {
        if (stars[i].m < 0) continue;   // 消去済み
        const int x = WIDTH  / 2 + stars[i].p.x;
        const int y = HEIGHT / 2 + stars[i].p.y;
        if (x < 0 || x >= WIDTH)  continue;
        if (y < 0 || y >= HEIGHT) continue;
        char c = 'o';
        if (stars[i].r >= 1.0) c = 'O'; // 質量が大きい星
        space[x][y] = c;
    }
    
    // 表示
    int x, y;
    fprintf(fp, "----------\n");
    for (y = 0; y < HEIGHT; y++) {
        for (x = 0; x < WIDTH; x++)
            fputc(space[x][y], fp);
        fputc('\n', fp);
    }
    fflush(fp); // ファイルへ書き出す
    
    printf("t = %5.1f", t);
    for (i = 0; i < nstars; i++)
        printf(", [%d]:stars.x = %5.2f, stars.y = %5.2f", i, stars[i].p.x, stars[i].p.y);
    printf("\n");
    
    usleep(100 * 1000);
}

/* 衝突の判定をする関数 */
void check_collision(void)
{
    int i, j;
    
    for (i = 0; i < nstars; i++) {
        if (stars[i].m < 0) continue;   // 消去済み
        for (j = 0; j < nstars; j++) {
            if (i == j || stars[j].m < 0) continue; // 消去済み
            const Vector2D dp = sub(stars[i].p, stars[j].p);
            const double r = norm(dp);
            
            // 星の衝突
            if (r < stars[i].r || r < stars[j].r) {
                // 運動量保存より速度を計算(質量で割ってはいない)
                stars[i].v = add(scalar_mul(stars[i].v, stars[i].m),
                                 scalar_mul(stars[j].v, stars[j].m));
                
                // 質量を更新して正しい速度を計算する
                stars[i].m += stars[j].m;
                stars[i].v = scalar_mul(stars[i].v, 1 / stars[i].m);
                
                // 位置の更新(中間にする)
                stars[i].p = scalar_mul(add(stars[i].p, stars[j].p),
                                        1.0 / 2.0);
                
                // 半径の更新(完全な球体として体積を足しあわせて考える)
                stars[i].r = pow(pow(stars[i].r, 3) + pow(stars[j].r, 3),
                                 1.0 / 3.0 );
                
                stars[j].m = DELETED; // 消したことにする
            }
        }
    }
}

/* 速度の更新をする関数 */
void update_velocities(double dt)
{
    int i, j;
    
    check_collision();  // 衝突の判定
         
    for (i = 0; i < nstars; i++) {
        Vector2D a = {0.0, 0.0};
        if (stars[i].m < 0) continue;   // 消去済み
        for (j = 0; j < nstars; j++) {
            if (i == j || stars[j].m < 0) continue; // 消去済み
            const Vector2D dp = sub(stars[j].p, stars[i].p);
            const double r = norm(dp);
            
            a = add(a,
                    scalar_mul(dp, stars[j].m / (r * r * r)));
        }
        a = scalar_mul(a, G);
        stars[i].v = add(stars[i].v,
                         scalar_mul(a, dt));
    }
}

/* 位置を更新する関数 */
void update_positions(double dt)
{
    int i;
    for (i = 0; i < nstars; i++) {
        if (stars[i].m < 0) continue;   // 消去済み
        stars[i].p = add(stars[i].p,
                         scalar_mul(stars[i].v, dt));
    }
}

int main(int argc, char *argv[])
{
    const char *space_file = "space.txt";   // 書き出すファイル
    double dt = 1.0;          // 時刻の刻み幅
    const double stop_time = 400;   // 実行時間
    FILE *fp;
    int c;           // オプション指定
    int i;
    double t = 0;
    
    if ((fp = fopen(space_file, "w")) == NULL) {
        fprintf(stderr, "error: cannot open %s.\n", space_file);
        return 1;
    }
    
    // コマンドライン引数の読み込み
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            if (i + 1 < argc)
                dt = atof(argv[i + 1]); // 刻み幅
        }
    }

    // 繰り返し
    for (i = 0; t <= stop_time; i++, t += dt) {
        update_velocities(dt);
        update_positions(dt);
        if (i % 10 == 0) {
            plot_stars(fp, t);
        }
    }
    
    fclose(fp);
}
