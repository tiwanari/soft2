/**
* オセロプログラム
* @author Tatsuya Iwanari
* 
* NegaMax法 + αβ枝刈り
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>//memcpy


#define SIDE 8 // 一辺の長さ 
#define ASIDE (SIDE + 2) // 局面用配列の一辺 緩衝地帯分２を足す Side for array, add 2
#define BOARDSIZE (ASIDE * ASIDE) // ボードの大きさ　Size of board
#define UP (-ASIDE)
#define DOWN ASIDE
#define RIGHT 1
#define LEFT (-1)
// Piece number
#define B 1 // BLACK 1
#define W 2 // WHITE 2
#define N 3 // 番兵 Sentinel (out of board) 3
#define BLACK_TURN 0 
#define WHITE_TURN 1

#define MAXDEPTH 8 //探索を行う最大深さ
#define INFINITY 1000 // 十分大きい数を無限大として扱う
#define SEARCH_LIMIT_DEPTH 128 //探索深さの上限


//座標からX座標またはY座標を計算するマクロ
#define X(pos) (pos % ASIDE)
#define Y(pos) (pos / ASIDE)
#define TURNCOLOR( turn ) (turn + 1)
#define OPPONENT(turn) !turn

#define TRUE 1
#define FALSE 0

#define PASSMOVE -1 //パス手には-1を入れる 
#define MOVENUM 32

// αβ枝刈り
#define ALPHA -2000
#define BETA  2000

struct Position //過去の局面などを記憶するための構造体
{
	unsigned char board[BOARDSIZE];
	unsigned char stonenum[2];
};
struct Position history[SEARCH_LIMIT_DEPTH];


// 表示に使う文字定数
const char* piece[3] = { "  ", " ●", " ○" };
const char* abc[8] = { "a","b","c","d","e","f","g","h"};
int turn; // 手番
int ply; // 手数 

typedef char Move;//手だとわかりやすくするため型を宣言（しなくてもよい）
Move nextmove;//次の手 
unsigned char stonenum[2];//石の数を記憶する変数

//2次元の座標を一次元に変換
int getposition( int x, int y ){ return y * ASIDE + x; };

//一番大事な変数、ここでは一次元で表現
unsigned char board[BOARDSIZE] = //intでも良いが、データ格納のことを考えてcharにしている
{
	N, N, N, N, N, N, N, N, N, N, 
	N, 0, 0, 0, 0, 0, 0, 0, 0, N,
	N, 0, 0, 0, 0, 0, 0, 0, 0, N,
	N, 0, 0, 0, 0, 0, 0, 0, 0, N,
	N, 0, 0, 0, B, W, 0, 0, 0, N,
	N, 0, 0, 0, W, B, 0, 0, 0, N,
	N, 0, 0, 0, 0, 0, 0, 0, 0, N,
	N, 0, 0, 0, 0, 0, 0, 0, 0, N,
	N, 0, 0, 0, 0, 0, 0, 0, 0, N,
	N, N, N, N, N, N, N, N, N, N 
};


//上下左右一列は使わないのですべて0のまま
int evalboard[BOARDSIZE] =
{
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0, 100, -40,  20,   5,   5,  20, -40, 100,   0,
   0, -40, -80,  -1,  -1,  -1,  -1, -80, -40,   0,
   0,  20,  -1,   5,   1,   1,   5,  -1,  20,   0,
   0,   5,  -1,   1,   0,   0,   1,  -1,   5,   0,
   0,   5,  -1,   1,   0,   0,   1,  -1,   5,   0,
   0,  20,  -1,   5,   1,   1,   5,  -1,  20,   0,
   0, -40, -80,  -1,  -1,  -1,  -1, -80, -40,   0,
   0, 100, -40,  20,   5,   5,  20, -40, 100,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

// ゲーム終了局面の評価値を返す 勝ち ∞ 引き分け 0 負け −∞
int getTerminalValue()
{
	int diff = stonenum[BLACK_TURN] - stonenum[WHITE_TURN];
	if ( diff > 0 ) // Win
		return INFINITY;
	else if ( diff == 0 ) // DRAW
		return 0;
	else // if ( diff < 0 ) // LOSE
		return -INFINITY;
}

// 合法手かどうか判定する関数 
// 合法手ならTRUE、違ったらFALSEを返す
int isLegalMove(Move pos)
{
	int dirx, diry, dir;
	int pos1;
	// 自分の色、相手の色は何か変数に入れておく
	int color = TURNCOLOR( turn );
	int opponentcolor = TURNCOLOR( OPPONENT(turn));

	//空きマスでないかCheck
	if ( board[pos] )
		return FALSE;
	
	// posの周り8方向を調べ相手石が取れるか調べる
	for ( dirx = -1; dirx <= 1; dirx++ )
	{
		for ( diry = -ASIDE; diry <= ASIDE; diry += ASIDE )
		{
			dir = dirx + diry;
			if ( dir == 0 )//方向０は意味ないのでパス
				continue;
			pos1 = pos + dir;//posの隣のマス
			//以下は board[pos1] の中身をチェックしながら、
			//pos1を +=dirしていく
			//だめなら continue

			//まず相手の石があるか調べる
			if( board[pos1] != opponentcolor )	
				continue;
			do // 相手の石がある間は次を調べる
			{
				pos1 += dir;
			}while ( board[pos1] == opponentcolor );
			// ひっくり返すためには最後に自分の石がないといけない
			if( board[pos1] != color )
				continue;

			//最後まで来たら成功
			return TRUE;
		}
	}
	return FALSE;	
}

//手の生成 generation of moves
//生成した手の数を返している 
int generateMoves( Move moves[] )	
{
	int num = 0;//生成する合法手の数
	int pos;
	// 左上から順に石を置き、合法手か調べる
	for ( pos = 1; pos < BOARDSIZE; pos++ )
	{
		if ( isLegalMove(pos) )
			moves[num++] = pos;//num番目の配列にposを入れる
	}
	return num;
}

// 評価関数の計算　Calculation of evaluation function
int getEvaluationValue()
{
	int pos, value, c;
	Move moves[MOVENUM];

	// 合法手数の差を評価関数とする(自由度)
	value = generateMoves(moves);
	//手番変更
	turn = OPPONENT(turn);//0→1、1→0となる
	value -= generateMoves(moves);
	//手番戻す
	turn = OPPONENT(turn);//0→1、1→0となる

	value *= 30;//自由度１を30点としておく（適当）
	//evalboardを使い、場所による得点計算
	for ( pos = 0; pos < BOARDSIZE; pos++ )
	{
		c = board[pos];
		if ( c == 0 )
			continue;
		else if ( c == TURNCOLOR(turn) )
			value += evalboard[pos];
		else
			value -= evalboard[pos];
	}
	return value;
}


// 表示関数 display function
void output()
{
	int x, y;
	printf( "  a  b  c  d  e  f  g  h \n" );
		
	for ( y = 1; y <= SIDE; y++ )
	{
		printf("%d|", y);
		for ( x = 1; x <= SIDE; x++ )
		{
			printf( "%s|", piece[board[getposition(x,y)]] );
		}
		printf("\n");
	}
}

// ゲーム終了局面ならTRUEを返す Return TRUE in case of GAME OVER
// 生成した手が０の時呼ばれる
int isTerminalNode()
{
	int num;
	Move moves[MOVENUM];
	//手番変更
	turn = OPPONENT(turn);//0→1、1→0となる
	//手を生成して数を見る
	num = generateMoves(moves);
	//手番戻す
	turn = OPPONENT(turn);//0→1、1→0となる
	
	if ( num == 0 )
		return TRUE;
	return FALSE;
}


// 実際に手を進める
void makeMove( Move pos , int depth )
{
	int dirx, diry, dir;
	int pos1;
	int change = 0;
	int isLegal = TRUE;

	// 自分の色、相手の色は何か変数に入れておく
	int color = TURNCOLOR( turn );
	int opponentcolor = TURNCOLOR( OPPONENT(turn));
	
	// 局面を配列historyに保存
	memcpy( history[depth].board, board, sizeof( board ));// 配列の中身をコピーするのはmemcpy()を使うと簡単
	memcpy( history[depth].stonenum, stonenum, sizeof( stonenum ));

	if( pos == PASSMOVE )
	{
		turn = OPPONENT(turn);
		return ;
	}
	board[pos] = color;

	// posの周り8方向を調べ相手石が取れるか調べる
	for ( dirx = -1; dirx <= 1; dirx++ )
	{
		for ( diry = -ASIDE; diry <= ASIDE; diry += ASIDE )
		{
			dir = dirx + diry;
			if ( dir == 0 )//方向０は意味ないのでパス
				continue;
			pos1 = pos + dir;//posの隣のマス
			//以下は board[pos1] の中身をチェックしながら、
			//pos1を +=dirしていく
			//だめなら continue

			//まず相手の石があるか調べる
			if( board[pos1] != opponentcolor )	
				continue;
			do // 相手の石がある間は次を調べる
			{
				pos1 += dir;
			}while ( board[pos1] == opponentcolor );
			// ひっくり返すためには最後に自分の石がないといけない
			if( board[pos1] != color )
				continue;

			// 最後まで来たら成功
			pos1 -= dir;
			// ひっくり返す
			do
			{
				board[pos1] = color;
				pos1 -= dir;
				change++;
			}while( pos1 != pos );
		}
	}

	// 最後に石の数をグローバル変数stonenum[]に足す（もしくは引く）
	stonenum[turn] += change + 1;	// 置いた石も足す
	stonenum[OPPONENT(turn)] -= change;
	turn = OPPONENT(turn);
}


// 手を戻す
void unmakeMove(int depth)
{
	// 配列の中身をコピーするのはmemcpy()を使うと簡単
	// 逆の事をmakeMove()でしないといけない
	// 局面を配列historyから復元
	memcpy( board, history[depth].board, sizeof( board ));
	memcpy( stonenum, history[depth].stonenum, sizeof( stonenum ));
	turn = OPPONENT(turn);
}

// negamax関数　negamax function + αβ枝刈り
int search(int depth, int al, int be)
{
	int i;
	int movenum;//手の数
	Move moves[MOVENUM];//手を入れる配列 an array of moves
	int value;
	int bestvalue = -INFINITY;

	if ( depth >= MAXDEPTH )// leaf node
		return getEvaluationValue();

	//手を生成
	movenum = generateMoves( moves );
	if ( movenum == 0 )
	{
		if ( isTerminalNode() )// Game Over
			return getTerminalValue();
		else 
			moves[movenum++] = PASSMOVE;
	}

	for ( i = 0; i < movenum; i++ )
	{
		makeMove( moves[i], depth );//一手進め
		value = - search( depth + 1, -be, -al );// 再帰(recursive)
		
		/* for debug
		if(depth == 0)
		{
			output();
			printf("i = %d, value = %d, move = %d\n", i, value, moves[i]);//for Debug
		}
		*/
		unmakeMove( depth );//一手戻る
		
		// 最高値
		if ( value > al )
		{
			al = value;
			if ( depth == 0 )
				nextmove = moves[i];
		}

		// αβ枝刈り
		if( value >= be ) break;
	}
	return al;
}


//人間の入力を管理する関数
void manPlayer()
{
	//入力をさせ、合法手かチェック
	char line[256];
	int x, y, num, move;

	//パスのチェック　関数generateMovesが完成したらコメントアウトを外す
	Move moves[MOVENUM];
	num = generateMoves( moves );

	// 合法手が無い場合
	if(num == 0)
	{
		// パスであることを表示する
		// 何か入力させる（Enter押すだけなど）
		printf("Pass!\n");
		printf("Press Enter!\n");
		gets(line);
		nextmove = PASSMOVE;
		return;
	}

	do
	{
		// input X coordinate
		printf("BLACK\n");  
		do
		{
			printf("x(a-h):");
			gets(line);
		}while(line[0] < 'a' || line[0] > 'h');
		x = line[0] - 'a' + 1;
		// input Y coordinate
		do
		{
			printf("y(1-8):");
			gets(line);
		}while(line[0] < '1' || line[0] > '8');
		y = line[0] - '1' + 1;

		// Check legal move
		move = getposition( x, y );
		if( isLegalMove(move) )
		{
			nextmove = move;
			return;
		}
		printf("Illegal Move!\n\n");
	}while(1);
}


//COMの手を生成する関数
void comPlayer()
{
	int value;
	printf( "Com Thinking...\n");
	value = search(0, ALPHA, BETA);
		
	// printf( "value = %d\n", value );
	if ( value == INFINITY )
		printf( "Computer Finds Win!!\n" );
}

//グローバル変数などを初期化
void init()
{
	turn = WHITE_TURN;
	ply = 0;
	stonenum[BLACK_TURN] = 2;
	stonenum[WHITE_TURN] = 2;
}

int main()
{
	//まずは変数宣言、初期化など
	int result;
	int num;
	Move moves[MOVENUM];
	int manturn = BLACK_TURN;
	char line[256];

	init();
	output();//局面の表示
	
	while(1)//一回の対局が終わるまでループ
	{
		num = generateMoves( moves );
		if ( num == 0 && isTerminalNode())//終局かチェック
		{
			//石の数で勝ち負け判定し表示
			result = stonenum[BLACK_TURN] - stonenum[WHITE_TURN];
			// result により表示を変える
			if ( result == 0 )
				printf ( "GAMEOVER!  DRAW!!\n");
			else 
				printf ( "GAMEOVER! %s WIN!!\n", ( result > 0 ? "BLACK": "WHITE"));
			printf("BLACK:%d WHITE:%d\n", stonenum[BLACK_TURN], stonenum[WHITE_TURN]);
			printf ( "Press ENTER\n");
			gets( line );
			return 0;
		}
		
		if ( turn == manturn )
			manPlayer();
		else
			comPlayer();
		
		ply++;

		// パスの処理
		if( nextmove != PASSMOVE )
		{
			makeMove( nextmove , 0);
			output();
			printf( "\nply = %d\n", ply );
			printf ( "%s -> %s%d\n\n", ( turn != BLACK_TURN? "BLACK": "WHITE"), abc[X(nextmove)-1], Y(nextmove));
		}
		else
			turn = OPPONENT(turn);	// ターンの更新
		
	}
	return 0;
}