/**
 * 世代ごとのセル数を出力できるように(変更点はprint_cellsのみ)
 * 学籍番号: 03-123006
 * 氏名: 岩成達哉
 */

#include <stdio.h>
#include <unistd.h>

#define HEIGHT (50)
#define WIDTH (70)

int cell[HEIGHT][WIDTH];        // フィールド
int cell_next[HEIGHT][WIDTH];   // 次のフィールド

/* フィールドをファイルへ書き出す関数 */
void print_cells(FILE *fp)
{
    int i, j;
    int num = 0;    // セル数
    
    fprintf(fp, "----------\n");
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            char c = (cell[i][j] == 1) ? '#' : ' ';
            if (c == '#') num++;    // セル数をカウントアップ
            fputc(c, fp);
        }
        fputc('\n', fp);
    }
    fprintf(fp, "--num = %d--\n", num); // セル数の表示
    fflush(fp);
    
    sleep(1);
}

/* フィールドの初期化をする関数 */
int init_cells()
{
    int i, j;
    
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            cell[i][j] = rand() % 2;
        }
    }
}

/* フィールドの更新 */
void update_cells()
{
    int i, j, k, l;
    
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            int n = 0;
            for (k = i - 1; k <= i + 1; k++) {
                if (k < 0 || k >= HEIGHT) continue;     // フィールド外
                for (l = j - 1; l <= j + 1; l++) {
                    if (k == i && l == j) continue;     // そのセル自身
                    if (l < 0 || l >= WIDTH) continue;  // フィールド外
                    n += cell[k][l];    // 8近傍セル数のカウント
                }
            }
            // 新しくセルを作成するかどうか
            if (cell[i][j] == 0) {
                if (n == 3)
                    cell_next[i][j] = 1;
                else
                    cell_next[i][j] = 0;
            } else {
                if (n == 2 || n == 3)
                    cell_next[i][j] = 1;
                else
                    cell_next[i][j] = 0;
            }
        }
    }
    
    // 代入
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            cell[i][j] = cell_next[i][j];
        }
    }
}


int main()
{
    int gen = 1;
    
    FILE *fp;
    
    // 書き出し用ファイルを開く
    if ((fp = fopen("cells.txt", "w")) == NULL) {
        fprintf(stderr, "error: cannot open a file.\n");
        return 1;
    }
    
    init_cells();
    print_cells(fp);
    
    // ずっと繰り返す
    while (1) {
        printf("gen = %d\n", gen++);
        update_cells();
        print_cells(fp);
    }
    
    fclose(fp);
}
