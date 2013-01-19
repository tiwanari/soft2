/**
 * 学籍番号: 03-123006
 * 氏名: 岩成達哉
 *  オセロAI
 *      課題4: 拡張機能の実装
 *          ・αβ枝刈りの実装
 *          ・Negamax法の実装
 *          ・COMのレベル(探索の深さ)の指定を可能に
 *          ・ハンデの指定を可能に
 *          ・代わりに打つ機能を追加
 *          ・後半は探索を深めに
 *          ・評価関数は打てる場所の数と評価ボードをもとに算出
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define is_in(x, y) (x >= 0 && x < 8 && y >= 0 && y < 8)

#define TRUE 1
#define FALSE 0

#define DEPTH_LIMIT 7   // 探索の深さ
#define DEPTH_INFINIT 64    // 終わりまで読むとき
#define INFINITY 100000 // 十分大きい値を無限大とする

#define MOVENUM 60      // 移動方法の最大

#define BLACK 1         // 黒石
#define WHITE -1        // 白石

// αβ枝刈り
#define ALPHA   -200000
#define BETA     200000

/* 盤面の位置を表す構造体の宣言 */
typedef struct _xy
{
    int x;
    int y;
} XY;
const XY PASSMOVE = {-1, -1};   // パスを表す

// ひっくり返せるか確認する際の方向を表す構造体
XY directions[8] =
{
    { -1, -1 }, {  0, -1 }, {  1, -1 },
    { -1,  0 },             {  1,  0 },
    { -1,  1 }, {  0,  1 }, {  1,  1 }
};

char board[8][8];    // 盤面(charで十分)

// 評価ボード
int eval_board[8][8] =
{
    {100, -40,  20,   5,   5,  20, -40, 100,},
    {-40, -80,  -1,  -1,  -1,  -1, -80, -40,},
    { 20,  -1,   5,   1,   1,   5,  -1,  20,},
    {  5,  -1,   1,   0,   0,   1,  -1,   5,},
    {  5,  -1,   1,   0,   0,   1,  -1,   5,},
    { 20,  -1,   5,   1,   1,   5,  -1,  20,},
    {-40, -80,  -1,  -1,  -1,  -1, -80, -40,},
    {100, -40,  20,   5,   5,  20, -40, 100,},
};

int turn;   // 順番を表す
int max_depth;  // 探索の深さ
int ply = 0;    // 手の数

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
            const char d = board[x][y];
            if (d == BLACK)
                printf("X|");
            else if (d == WHITE)
                printf("O|");
            else
                printf(" |");
        }
		printf("\n");
	}
    printf("\n");
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
    int n = 0, i;
    
    // パスのとき
    if (sq.x == PASSMOVE.x || sq.y == PASSMOVE.y)
        return 0;
    
    assert(is_legal_move(side, sq));
    
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

/* ゲーム終了局面の評価値を返す関数(勝ち:∞, 引き分け:0, 負け:−∞) */
int get_terminal_value(int side)
{
	int mine, opp;
    
    // 数をカウント(どちらの番かで返す値を変える)
    if ((turn == BLACK && turn == side)
        || (turn == WHITE && turn == -side))
        count_disk(&mine, &opp);
    else
        count_disk(&opp, &mine);
    
	if (mine > opp) // Win
		return INFINITY;
	else if (mine == opp) // DRAW
		return 0;
	else // if (diff < 0) // LOSE
		return -INFINITY;
}

/* 評価関数 */
int eval_func(int side)
{
    int value;
    XY moves[MOVENUM];
    int x, y;
    
    // 合法手数(自由度)の差を評価関数とする
    value = generate_moves(side, moves);
    value -= generate_moves(-side, moves);
    
    // 自由度1につき30としておく
    value *= 30;
    
    for (x = 0; x < 8; x++)
    {
        for (y = 0; y < 8; y++)
        {
            const char d = board[x][y];
            if (d == side)
                value += eval_board[x][y];
            else if (d == -side)
                value -= eval_board[x][y];
        }
    }
    
    return value;
}

/* Negamax法 + αβ枝刈りで手を生成する */
int alpha_beta(int depth, int side, XY *move, int al, int be)
{
    int value;
    int nmoves;
    XY moves[MOVENUM], opp[MOVENUM];
    char pre_board[8][8];
    int i;
    
    if (depth == max_depth)
        return eval_func(side);
    
    // 手を生成
	nmoves = generate_moves(side, moves);
    
    // 手がないとき
	if (nmoves == 0)
	{
		if (generate_moves(-side, opp) == 0) // 相手もおけないときは終了
			return get_terminal_value(side);
		else    // 違うときはパス
			moves[nmoves++] = PASSMOVE;
	}
    
    // たどっていく
    for (i = 0; i < nmoves; i++)
	{
        memcpy(pre_board, board, sizeof(board));    // 盤面の保存
		place_disk(side, moves[i]);   // 一手進める
        
        // 再帰(recursive)
		value = -alpha_beta(depth + 1, -side, move, -be, -al);
		
        memcpy(board, pre_board, sizeof(board));    // 戻す
        
        // 値の更新
        if (value > al)
        {
            al = value;
            if (depth == 0)
                *move = moves[i];
        }
        
		// αβ枝刈り
		if(value >= be) break;
	}
    
	return al;
}

/* COMの手を生成する関数 */
void com_player1(const int side, XY *move)
{
    XY moves[MOVENUM];
    int value;
    int tmp;
	printf( "Com Thinking...\n");
    
    // 手がなければパス
    if (generate_moves(side, moves) == 0)
    {
        printf("Pass!\n\n");
        *move = PASSMOVE;
        return ;
    }
    
    tmp = max_depth;
    // 読む数を変える
    if(ply > 50)   // 終盤はたくさん読む
        max_depth += 4;
    else if(ply > 40)
        max_depth += 3;
    else if(ply > 30)
        max_depth += 2;
    
    value = alpha_beta(0, side, move, ALPHA, BETA); // Alpha-Beta法で手を生成
    
    max_depth = tmp;    // もとに戻す
    
    //printf("value = %d\n", value);
    if (value == INFINITY)  // 勝ち
        printf("COM Finds Win!\n");
    else if (value == -INFINITY)
        printf("COM Finds Lose...\n");
}

/* COMの手を生成する関数 */
void com_player2(const int side, XY *move)
{
    XY moves[MOVENUM];
    int value;
	printf( "Com Thinking...\n");
    
    // 手がなければパス
    if (generate_moves(side, moves) == 0)
    {
        printf("Pass!\n\n");
        *move = PASSMOVE;
        return ;
    }
    
    value = alpha_beta(0, side, move, ALPHA, BETA); // Alpha-Beta法で手を生成
    
    //printf("value = %d\n", value);
    if (value == INFINITY)  // 勝ち
        printf("COM Finds Win!\n");
    else if (value == -INFINITY)
        printf("COM Finds Lose...\n");
}


/* ランダムに手を生成する関数 */
void randam_player(const int side, XY *move)
{
    XY moves[MOVENUM];
    int nmoves;
    
	printf( "Com Thinking...\n");
    
    nmoves = generate_moves(side, moves);
    
    // 手がなければパス
    if (nmoves == 0)
    {
        printf("Pass!\n\n");
        *move = PASSMOVE;
        return ;
    }
    *move = moves[rand() % nmoves];
}

/* 人間の入力を管理する関数 */
void man_player(const int side, XY *move)
{
    XY moves[MOVENUM];
    char buf[1000];
    int tmp;
    
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
            printf("Where? (\"help\": Com will help you) ");
            fgets(buf, sizeof(buf), stdin);
            
            // 代わりに打つ
            if (strstr(buf, "help") != NULL)
            {
                tmp = max_depth;            // 一時退避
                max_depth = DEPTH_LIMIT;    // 最良の手を出すため
                alpha_beta(0, side, move, ALPHA, BETA);
                max_depth = tmp;    // もとに戻す
                
                return ;
            }
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

/* 結果を表示する関数 */
void show_result(void)
{
    int black, white;
    
    count_disk(&black, &white); // 数える
    
    printf("Result : Black %d, White %d\n", black, white);
    if (black == white)
        printf("Draw!!\n");
    else if (black > white)
        printf("Black Win!!\n");
    else
        printf("White Win!!\n");
}

/* ハンデを付ける関数 */
void give_handicap(int handi, int hum)
{
    if (handi <  0)  board[0][0] = -hum;
    if (handi < -1)  board[0][7] = -hum;
    if (handi < -2)  board[7][0] = -hum;
    if (handi < -3)  board[7][7] = -hum;
    
    if (handi > 0)  board[0][0] = hum;
    if (handi > 1)  board[0][7] = hum;
    if (handi > 2)  board[7][0] = hum;
    if (handi > 3)  board[7][7] = hum;
}

int main(int argc, char **argv)
{
    const int human_side = (argc >= 2) ? atoi(argv[1]) : 0; // 先攻と後攻の選択
    XY moves[MOVENUM];
    int handi;
    
    init_board();   // 盤面の初期化
    print_board();  // 盤面の表示
    max_depth = DEPTH_LIMIT;    // 探索の深さ
    
    srand((unsigned)time(NULL)); // 乱数列の初期化
    
    if (human_side == 1 || human_side == -1)    // 人が参加するとき
    {
        printf("Choose Com Level(1 ~ %d): ", DEPTH_LIMIT);
        scanf("%d", &max_depth);
        
        // 丸める
        if (max_depth < 1)              max_depth = 1;
        if (max_depth > DEPTH_LIMIT)    max_depth = DEPTH_LIMIT;
        printf("Level %d\n\n", max_depth);
        
        
        printf("Do you want a handicap?\n");
        printf("handi(-4 ~ 4): ");
        scanf("%d", &handi);
        give_handicap(handi, human_side);
        print_board();  // 盤面の表示
        getchar();    // 改行文字を削っておく
    }
        
    for (turn = 1;; turn *= -1)   // ターンを交代
    {
        XY nextmove;
        
        // どちらも置けなくなったときは終了
        if (generate_moves(turn, moves) == 0
            && generate_moves(-turn, moves) == 0)
            break;
        
        
        if (turn == human_side)
            man_player(turn, &nextmove);    // 人間の番
        else
            com_player1(turn, &nextmove);    // COMの番
        
        // パスかどうかの判定
        if (nextmove.x != PASSMOVE.x && nextmove.y != PASSMOVE.y)
        {
            printf("%s -> %c%c\n\n",
                   (turn == BLACK ? "BLACK" : "WHITE"),
                   'a' + nextmove.x, '1' + nextmove.y);
            place_disk(turn, nextmove); // ディスクの設置
            print_board();          // 盤面の表示
            ply++;  // 手数をカウント
        }
    }
    show_result();  // 結果の表示
    return 0;
}