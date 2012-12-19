#include <stdio.h>
#include <process.h>
#include "image.h"

/* 
unsigned char型配列に格納された
24ビット-ビットマップデータをBMPファイルに出力する
[注意] 画像の横幅は４の倍数であること！
*/
void writeBmp(
	unsigned char image[Y_SIZE][X_SIZE][3],
	char *filename                         )
{
	FILE *fp;
	int i, j, k;

	/* ファイルオープン */
	if ( (fp = fopen(filename, "wb")) == NULL ) 
	{
		printf("writeBmp: Open error!\n");
		exit(1);
	}
	printf("output file : %s\n", filename);

	/* ヘッダー情報 */
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

	/* ビットマップデータ */
	for (i=0; i<biHeight; i++)
	{
		for (j=0; j<biWidth; j++)
		{
			for (k=0; k<3; k++)
				fwrite(&image[i][j][2-k], 1, 1, fp);
		}
	}

	fclose(fp);
}

/* 256諧調白黒濃淡画像へ変換 */
void to256BW(
	unsigned char image[Y_SIZE][X_SIZE][3],
	unsigned char image_bw[Y_SIZE][X_SIZE]  )
{
	int y, x, a;
	for (y = 0; y < biHeight; y++)
	{
		for (x = 0; x < biWidth; x++)
		{
			a = 0.3*image[y][x][0] + 0.59*image[y][x][1] + 0.11*image[y][x][2];
			if (a<LOW) a = LOW;
			if (a>HIGH) a = HIGH;
			image_bw[y][x] = a;
		}
	}
}

/* 24ビットBMP形式に変換 */
void to24BMP(
	unsigned char image_bw[Y_SIZE][X_SIZE],
	unsigned char image[Y_SIZE][X_SIZE][3] )
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
void fromTextTo24BMP(
	char *filename,
	unsigned char image[Y_SIZE][X_SIZE][3])
{
	char temp;
	FILE *fp;
	int i, j, k;

	/* ファイルオープン */
	if ( (fp = fopen(filename, "r")) == NULL ) 
	{
		printf("fromTextTo24BMP: Open error!\n");
		exit(1);
	}

	for(i = 0; i < Y_SIZE; i++)
	{
		for(j = 0; j < X_SIZE; j++)
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
}