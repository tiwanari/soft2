/*
 * 物理シミュレーション
 *  課題2: ２次元空間に拡張
 * 学籍番号: 03-123006
 * 氏名: 岩成達哉
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIDTH 75    // 幅
#define HEIGHT 50   // 高さ

const double G = 1.0;   // 万有引力定数

/* 星 */
struct star
{
    double m;
    double x;
    double y;
    double vx;
    double vy;
};

// 星を設置
struct star stars[] = {
    { 1.0, -10.0, 5.0, 0.1, 0.0 },
    { 0.5,  10.0, 10.0, 0.0, 0.2 },
    { 0.5,  0.0, 0.0, 0.1, 0.2 },
    { 0.3,  30.0, 30.0, 0.0, 0.2 },
    { 0.3,  30.0, 50.0, 0.0, -0.2 } };

// 星の数
const int nstars = sizeof(stars) / sizeof(struct star);

/* 星の表示をする関数 */
void plot_stars(FILE *fp, const double t)
{
    int i;
    char space[WIDTH][HEIGHT];    // 表示用
    
    memset(space, ' ', sizeof(space));    // 初期化
    // 星を表示
    for (i = 0; i < nstars; i++) {
        const int x = WIDTH  / 2 + stars[i].x;
        const int y = HEIGHT / 2 + stars[i].y;
        if (x < 0 || x >= WIDTH)  continue;
        if (y < 0 || y >= HEIGHT) continue;
        char c = 'o';
        if (stars[i].m >= 1.0) c = 'O'; // 質量が大きい星
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
        printf(", [%d]:stars.x = %7.2f, stars.y = %7.2f", i, stars[i].x, stars[i].y);
    printf("\n");
    
    usleep(100 * 1000);
}

/* 速度の更新をする関数 */
void update_velocities(const double dt)
{
    int i, j;
    for (i = 0; i < nstars; i++) {
        double ax = 0;
        double ay = 0;
        for (j = 0; j < nstars; j++) {
            if (i == j) continue;
            const double dx = stars[j].x - stars[i].x;
            const double dy = stars[j].y - stars[i].y;
            const double r = sqrt(dx * dx + dy * dy);
            ax += stars[j].m * dx / (r * r * r);
            ay += stars[j].m * dy / (r * r * r);
        }
        ax *= G;
        ay *= G;
        stars[i].vx += ax * dt;
        stars[i].vy += ay * dt;
    }
}

/* 位置を更新する関数 */
void update_positions(const double dt)
{
    int i;
    for (i = 0; i < nstars; i++) {
        stars[i].x += stars[i].vx * dt;
        stars[i].y += stars[i].vy * dt;
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

    
    for (i = 0; t <= stop_time; i++, t += dt) {
        update_velocities(dt);
        update_positions(dt);
        if (i % 10 == 0) {
            plot_stars(fp, t);
        }
    }
    
    fclose(fp);
}
