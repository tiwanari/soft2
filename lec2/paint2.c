/*
 * 簡易ペイントソフト
 *  課題2: 円や長方形を描く
 * 学籍番号: 03-123006
 * 氏名: 岩成達哉
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIDTH 70    // 横幅
#define HEIGHT 40   // 縦幅
#define BUFSIZE 1000    // バッファの大きさ
#define COM_OTHER 0     // 無いコマンド
#define COM_QUIT 1      // 終了コマンド
#define HISTORY_SIZE 100    // 履歴の多さ

char canvas[WIDTH][HEIGHT];     // キャンバス
char *history[HISTORY_SIZE];    // 履歴(undo用)

/* キャンバスを表示する関数 */
void print_canvas(FILE *fp)
{
    int x, y;
    
    fprintf(fp, "----------\n");
    
    for (y = 0; y < HEIGHT; y++) {
        for (x = 0; x < WIDTH; x++) {
            char c = canvas[x][y];
            fputc(c, fp);
        }
        fputc('\n', fp);
    }
    fflush(fp); // すぐに書き出し
}

/* キャンバスの初期化をする関数 */
int init_canvas()
{
    memset(canvas, ' ', sizeof(canvas));
}

/* 線を描く関数 */
void draw_line(int param[])
{
    int x0 = param[0], y0 = param[1], x1 = param[2], y1 = param[3];
    int dx = abs(x1 - x0), dy = abs(y1 - y0);   // 差分の計算
    int num = (dx >= dy) ? dx : dy;             // チェス盤距離
    double delta = (num != 0) ? 1.0 / num : 0;  // 分割数を距離に応じて変える
    int i;
    
    for (i = 0; i <= num; i++)
    {
        int x = x0 + delta * i * (x1 - x0) + 0.5;   // 四捨五入
        int y = y0 + delta * i * (y1 - y0) + 0.5;   //
        
        if (0 <= x && x < WIDTH && 0 <= y && y < HEIGHT) {  // 変更してないか
            canvas[x][y] = '#'; // 変更する
        }
    }
}

/* 円を描く関数 */
void draw_circle(int param[])
{
    int x0 = param[0], y0 = param[1], r = param[2];
    int n = ceil(2 * M_PI * r); // 分割数として円周を使う
    int i;
    
    if (r <= 0) // 0以下なら実行しない
        return ;
    
    // 近似的な円を描く
    for (i = 1; i < n; i++) {
        int x = x0 + r * cos((double)i / r);
        int y = y0 + r * sin((double)i / r);
        
        if (0 <= x && x < WIDTH && 0 <= y && y < HEIGHT) {
            canvas[x][y] = '#';
        }
    }
    
}

/* 長方形を描く関数 */
void draw_rectangle(int param[])
{
    int x0 = param[0], y0 = param[1], x1 = param[2], y1 = param[3];
    // 頂点を計算
    int point[4][4] = {
        {x0, y0, x0, y1},
        {x0, y1, x1, y1},
        {x1, y1, x1, y0},
        {x1, y0, x0, y0},
    };
    int i;
    
    // 直線を引く関数を4回呼び出す
    for (i = 0; i < 4; i++) {
        draw_line(point[i]);
    }
}

/* コマンドを実行する関数 */
void exe_command(void (*func)(int []), int n)
{
    char *text;
    int *param = (int *)malloc(sizeof(int) * n);
    int i;
    
    // 引数の格納
    for (i = 0; i < n; i++) {
        text = strtok(NULL, " \n");
        if (text == NULL)
            break;
        
        param[i] = atoi(text);
    }
    
    // 引数が足りなければ何もしない
    if (i != n)
        return;
    
    func(param);    // 関数ポインタで指定された内容で実行
    free(param);
}

/* コマンドを解釈する関数 */
int interpret_command(const char *command)
{
    char buf[BUFSIZE];
    strcpy(buf, command);
    
    char *s = strtok(buf, " ");
    char c = tolower(s[0]);
    
    switch (c) {
        case 'l':   // 線を描く
            exe_command(draw_line, 4);
            break;
        case 'c':   // 円を描く
            exe_command(draw_circle, 3);
            break;
        case 'r':   // 長方形を描く
            exe_command(draw_rectangle, 4);
            break;
        case 'q':   // 終了
            return COM_QUIT;
        default:
            break;
    }
    return COM_OTHER;
}

int main()
{
    int com, n, i;
    const char *canvas_file = "canvas.txt"; // 書き出すファイル名
    FILE *fp;
    char buf[BUFSIZE];
    
    if ((fp = fopen(canvas_file, "w")) == NULL) {
        fprintf(stderr, "error: cannot open %s.\n", canvas_file);
        return 1;
    }
    
    init_canvas();  // キャンバスの初期化
    print_canvas(fp);
    
    n = 0;  // 現在のターン数を初期化
    while (1) {
        printf("%d > ", n);
        fgets(buf, BUFSIZE, stdin); // 標準入力からの読み出し
        
        // コマンドをそのまま残しておく
        history[n] = (char*)malloc(sizeof(char) * (strlen(buf) + 1));
        strcpy(history[n], buf);
        if (++n >= HISTORY_SIZE) break; // 履歴の限界まで
        
        com = interpret_command(buf);   // コマンドの実行
        if (com == COM_QUIT) break; // 終了処理
        print_canvas(fp);
    }
    
    fclose(fp);
    
    // コマンドの書き出し
    if ((fp = fopen("history.txt", "w")) != NULL) {
        for (i = 0; i < n; i++) {
            fprintf(fp, "%s", history[i]);
            free(history[i]);
        }
        fclose(fp);
    }
    return 0;
}
