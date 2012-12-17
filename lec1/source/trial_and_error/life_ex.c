#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define HEIGHT (20)
#define WIDTH (20)

#define OUTPUT_FILE "cells.txt"

#define BUFSIZE (1000)

char cell[HEIGHT][WIDTH];
char cell_next[HEIGHT][WIDTH];

void print_cells(FILE *fp)
{
  int i, j;

  fprintf(fp, "----------\n");

  for (i = 0; i < HEIGHT; i++) {
    for (j = 0; j < WIDTH; j++) {
      char c = (cell[i][j] == 1) ? '#' : ' ';
      fputc(c, fp);
    }
    fputc('\n', fp);
  }
  fflush(fp);

  //sleep(1);
}

void update_cells()
{
  int i, j, k, l;

  for (i = 0; i < HEIGHT; i++) {
    for (j = 0; j < WIDTH; j++) {
      int n = 0;
      for (k = i - 1; k <= i + 1; k++) {
	if (k < 0 || k >= HEIGHT) continue;
	for (l = j - 1; l <= j + 1; l++) {
	  if (k == i && l == j) continue;
	  if (l < 0 || l >= WIDTH) continue;
	  n += cell[k][l];
	}
      }
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

  for (i = 0; i < HEIGHT; i++) {
    for (j = 0; j < WIDTH; j++) {
      cell[i][j] = cell_next[i][j];
    }
  }
}


void find_longest_life_span(char *init_cell, int sum, int N, FILE *fp)
{
  if (N >= 0) {
    init_cell[N] = 0;
    find_longest_life_span(init_cell, sum, N - 1, fp);
    
    if (sum < 10) {
      init_cell[N] = 1;
      find_longest_life_span(init_cell, sum + 1, N - 1, fp);
    }
  }
  else {
    if (sum > 1) {
      memcpy(cell, init_cell, HEIGHT * WIDTH);
      print_cells(fp);
      //find_life_span(fp);
    }
  }
}

int main()
{
  FILE *fp;
  char init_cell[WIDTH * HEIGHT];
 
  if ((fp = fopen(OUTPUT_FILE, "w")) == NULL) {
    fprintf(stderr, "error: cannot open a file.\n");
    return 1;
  }

  find_longest_life_span(init_cell, 0, HEIGHT * WIDTH - 1, fp);

  fclose(fp);
}
