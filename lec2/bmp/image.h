/* 最大画像サイズ */
#define Y_SIZE 256
#define X_SIZE 256

/* BMPファイル用 */
typedef unsigned short WORD;	// 2byte
typedef unsigned long DWORD;	// 4byte

WORD  bfType;
DWORD bfSize;
WORD  bfReserved1,
      bfReserved2;
DWORD bfOffBits;
DWORD biSize, 
      biWidth, biHeight;
WORD  biPlanes,
      biBitCount;
DWORD biCompression, 
      biSizeImage,
      biXPelsPerMeter,
      biYPelsPerMeter,
      biClrUsed, 
      biClrImportant;

#define HIGH   255	/* 2値画像のhigh level */
#define LOW      0	/* 2値画像のlow level */
#define LEVEL  256	/* 濃度レベル数 */
