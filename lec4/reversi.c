#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

struct xy
{
  int x;
  int y;
};

struct xy directions[8] = {
  { -1, -1 }, {  0, -1 }, { 1, -1 }, 
  { -1,  0 },             { 1,  0 }, 
  { -1,  1 }, {  0,  1 }, { 1,  1 } 
};

int board[8][8];

void init_board()
{
  int x, y;
  for (y = 0; y < 8; y++) 
    for (x = 0; x < 8; x++)
      board[x][y] = 0;

  board[4][3] = board[3][4] =  1;  // black disks
  board[3][3] = board[4][4] = -1;  // white disks
}

void print_board()
{
  int x, y;
  puts("\n abcdefgh");
  for (y = 0; y < 8; y++) {
    printf("%d", y + 1);
    for (x = 0; x < 8; x++) {
      const int d = board[x][y];
      char c = '.';
      if (d ==  1) c = 'O'; // black disk
      if (d == -1) c = '#'; // white disk
      putchar(c);
    }
    putchar('\n');
  }
  putchar('\n');
}

int can_flip(const int side, 
	     struct xy sq, const struct xy dir) // square, direction
{
  assert(board[sq.x][sq.y] == 0);
  int n = 0;
  do {
    sq.x += dir.x; 
    sq.y += dir.y; 
    n++;
    if (sq.x < 0 || sq.x >= 8 || sq.y < 0 || sq.y >= 8) return 0;
  } while (board[sq.x][sq.y] == -side);
  if (board[sq.x][sq.y] == 0) return 0;
  if (n <= 1) return 0;
  return 1;
}

int is_legal_move(int side, struct xy sq)
{
  assert(sq.x >= 0 && sq.x < 8 && sq.y >= 0 && sq.y < 8);
  int i;
  if (board[sq.x][sq.y] != 0) return 0;
  for (i = 0; i < 8; i++) {
    if (can_flip(side, sq, directions[i])) return 1;
  }
  return 0;
}

int place_disk(const int side, const struct xy sq)
{
  assert(is_legal_move(side, sq));
  int n = 0, i;
  for (i = 0; i < 8; i++) {
    const struct xy dir = directions[i];
    if (!can_flip(side, sq, dir)) continue;
    int x = sq.x + dir.x;
    int y = sq.y + dir.y;
    while (board[x][y] == -side) {
      board[x][y] = side;
      n++;
      x += dir.x;
      y += dir.y;
    }
  }
  board[sq.x][sq.y] = side;
  assert(n > 0);
  return n;
}

int generate_random_move(const int side, struct xy *move)
{
  struct xy legal_moves[60];
  int x, y, nmoves = 0, nempty = 0;
  for (x = 0; x < 8; x++) {
    for (y = 0; y < 8; y++) {
      if (board[x][y] != 0) continue;
      nempty++;
      struct xy sq;
      sq.x = x; sq.y = y;
      if (!is_legal_move(side, sq)) continue;
      assert(nmoves < 60);
      legal_moves[nmoves++] = sq;
    }
  }
  if (nempty == 0) return -1;
  if (nmoves == 0) return  0;
  *move = legal_moves[rand() % nmoves];
  return 1;
}

int main(int argc, char **argv)
{
  const int human_side = (argc >= 2) ? atoi(argv[1]) : 0;

  init_board();
  print_board();

  int turn;
  for (turn = 1;; turn *= -1) {
    struct xy move;
    if (turn == human_side) {
      do {
	char buf[1000];
	printf("Where? ");
	scanf("%s", buf);
	move.x = buf[0] - 'a';
	move.y = buf[1] - '1';
      } while (!is_legal_move(turn, move));
    } else {
      const int r = generate_random_move(turn, &move);
      if (r == -1) break;     // no empty square
      if (r ==  0) continue;  // pass (no legal move)
      printf("turn = %d, move = %c%c\n", turn, 'a' + move.x, '1' + move.y);
    }
    place_disk(turn, move);
    print_board();
  }
}
