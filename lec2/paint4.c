/*
 * 簡易ペイントソフト
 * ・undoを変更点を保持して戻す形に変更
 *  (線形リストを動的スタックのように扱うことで表現)
 * ・redoの実装
 * ・ペンの変更を可能に
 * ・BMPへの変換を可能に(ペンの文字によって濃淡画像を作る)
 * ・塗りつぶし
 * ・ヘルプの表示
 * ・テキストファイルの読み込み(64*64)[未完]
 * ・ビットマップファイルの読み込み[しきい値を設けてアスキー文字で表示しようとしたが未完]
 * 学籍番号: 03-123006
 * 氏名: 岩成達哉
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIDTH 64    // 横幅
#define HEIGHT 64   // 縦幅
#define BUFSIZE 1000    // バッファの大きさ

// コマンドの結果を表すコードとコメント
enum COM_STATE {COM_SUCCESS, COM_ERROR, COM_FEW_PARAM, COM_OTHER, COM_QUIT, COM_UTIL};
const char *ERROR_MES[] = {
    "正常に処理されました。\n",
    "パラメータが不正です。ヘルプを見るためには H と入力してください。\n",
    "パラメータが足りません。ヘルプを見るためには H と入力してください。\n",
    "存在しないコマンドです。ヘルプを見るためには H と入力してください。\n",
    "終了します。\n",
    "正常に処理されました。\n"
};

/* 変更された点の情報 */
typedef struct struct_diff {
    int x;
    int y;
    char before;
    struct struct_diff *next;
} DIFF;

/* 1つのコマンドで変更された情報の集まり */
typedef struct struct_diff_list {
    DIFF *diff;
    struct struct_diff_list *next;
} STACK;

unsigned char g_canvas[WIDTH][HEIGHT];  // キャンバス
STACK *history;                         // 履歴
STACK *undo_history;                    // redo用のundoしたリスト
unsigned char pen = '#';                // ペンの文字

/***** bitmap関連の関数 START *****/
typedef unsigned short WORD;	// 2byte
typedef unsigned int  DWORD;	// 4byte

// ヘッダ情報
WORD    bfType;
DWORD   bfSize;
WORD    bfReserved1,
        bfReserved2;
DWORD   bfOffBits;
DWORD   biSize,
        biWidth, biHeight;
WORD    biPlanes,
        biBitCount;
DWORD   biCompression,
        biSizeImage,
        biXPelsPerMeter,
        biYPelsPerMeter,
        biClrUsed,
        biClrImportant;

unsigned char image_in[WIDTH][HEIGHT][3];	// 入力カラー画像配列
unsigned char image_out[WIDTH][HEIGHT][3];	// 出力カラー画像配列
unsigned char image_temp[WIDTH][HEIGHT];	// 濃淡画像作成用配列

#define HIGH   255	// 2値画像のhigh level
#define LOW      0	// 2値画像のlow level
#define LEVEL  256	// 濃度レベル数

/* 24bit BMPのヘッダ情報を作成 */
void init_bmp(void)
{
	// ファイル上ではBITMAPFILE HEADER, BITMAPINFO HEADERの順
	// BITMAPINFO HEADER       説明(デフォルト値，よく使う値)
	biSize = 0x28;          // BITMAPINFO HEADER構造体のサイズ(0x28)
	biWidth = WIDTH;        // 画像の幅
	biHeight = HEIGHT;      // 画像の高さ
	biPlanes = 0x01;		// カラープレーン(0x01)
	biBitCount = 0x18;		// ピクセルあたりのビット数(0x01,0x04,0x08,0x18)
	biCompression = 0x00;	// 圧縮タイプ(0x00)
	biSizeImage = biWidth * biHeight * (biBitCount / 8);	// ビットイメージバイト数
	biXPelsPerMeter = 0x00;	// 水平解像度(0x00)
	biYPelsPerMeter = 0x00;	// 垂直解像度(0x00)
	biClrUsed = 0x00;		// カラーテーブル内のカラーインデックス数(0x00)
	biClrImportant = 0x00;	// 重要なカラーインデックス数(0x00)
    
	// BITMAPFILE HEADER   説明(デフォルト値，よく使う値)
	bfType = 0x4d42;        // ファイルのタイプ(0x4d42)
	bfSize = biSizeImage + 32; // ファイルのバイト数 ヘッダ部分の大きさを加算
	bfReserved1 = 0x00;     // 予約域(0x00)
	bfReserved2 = 0x00;     // 予約域(0x00)
	bfOffBits = 0x36;       // 先頭から画像情報までのオフセット(0x36)
}

/*
 * char型配列に格納されたデータをBMPファイルに出力する
 * 横幅は4の倍数にすること
 */
void write_bmp(
            unsigned char image[WIDTH][HEIGHT][3],
            char *filename                         )
{
	FILE *fp;
	int i, j, k;
    
	if ( (fp = fopen(filename, "wb")) == NULL )
	{
		printf("writeBmp: Open error!\n");
		exit(1);
	}
	printf("output file : %s\n", filename);
    
	// ヘッダー情報
	fwrite(&bfType, sizeof(bfType), 1, fp);
	fwrite(&bfSize, sizeof(bfSize), 1, fp);
	fwrite(&bfReserved1, sizeof(bfReserved1), 1, fp);
	fwrite(&bfReserved2, sizeof(bfReserved2), 1, fp);
	fwrite(&bfOffBits, sizeof(bfOffBits), 1, fp);
    
	fwrite(&biSize, sizeof(biSize), 1, fp);
	fwrite(&biWidth, sizeof(biWidth), 1, fp);
	fwrite(&biHeight, sizeof(biHeight), 1, fp);
	fwrite(&biPlanes, sizeof(biPlanes), 1, fp);
	fwrite(&biBitCount, sizeof(biBitCount), 1, fp);
	fwrite(&biCompression, sizeof(biCompression), 1, fp);
	fwrite(&biSizeImage, sizeof(biSizeImage), 1, fp);
	fwrite(&biXPelsPerMeter, sizeof(biXPelsPerMeter), 1, fp);
	fwrite(&biYPelsPerMeter, sizeof(biYPelsPerMeter), 1, fp);
	fwrite(&biClrUsed, sizeof(biClrUsed), 1, fp);
	fwrite(&biClrImportant, sizeof(biClrImportant), 1, fp);
    
	// ビットマップデータ
	for (i = biHeight - 1; i >= 0; i--)
	{
		for (j = 0; j < biWidth; j++)
		{
			for (k=0; k<3; k++)
            {
                // 背景は白に
                if (image[j][i][2-k] == ' ')
                    image[j][i][2-k] = 255;
                
                fwrite(&image[j][i][2-k], 1, 1, fp);
            }
		}
	}
    
	fclose(fp);
}

/* 256諧調白黒濃淡画像へ変換 */
void to_256_bw(
    unsigned char image[WIDTH][HEIGHT][3],
    unsigned char image_bw[WIDTH][HEIGHT]  )
{
	int y, x, a;
	for (y = 0; y < biHeight; y++)
	{
		for (x = 0; x < biWidth; x++)
		{
			a = 0.3*image[y][x][0] + 0.59*image[y][x][1] + 0.11*image[y][x][2];
			if (a < LOW) a = LOW;
			if (a > HIGH) a = HIGH;
			image_bw[y][x] = a;
		}
	}
}

/* 閾値により画像へ変換 */
void to_ascii_image(
    unsigned char image_prev[WIDTH][HEIGHT],
    unsigned char image_post[WIDTH][HEIGHT]  )
{
	int y, x;
    unsigned char a;
	for (y = 0; y < biHeight; y++)
	{
		for (x = 0; x < biWidth; x++)
		{
			a = image_prev[y][x];
            if (a < 32)
                a = '@';
            else if(a < 64)
                a = '#';
            else if(a < 96)
                a = '*';
            else if(a < 128)
                a = '=';
            else if(a < 160)
                a = '!';
            else if(a < 192)
                a = '-';
            else if(a < 224)
                a = '.';
            else
                a = ' ';
			image_post[y][x] = a;
		}
	}
}

/* 濃淡画像を24ビットBMP形式に変換 */
void to_24_bmp(
    unsigned char image_bw[WIDTH][HEIGHT],
    unsigned char image[WIDTH][HEIGHT][3] )
{
	int y, x, a;
	for (y=0; y<biHeight; y++)
	{
		for (x=0; x<biWidth; x++)
		{
			a = image_bw[y][x];
			image[y][x][0] = a;
			image[y][x][1] = a;
			image[y][x][2] = a;
		}
	}
}

/* textファイルを24ビットBMP形式に変換 */
int from_text_to_24_bmp(
                        char *filename,
                        unsigned char image[WIDTH][HEIGHT][3])
{
	char temp;
	FILE *fp;
	int i, j, k;
    
	if ( (fp = fopen(filename, "r")) == NULL )
	{
		printf("File Open Error: %s\n", filename);
		return -1;
	}
    
	for(i = 0; i < WIDTH; i++)
	{
		for(j = 0; j < HEIGHT; j++)
		{
			if( (temp = fgetc(fp)) != EOF )
			{
				image[i][j][0] = temp;
				image[i][j][1] = temp;
				image[i][j][2] = temp;
			}
			else
			{
				image[i][j][0] = 0;
				image[i][j][1] = 0;
				image[i][j][2] = 0;
			}
		}
	}
	fclose(fp);
    
    return 0;
}

/* textファイルを読み込む関数 */
int from_text(
        char *filename,
        unsigned char image[WIDTH][HEIGHT])
{
	char temp;
	FILE *fp;
	int i, j;
    int flag = 1;
    
	if ( (fp = fopen(filename, "r")) == NULL )
	{
		printf("File Open Error: %s\n", filename);
		return -1;
	}
    
    for(j = 0; j < WIDTH; j++)
    {
        for(i = 0; i < HEIGHT; i++)
        {
			if( (temp = fgetc(fp)) != EOF)
			{
                if (temp == '\n') {
                    continue;
                }
                else
                    image[i][j] = temp;
			}
			else
			{
				image[i][j] = ' ';
			}
		}
	}
	fclose(fp);
    
    return 0;
}

/* 24BMPを読み込み */
int from_24_bmp_to_text(
    char *filename,
    unsigned char image[WIDTH][HEIGHT][3])
{
	FILE *fp;
    unsigned char header[32];   // ヘッダ(読み飛ばし)
    int i, j, k;
    
    // ファイルから読み込む
    if ( (fp = fopen(filename, "rb")) == NULL )
    {
        printf("File Open Error: %s\n", filename);
        return -1;
    }
    
    fseek(fp, 32, SEEK_SET);
    
    // ビットマップデータ
	for (i = biHeight - 1; i >= 0; i--)
	{
		for (j = 0; j < biWidth; j++)
		{
			for (k=0; k<3; k++)
            {
                // 1つずつ読み込む
                fread(&image[j][i][2-k], 1, 1, fp);
            }
		}
	}
    fclose(fp);
    
    return 0;
}

/***** bitmap関連の関数 END *****/

/***** history関連の関数 START *****/
/* 1回のコマンドにつき1つの履歴を作る関数 */
void make_history()
{
    STACK *n;
    
    // 先頭に入れるだけ
    n = (STACK *)malloc(sizeof(STACK));
    n->diff = NULL;
    n->next = history;
    history = n;
}

/* 変更を点ごとに記録する関数 */
void add_diff(int x, int y, char c)
{
    DIFF *p, *n;
    
    // 範囲内かどうか
    if (0 > x || x >= WIDTH || 0 > y || y >= HEIGHT)
        return;
    
    // 前の状態と異なるなら
    if (g_canvas[x][y] != c) {
        // データをセットして先頭に入れる
        n = (DIFF *)malloc(sizeof(DIFF));
        n->x = x;
        n->y = y;
        n->before = g_canvas[x][y]; // 前の状態を持つ
        n->next = history->diff;
        history->diff = n;
        
        g_canvas[x][y] = c; // 変更しておく
    }
}

/* 履歴を一つ削除する関数 */
void delete_history(STACK **his)
{
    STACK *d;
    DIFF *p, *q;
    
    d = *his;   // 親
    
    if (d != NULL)
    {
        p = d->diff;
        // 全てなくなるまで回す
        while (p != NULL) {
            q = p;
            p = p->next;
            free(q);
        }
        *his = d->next;
        free(d);
    }
}
/***** history関連の関数 END *****/

/***** canvas関連の関数 START *****/
/* キャンバスの初期化をする関数 */
int init_canvas()
{
    memset(g_canvas, ' ', sizeof(g_canvas));
}

/* キャンバスを表示する関数 */
void print_canvas(FILE *fp)
{
    int x, y;
    
    fprintf(fp, "----------\n");
    
    for (y = 0; y < HEIGHT; y++)
    {
        for (x = 0; x < WIDTH; x++)
        {
            char c = g_canvas[x][y];
            fputc(c, fp);
        }
        fputc('\n', fp);
    }
    fflush(fp); // すぐに書きだす
}
/***** canvas関連の関数 END *****/


/***** 描画関連の関数 START *****/
/* 線を描く関数 */
int draw_line(int param[])
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
        
        add_diff(x, y, pen);    // 変更してdiffを取る
    }
    return COM_SUCCESS;
}

/* 円を描く関数 */
int draw_circle(int param[])
{
    int x0 = param[0], y0 = param[1], r = param[2];
    int n = ceil(2 * M_PI * r); // 分割数として円周を使う
    int i;
    
    if (r <= 0) // 0以下なら実行しない
        return COM_ERROR;
    
    // 近似的な円を描く
    for (i = 1; i < n; i++) {
        int x = x0 + r * cos((double)i / r);
        int y = y0 + r * sin((double)i / r);
        
        add_diff(x, y, pen);    // 変更してdiffを取る
    }
    
    return COM_SUCCESS;
}

/* 長方形を描く関数 */
int draw_rectangle(int param[])
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
    for (i = 0; i < 4; i++)
    {
        draw_line(point[i]);
    }
    
    return COM_SUCCESS;
}

/* 
 * 塗りつぶしを行う再帰的な関数
 * oringnalは塗りつぶしを始める最初の点
 * これと同じものを塗り替える
 */
void fill(int x, int y, unsigned char original)
{
    add_diff(x, y, pen); // penで塗る
    
    // 上
    if (y - 1 > 0 && g_canvas[x][y-1] == original)
        fill(x, y-1, original);
    
    // 左
    if (x - 1 > 0 && g_canvas[x-1][y] == original)
        fill(x-1, y, original);
    
    // 右
    if (x + 1 < WIDTH && g_canvas[x+1][y] == original)
        fill(x+1, y, original);
    
    // 下
    if (y + 1 < HEIGHT && g_canvas[x][y+1] == original)
        fill(x, y+1, original);
}

/* 塗りつぶしを行う関数を呼び出すラッパ的な関数 */
int fill_div(int param[])
{
    int x = param[0], y = param[1];
    
    // 初期位置の確認
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return COM_ERROR;
    
    // 関数を呼び出す
    fill(x, y, g_canvas[x][y]);
    return COM_SUCCESS;
}

/* 長方形の領域を消す関数 */
int erase(int param[])
{
    int x0 = param[0], y0 = param[1], x1 = param[2], y1 = param[3];
    int x, y;
    
    for (x = x0; x < x1; x++) {
        for (y = y0; y < y1; y++) {
            add_diff(x, y, ' ');    // 空白で塗りつぶし
        }
    }
    return COM_SUCCESS;
}
/***** 描画関連の関数 END *****/

/***** Utility関連の関数 START *****/
/* ヘルプを表示する関数 */
void show_help()
{
    printf("*****************************\n");
    printf("【コマンド一覧】\n");
    
    printf("-- 描画関連 --\n");
    printf("l  2点(x0, y0),(x1, y1)を結ぶ直線を引く\n");
    printf("\tUsage: l x0 y0 x1 y1\n");
    printf("c  点(x0, y0)を中心に半径rの円を描く\n");
    printf("\tUsage: c x0 y0 r\n");
    printf("r  2点(x0, y0),(x1, y1)を向かい合う点とする長方形を描く\n");
    printf("\tUsage: r x0 y0 x1 y1\n");
    printf("f  点(x0, y0)を含む閉領域の塗りつぶしを行う\n");
    printf("\tUsage: f x0 y0\n");
    printf("e  2点(x0, y0),(x1, y1)を向かい合う点とする長方形領域を消しゴムで消す\n");
    printf("\tUsage: e x0 y0 x1 y1\n");
    
    printf("\n");
    
    printf("-- Utility関連 --\n");
    printf("P  penの文字(np)を変更する(defaultは#)\n");
    printf("\tUsage: P np\n");
    printf("U  undoをする\n");
    printf("R  redoをする\n");
    printf("W  現在のcanvasをBMPに変換する\n");
    printf("\tUsage: W output(bmp拡張子は不要)\n");
    printf("L  BMPを読み込んでcanvasにASCII文字として表示する\n");
    printf("\tUsage: L input(bmp拡張子は不要)\n");
    printf("T  テキストファイルを読み込んでcanvasにASCII文字として表示する\n");
    printf("\tUsage: T input(txt拡張子は不要)\n");
    printf("H  helpを表示する\n");
    printf("Q  終了する\n");
    printf("*****************************\n");
}

/* 
 * undoを行う関数
 * redoはundoのundoとして表現
 */
int undo(STACK **his, STACK **undo_his)
{
    STACK *d;
    DIFF *p, *q;
    char bef;
    
    d = *his;   // 親
    
    if (d != NULL)
    {
        p = d->diff;
        while (p != NULL)
        {
            q = p;
            p = p->next;
            // 現在の状態を受け取る(これを新たなdiffとする)
            bef = g_canvas[q->x][q->y];
            g_canvas[q->x][q->y] = q->before;
            q->before = bef;
        }
        *his = d->next;
        
        d->next = *undo_his;    // undoのリストに加える
        *undo_his = d;
    }
    
    return COM_UTIL;
}

/* penを変える関数 */
int change_pen()
{
    char *param;
    
    param = strtok(NULL, " \n");
    if (param == NULL)
        return COM_FEW_PARAM;
    
    pen = param[0];
    
    return COM_UTIL;
}

/* 現在のキャンバスをBMPに変換する関数 */
int convert_to_bmp()
{
    char *param;
    char *output;
    
    param = strtok(NULL, " \n");
    
    if (param == NULL)
        return COM_FEW_PARAM;
    
    // 拡張子をつける
    output = (char *)malloc(sizeof(char) * (strlen(param) + 4));
    sprintf(output, "%s.bmp", param);
    
    init_bmp();                         // ヘッダ情報の初期化
    to_24_bmp(g_canvas, image_out);     // canvasを24ビットBMP形式に
    write_bmp(image_out, output);       // 書き出し
    
    free(output);
    return COM_UTIL;
}

/* 24BMPをテキストする関数 */
int convert_to_ascii_from_bmp()
{
    char *param;
    char *input;
    
    param = strtok(NULL, " \n");
    
    if (param == NULL)
        return COM_FEW_PARAM;
    
    // 拡張子をつける
    input = (char *)malloc(sizeof(char) * (strlen(param) + 4));
    sprintf(input, "%s.bmp", param);
    
    init_bmp(); // ヘッダ情報の書き出し
    if( from_24_bmp_to_text(input, image_in) != 0)// BMPファイル読み込み
    {
        free(input);
        return COM_ERROR;   // 開けないとき
    }
    free(input);
    
    // 今までのUNDOの履歴を消去
    while (undo_history != NULL)
        delete_history(&undo_history);
    
    to_256_bw(image_in, image_temp);        // 濃淡画像に
    //to_ascii_image(image_temp, g_canvas);   // 濃さでアスキー文字による画像を生成
    
    return COM_UTIL;
}

/* おまけ: テキストをキャンバスに表示する関数 */
int convert_to_bmp_from_txt()
{
    char *param;
    char *input;
    
    param = strtok(NULL, " \n");
    
    if (param == NULL)
        return COM_FEW_PARAM;
    
    // 今までのUNDOの履歴を消去
    while (undo_history != NULL)
        delete_history(&undo_history);
    
    // 拡張子をつける
    input = (char *)malloc(sizeof(char) * (strlen(param) + 4));
    sprintf(input, "%s.txt", param);
    
    init_bmp();
    
    if (from_text(input, image_temp) != 0)   // 文字列を24ビットBMP形式に
        return COM_ERROR;   // 開けなかったとき
    
    memcpy(g_canvas, image_temp, sizeof(unsigned char) * HEIGHT * WIDTH); // cpy
    //to_ascii_image(image_temp, g_canvas);   // 濃さでアスキー文字による画像を生成
    return COM_UTIL;
}
/***** Utility関連の実行関数 END *****/

/***** コマンドの実行関数 START *****/
/* コマンドを実行する関数 */
int exe_command(int (*func)(int []), int n)
{
    char *text;
    int *param = (int *)malloc(sizeof(int) * n);
    int res;
    int i;
    
    // 引数の格納
    for (i = 0; i < n; i++)
    {
        text = strtok(NULL, " \n");
        if (text == NULL)
            break;
        
        param[i] = atoi(text);
    }
    
    // 引数が足りなければ何もしない
    if (n != i)
        return COM_FEW_PARAM;
    
    // 今までのUNDOの履歴を消去
    while (undo_history != NULL)
        delete_history(&undo_history);

    make_history(); // 変更点を保存する構造体を確保
    res = func(param);  // 関数の実行
    free(param);        // 消しておく
    return res;
}

/* コマンドを解釈する関数 */
int interpret_command(const char *command)
{
    char buf[BUFSIZE];
    strcpy(buf, command);
    
    char *s = strtok(buf, " ");
    char c = s[0];
    
    switch (c)
    {
        case 'l':   // 線を描く
            return exe_command(draw_line, 4);
        case 'c':   // 円を描く
            return exe_command(draw_circle, 3);
        case 'r':   // 長方形を描く
            return exe_command(draw_rectangle, 4);
        case 'f':   // 塗りつぶす
            return exe_command(fill_div, 2);
        case 'e':   // 消しゴム
            return exe_command(erase, 4);
        case 'U':   // Undo
            return undo(&history, &undo_history);
        case 'R':   // Redo
            return undo(&undo_history, &history);   // undoのundo
        case 'Q':   // 終了
            return COM_QUIT;
        case 'H':   // ヘルプを表示
            show_help();
            return COM_UTIL;
        case 'P':   // ペンを変更
            return change_pen();
        case 'W':   // ビットマップに書き出し
            return convert_to_bmp();
        case 'L':   // ビットマップを読み込み
            return convert_to_ascii_from_bmp();
        case 'T':   // テキストを読み込み
            return convert_to_bmp_from_txt();
        default:
            break;
    }
    return COM_OTHER;
}
/***** コマンドの実行関数 END *****/

int main()
{
    int com, i;
    const char *canvas_file = "canvas.txt"; // 書き出すファイル名
    const char *history_file = "_history_temp.txt"; // 履歴の一時保存先
    FILE *fp_can, *fp_his;
    char buf[BUFSIZE];
    
    if ((fp_can = fopen(canvas_file, "w")) == NULL)
    {
        fprintf(stderr, "error: cannot open %s.\n", canvas_file);
        return 1;
    }
    
    init_canvas();  // キャンバスの初期化
    print_canvas(fp_can);
    
    if ((fp_his = fopen(history_file, "w")) == NULL)
    {
        fprintf(stderr, "error: cannot open %s.\n", canvas_file);
        return 1;
    }
    
    while (1)
    {
        printf("現在のpen[%c] > ", pen);   // ペンの表示
        fgets(buf, BUFSIZE, stdin); // 標準入力からの読み出し
        
        com = interpret_command(buf);   // 実行
        
        // 結果が成功したかどうか
        if (com != COM_SUCCESS && com != COM_UTIL && com != COM_QUIT)
        {
            printf("%s", ERROR_MES[com]);   // 問題を通知
            
            // コマンドがエラーだった場合は記録しない
            if (com == COM_ERROR)
                delete_history(&history);
        }
        else
        {
            // コマンドの書き出し
            fprintf(fp_his, "%s", buf);
            
            // 終了
            if (com == COM_QUIT)
                break;
            
            print_canvas(fp_can);
        }
    }
    fclose(fp_can);
    fclose(fp_his);
    
    
    // 解放
    while (history != NULL)
        delete_history(&history);
    while (undo_history != NULL)
        delete_history(&undo_history);
    
    // ファイル名を変更して履歴を残す
    if(rename(history_file, "history.txt") != 0) {
        printf("cannot save history\n");
        return 1;
    }
    
    return 0;
}