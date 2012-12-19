#include <stdio.h>
#include <process.h>
#include "image.h"

unsigned char image_in[Y_SIZE][X_SIZE][3];	/* 入力カラー画像配列 */
unsigned char image_out[Y_SIZE][X_SIZE][3];	/* 出力カラー画像配列 */
unsigned char image_temp[Y_SIZE][X_SIZE];	/* 濃淡画像作成用配列 */

// 24bit BMPのヘッダ情報を作成
void init(void)
{
	// BITMAPFILE HEADER   説明(デフォルト値，よく使う値)
	bfType = 0x4d42;	// ファイルのタイプ(0x4d42)
	bfSize = 196662;	// ファイルのバイト数
	bfReserved1 = 0x00;	// 予約域(0x00)
	bfReserved2 = 0x00;	// 予約域(0x00)
	bfOffBits = 0x36;	// 先頭から画像情報までのオフセット(0x36)
	
	// BITMAPINFO HEADER       説明(デフォルト値，よく使う値)
	biSize = 0x28;			// BITMAPINFO HEADER構造体のサイズ(0x28)
	biWidth = 256;			// 画像の幅
	biHeight = 256;			// 画像の高さ
	biPlanes = 0x01;		// カラープレーン(0x01)
	biBitCount = 0x18;		// ピクセルあたりのビット数(0x01,0x04,0x08,0x18)
	biCompression = 0x00;	// 圧縮タイプ(0x00)
	biSizeImage = 196608;	// ビットイメージバイト数
	biXPelsPerMeter = 0x00;	// 水平解像度(0x00)
	biYPelsPerMeter = 0x00;	// 垂直解像度(0x00)
	biClrUsed = 0x00;		// カラーテーブル内のカラーインデックス数(0x00)
	biClrImportant = 0x00;	// 重要なカラーインデックス数(0x00)
}

void main(void)
{
	char input[100], output[100];
	unsigned char image[Y_SIZE][X_SIZE] = {0};
	int i, j;

	for(i = 0; i < Y_SIZE; i++)
	{
		for(j = 0; j < X_SIZE; j++)
			image[i][j] = rand() % 256;
	}
	
	printf("入力テキストファイル名(input.txt)：");  scanf("%s", input);
	printf("出力画像ファイル名(output.bmp)："); scanf("%s", output);

	/* ヘッダ情報の初期化 */
	init();

	/* 画像処理 */
	//to256BW(image_in, image_temp);	/* 濃淡画像 */
	//to24BMP(image_temp, image_out); /* 24ビットBMPの作成 */
	
	//to24BMP(image, image_out);
	
	fromTextTo24BMP(input, image_out);

	/* 画像の出力 */
	writeBmp(image_out, output);  /* RGB24ビット画像をファイルに出力 */
}
