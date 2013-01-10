/**
 * オセロAI
 *  課題2: 対局終了時に勝利者を判定するように
 *  学籍番号: 03-123006
 *  氏名: 岩成達哉
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define is_in(x, y) ((x) >= 0 && (x) < 8 && (y) >= 0 && (y) < 8)

typedef struct _xy
{
    int x;
    int y;
} XY;

XY directions[8] = {
    { -1, -1 }, {  0, -1 }, { 1, -1 },
    { -1,  0 },             { 1,  0 },
    { -1,  1 }, {  0,  1 }, { 1,  1 }
};

int board[8][8];

/* 盤面の初期化をする関数 */
void init_board()
{
    int x, y;
    for (y = 0; y < 8; y++)
        for (x = 0; x < 8; x++)
            board[x][y] = 0;
    
    board[4][3] = board[3][4] = 1;  // black disks
    board[3][3] = board[4][4] = -1;  // white disks
}

/* 盤面を表示する関数 */
void print_board()
{
    int x, y;
    puts("\n abcdefgh");
    for (y = 0; y < 8; y++) {
        printf("%d", y + 1);
        for (x = 0; x < 8; x++) {
            const int d = board[x][y];
            char c = '.';   // empty
            if (d ==  1) c = 'O'; // black disk
            if (d == -1) c = '#'; // white disk
            putchar(c);
        }
        putchar('\n');
    }
    putchar('\n');
}

/* ひっくり返せるかどうかの判定をする関数 */
int can_flip(const int side,
             XY sq, const XY dir) // square, direction
{
    assert(board[sq.x][sq.y] == 0);
    int n = 0;
    do {
        sq.x += dir.x;
        sq.y += dir.y;
        n++;
        if (!is_in(sq.x, sq.y)) return 0;   // 端まで判定が及んだ時
    } while (board[sq.x][sq.y] == -side);
    if (board[sq.x][sq.y] == 0) return 0;   // 対応する場所には置かれてなかったとき
    if (n <= 1) return 0;   // ひっくり返せる数が1つだけだったとき
    return 1;
}

/* 合法手かどうかの判定 */
int is_legal_move(int side, XY sq)
{
    assert(is_in(sq.x, sq.y));
    int i;
    if (board[sq.x][sq.y] != 0) return 0;   // すでにおいてある時は置けない
    for (i = 0; i < 8; i++) {
        if (can_flip(side, sq, directions[i])) return 1;    // ひっくり返せるなら
    }
    return 0;   // 置けない
}

/* ディスクの設置をする関数 */
int place_disk(const int side, const XY sq)
{
    assert(is_legal_move(side, sq));
    int n = 0, i;
    for (i = 0; i < 8; i++) {
        const XY dir = directions[i];
        if (!can_flip(side, sq, dir)) continue; // この方向にはひっくり返せないとき
        int x = sq.x + dir.x;
        int y = sq.y + dir.y;
        // ある方向のディスクをひっくり返す
        while (board[x][y] == -side) {
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
int generate_random_move(const int side, XY *move)
{
    XY legal_moves[60];
    int x, y, nmoves = 0, nempty = 0;
    for (x = 0; x < 8; x++) {
        for (y = 0; y < 8; y++) {
            if (board[x][y] != 0) continue; // すでに置かれているなら次を見る
            nempty++;   // 置かれていない場所をカウント
            XY sq = {x, y}; // 確認用
            if (!is_legal_move(side, sq)) continue; // 合法手でないなら次へ
            assert(nmoves < 60);
            legal_moves[nmoves++] = sq;     // 合法手の代入
        }
    }
    if (nempty == 0) return -1; // 全て埋まっているなら
    if (nmoves == 0) return  0; // 合法手がない
    *move = legal_moves[rand() % nmoves];   // 移動する場所を代入
    return 1;
}

/* 合法手の数を返す関数 */
int get_num_of_legal(const int side)
{
    int x, y, nmoves = 0;
    for (x = 0; x < 8; x++) {
        for (y = 0; y < 8; y++) {
            if (board[x][y] != 0) continue; // すでに置かれているなら次を見る
            XY sq = {x, y}; // 確認用
            if (!is_legal_move(side, sq)) continue; // 合法手でないなら次へ
            nmoves++;   // 合法手のカウント
        }
    }
    return nmoves;
}

/* 結果を表示する関数 */
void show_result(void)
{
    int black = 0, white = 0;
    int x, y;
    for (x = 0; x < 8; x++)
    {
        for (y = 0; y < 8; y++)
        {
            if (board[x][y] == 1)
                black++;
            else if(board[x][y] == -1)
                white++;
        }
    }
    
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
    int is_passed = 0;
    
    init_board();   // 盤面の初期化
    print_board();  // 盤面の表示
    
    int turn;
    for (turn = 1;; turn *= -1) {   // ターンを交代
        XY move;
        
        // 合法手がなければパス
        if (get_num_of_legal(turn) == 0)
        {
            if (is_passed == 1) // どちらも置けなくなったときは終了
                break;
            
            is_passed = 1;  // パスをした
            continue;
        }
        
        if (turn == human_side) {   // 人間側
            while (1) {
                char buf[1000];
                printf("Where? ");
                scanf("%s", buf);
                move.x = buf[0] - 'a';
                move.y = buf[1] - '1';
                
                // ボード外の時
                if (!is_in(move.x, move.y))
                {
                    printf("Out of Board\n");
                    continue;
                }
                
                // 合法手かどうかの判定
                if (is_legal_move(turn, move))
                    break;
                else
                    printf("Illeagal Move\n");
            }
        } else {
            const int r = generate_random_move(turn, &move);    // ランダムな手を生成
            if (r == -1) break;     // no empty square
            if (r ==  0) continue;  // pass (no legal move)
            printf("turn = %d, move = %c%c\n", turn, 'a' + move.x, '1' + move.y);
        }
        place_disk(turn, move); // ディスクの設置
        print_board();          // 盤面の表示
        is_passed = 0;          // パスをしてない
    }
    
    show_result();  // 結果の表示
}
