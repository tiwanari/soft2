/**
 * 学籍番号: 03-123006
 * 氏名: 岩成達哉
 *  オセロAI
 *      課題2: 対局終了時に勝利者を判定するように
 *          ll.256-269に結果を表示する関数show_result()をつけて
 *          l.308で表示するようにしています
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define is_in(x, y) ((x) >= 0 && (x) < 8 && (y) >= 0 && (y) < 8)
#define TRUE 1
#define FALSE 0

#define MOVENUM 60

#define BLACK 1
#define WHITE -1

typedef struct _xy
{
    int x;
    int y;
} XY;
const XY PASSMOVE = {-1, -1};   // パスを表す

XY directions[8] = {
    { -1, -1 }, {  0, -1 }, {  1, -1 },
    { -1,  0 },             {  1,  0 },
    { -1,  1 }, {  0,  1 }, {  1,  1 }
};

int board[8][8];

/* 盤面の初期化をする関数 */
void init_board()
{
    int x, y;
    for (y = 0; y < 8; y++)
        for (x = 0; x < 8; x++)
            board[x][y] = 0;
    
    board[4][3] = board[3][4] = BLACK;
    board[3][3] = board[4][4] = WHITE;
}

/* 盤面を表示する関数 */
void print_board()
{
    int x, y;
	printf( "  a b c d e f g h \n" );
    
	for (y = 0; y < 8; y++)
	{
		printf("%d|", y+1);
		for (x = 0; x < 8; x++)
        {
            if (board[x][y] == BLACK)
                printf("X|");
            else if (board[x][y] == WHITE)
                printf("O|");
            else
                printf(" |");
                
        }
		printf("\n");
	}
    printf("\n");
    
}

/* ひっくり返せるかどうかの判定をする関数 */
int can_flip(const int side,
             XY sq, const XY dir) // square, direction
{
    assert(board[sq.x][sq.y] == 0);
    int n = 0;
    do
    {
        sq.x += dir.x;
        sq.y += dir.y;
        n++;
        if (!is_in(sq.x, sq.y)) // 端まで判定が及んだ時
            return FALSE;
    } while (board[sq.x][sq.y] == -side);
    
    if (board[sq.x][sq.y] == 0) // 対応する場所には置かれてなかったとき
        return FALSE;
    
    if (n <= 1) // ひっくり返せる数が最初の1つだけだったとき
        return FALSE;
    
    return TRUE;
}

/* 合法手かどうかの判定 */
int is_legal_move(const int side, const XY sq)
{
    assert(is_in(sq.x, sq.y));
    int i;
    
    if (board[sq.x][sq.y] != 0)// すでにおいてある時は置けない
        return FALSE;
    
    for (i = 0; i < 8; i++)
    {
        if (can_flip(side, sq, directions[i]))// ひっくり返せるなら
            return TRUE;
    }
    return FALSE;   // 置けない
}

/* 合法手を作る関数 */
int generate_moves(const int side, XY *moves)
{
    int nmoves = 0;
    int x, y;
    XY tmp;
    
    for (x = 0; x < 8; x++)
    {
        for (y = 0; y < 8; y++)
        {
            XY tmp = {x, y};    // 検査用
            
            if (!is_legal_move(side, tmp)) // 合法手でないなら次へ
                continue;
            
            assert(nmoves < 60);
            moves[nmoves++] = tmp;     // 合法手の代入
        }
    }
    
    return nmoves;
}

/* ディスクの設置をする関数 */
int place_disk(const int side, const XY sq)
{
    assert(is_legal_move(side, sq));
    int n = 0, i;
    for (i = 0; i < 8; i++)
    {
        const XY dir = directions[i];
        
        if (!can_flip(side, sq, dir))// この方向にはひっくり返せないとき
            continue;
        
        int x = sq.x + dir.x;
        int y = sq.y + dir.y;
        
        // ある方向のディスクをひっくり返す
        while (board[x][y] == -side)
        {
            board[x][y] = side;
            n++;
            x += dir.x;
            y += dir.y;
        }
    }
    board[sq.x][sq.y] = side;   // 最後に端をひっくり返す
    assert(n > 0);
    return n;
}

/* ランダムに配置 */
void generate_random_move(const int side, XY *move)
{
    XY legal_moves[60];
    int x, y;
    int nmoves;

    nmoves = generate_moves(side, legal_moves); // 合法手を作る
    if (nmoves == 0)    // パス
	{
        printf("Pass!\n");
        *move = PASSMOVE;
        return ;
	}
    
    *move = legal_moves[rand() % nmoves];   // 移動する場所を代入
    return ;
}

/* 人間の入力を管理する関数 */
void man_player(const int side, XY *move)
{
    XY moves[MOVENUM];
    char buf[1000];
    
    // 手がなければパス
    if (generate_moves(side, moves) == 0)
    {
        printf("Pass!\n");
		printf("Press Enter!\n");
		fgets(buf, sizeof(buf), stdin);
        *move = PASSMOVE;
        return ;
    }
    
    while (1)
    {
        // 手の入力
        do
		{
            printf("Where? ");
            fgets(buf, sizeof(buf), stdin);
		} while(strlen(buf) < 1
                || buf[0] < 'a' || buf[0] > 'h'
                || buf[1] < '1' || buf[1] > '8');
        
        move->x = buf[0] - 'a';
        move->y = buf[1] - '1';
        
        
        // 合法手かどうかの判定
        if (is_legal_move(side, *move))
            break;
        else
            printf("Illeagal Move\n\n");
    }
}

/* COMの手を生成する関数 */
void com_player(const int side, XY *move)
{
	printf( "Com Thinking...\n");
    generate_random_move(side, move);    // ランダムに手を生成
}

/* ディスクの数をカウントする関数 */
void count_disk(int *black, int *white)
{
    int x, y;
    
    *black = *white = 0;
    
    for (x = 0; x < 8; x++)
    {
        for (y = 0; y < 8; y++)
        {
            if (board[x][y] == BLACK)
                (*black)++;
            else if(board[x][y] == WHITE)
                (*white)++;
        }
    }
}

/* 結果を表示する関数 */
void show_result(void)
{
    int black, white;
    
    count_disk(&black, &white);
    
    printf("Result : Black %d, White %d\n", black, white);
    if (black == white)
        printf("Draw!!\n");
    else if (black > white)
        printf("Black Win!!\n");
    else
        printf("White Win!!\n");
}

int main(int argc, char **argv)
{
    const int human_side = (argc >= 2) ? atoi(argv[1]) : 0; // 先攻と後攻の選択
    XY moves[MOVENUM];
    
    init_board();   // 盤面の初期化
    print_board();  // 盤面の表示
    
    srand((unsigned)time(NULL)); // 乱数列の初期化
    
    int turn;
    
    for (turn = 1;; turn *= -1)   // ターンを交代
    {
        XY nextmove;
        
        // どちらも置けなくなったときは終了
        if (generate_moves(turn, moves) == 0
            && generate_moves(-turn, moves) == 0)
            break;
        
        
        if (turn == human_side)
            man_player(turn, &nextmove);
        else
            com_player(turn, &nextmove);
        
        // パスかどうかの判定
        if (nextmove.x != PASSMOVE.x && nextmove.y != PASSMOVE.y)
        {
            printf("%s -> %c%c\n\n",
                   (turn == BLACK ? "BLACK" : "WHITE"),
                   'a' + nextmove.x, '1' + nextmove.y);
            place_disk(turn, nextmove); // ディスクの設置
            print_board();          // 盤面の表示
        }
    }
    show_result();  // 結果の表示
    return 0;
}