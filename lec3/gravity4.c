/*
 * 物理シミュレーション
 *  課題4: 精度を上げる
 *  ・ルンゲ=クッタ法に変更
 *  ・半径を持たせる
 *  ・衝突時に半径と位置を調整
 *  ・2次元ベクトルの処理は別ファイルvector2D.c/vector2D.hに
 *  ・乱数列による星の位置の初期化
 *  ・ファイルから数値を読み込んで星の位置を初期化
 *  ・正規分布を用いて星の位置を初期化
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
#define NUM_STARS 20    // 星の数

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
Star stars[NUM_STARS] = {};

// 星の数
int nstars = sizeof(stars) / sizeof(Star);

/** 
 * 星の表示をする関数
 * @param FILE* fp 書き出すファイルポインタ
 * @param double t 時刻
 */
void plot_stars(FILE *fp, double t)
{
    int count, i;
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
    
    printf("t = %5.1f \n", t);
    for (i = 0, count = 0; i < nstars; i++)
    {
        if (stars[i].m < 0) continue;   // 削除済み
        printf("r =%6.2f, m =%6.2f, ", stars[i].r, stars[i].m);
        show_component(stars[i].p);
        printf(", ");
        show_component(stars[i].v);
        printf(", ");
        count++;
        if (count % 2 == 0)
            printf("\n");
    }
    
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

/**
 * 速度の更新をする関数
 * @param double dt 刻み幅
 */
void update_velocities_and_position(double dt)
{
    int i, j;
    Vector2D next_v;
    check_collision();  // 衝突の判定
         
    for (i = 0; i < nstars; i++)
    {
        if (stars[i].m < 0) continue;   // 消去済み
        // この点の現在位置と次の予想位置との中間
        Vector2D middle_i = add(stars[i].p,
                                scalar_mul(
                                           stars[i].v,
                                           dt/2
                                           )
                                );
        Vector2D a = {0.0, 0.0};    // 加速度
        for (j = 0; j < nstars; j++)
        {
            if (i == j || stars[j].m < 0) continue; // 消去済み
            // 影響する点の次の在位置と次の予想位置との中間
            const Vector2D middle_j = add(stars[j].p,
                                          scalar_mul(
                                                     stars[j].v,
                                                     dt/2
                                                    )
                                          );
            // ２点の差ベクトル
            const Vector2D dp = sub(middle_j, middle_i);
            const double r = norm(dp);  // 距離
            
            // 加速度を計算
            a = add(a,
                    scalar_mul(dp, stars[j].m / (r * r * r)));
        }
        a = scalar_mul(a, G);   // 定数をかける
        // 次の速度を保存
        next_v = add(stars[i].v,
                     scalar_mul(a, dt));
        // 位置の更新
        stars[i].p = add(stars[i].p,
                         scalar_mul(
                                    add(next_v, stars[i].v),
                                    dt / 2
                                    )
                         );
        // 次の速度を代入
        stars[i].v = next_v;
    }
}

/**
 * 乱数を作る関数
 * @return double 乱数
 */
double genRand()
{
    return rand() / (1.0 + RAND_MAX);
}

/**
 * 正規分布を返す関数
 * @param double myu 平均
 * @param double sgm 分散
 * @return double 正規分布による値
 * 参考: http://www-section.cocolog-nifty.com/blog/2008/09/box-muller-2275.html
 */
double gaussian(double myu, double sgm)
{
    static int    frag = 0;
    static double save = 0.0;
    
    if(frag == 0)
    {
        double u_0 = genRand();
        double u_1 = genRand();
        double v_0 = myu + sgm * sqrt(-2 * log(1 - u_0)) * cos(2 * M_PI * u_1);
        double v_1 = myu + sgm * sqrt(-2 * log(1 - u_0)) * sin(2 * M_PI * u_1);
        save = v_1;
        frag = 1;
        return v_0; // 1つ目のデータを返す
    }
    else
    {
        frag = 0;
        return save;    // 2つ目のデータを返す
    }  
}

/* 乱数列による星の初期化 */
void init_stars(void)
{
    int i;
    
    srand((unsigned)time(NULL));
    
    for (i = 0; i < nstars; i++)
    {
        stars[i].m = (double)(rand() % 20) / 11;
        stars[i].r = (double)(rand() % 11) / 11;
        stars[i].p.x = (double)(rand() % 101 - 50) / 7;
        stars[i].p.y = (double)(rand() % 101 - 50) / 7;
        stars[i].v.x = (double)(rand() % 11 - 5) / 7;
        stars[i].v.y = (double)(rand() % 11 - 5) / 7;
    }
}

/**
 * ファイルによる星の初期化をする関数 
 * @param char *file_name ファイル名
 */
void init_stars_by_file(char *file_name)
{
    FILE *fp;
    int i;
    
    if ((fp = fopen(file_name, "r")) == NULL)
    {
        printf("Failed to Open: %s\n", file_name);
        exit(1);
    }
    
    srand((unsigned)time(NULL));
    
    for (i = 0; i < nstars; i++)
    {
        if ( fscanf(fp, "%lf %lf %lf %lf %lf %lf",
               &stars[i].m,
               &stars[i].r,
               &stars[i].p.x,
               &stars[i].p.y,
               &stars[i].v.x,
               &stars[i].v.y) == EOF)
        {
            stars[i].m = DELETED;   // 存在しないなら
        }
    }
    fclose(fp);
}


/**
 * 正規分布によって星の位置の初期化を行う関数
 */
void init_stars_by_gaussian(void)
{
    int i;
    
    for (i = 0; i < nstars; i++)
    {
        stars[i].m = fabs(gaussian(0.0, 1.0) * 2);
        stars[i].r = (double)(rand() % 11) / 11;
        stars[i].p.x = gaussian(0.0, 1.0) * 10;
        stars[i].p.y = gaussian(0.0, 1.0) * 10;
        stars[i].v.x = gaussian(0.0, 1.0);
        stars[i].v.y = gaussian(0.0, 1.0);
    }
}


int main(int argc, char *argv[])
{
    const char *space_file = "space.txt";   // 書き出すファイル
    double dt = 0.1;          // 時刻の刻み幅
    const double stop_time = 400;   // 実行時間
    FILE *fp;
    int c;           // オプション指定
    int i;
    double t = 0;
    
    
    if ((fp = fopen(space_file, "w")) == NULL)
    {
        fprintf(stderr, "error: cannot open %s.\n", space_file);
        return 1;
    }
    
    init_stars();   // 乱数で初期化
    
    // コマンドライン引数の読み込み
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0)
        {
            if (i + 1 < argc)
                dt = atof(argv[i + 1]); // 刻み幅
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            if (i + 1 < argc)
                init_stars_by_file(argv[i + 1]); // 開くファイルを読み込む
        }
        else if (strcmp(argv[i], "-g") == 0)
        {
            init_stars_by_gaussian();   // 正規分布による初期化
        }
    }
    
    
    // 繰り返し
    for (i = 0; t <= stop_time; i++, t += dt) {
        update_velocities_and_position(dt);
        if (i % 10 == 0) {
            plot_stars(fp, t);
        }
    }
    
    fclose(fp);
}
