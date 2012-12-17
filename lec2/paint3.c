#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIDTH 70
#define HEIGHT 40
#define BUFSIZE 1000
#define COM_OTHER 0
#define COM_QUIT 1
#define HISTORY_SIZE 100


char canvas[WIDTH][HEIGHT];
char *history[HISTORY_SIZE];
int g_history_n = 0;

void print_canvas(FILE *fp)
{
    int x, y;
    
    fprintf(fp, "----------\n");
    
    for (y = 0; y < HEIGHT; y++) {
        for (x = 0; x < WIDTH; x++) {
            char c = canvas[x][y];
            fputc(c, fp);
        }
        fputc('\n', fp);
    }
    fflush(fp);
}

int init_canvas()
{
    memset(canvas, ' ', sizeof(canvas));
}

void draw_line(int param[])
{
    int x0 = param[0], y0 = param[1], x1 = param[2], y1 = param[3];
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int num = (dx >= dy) ? dx : dy;
    double delta = (num != 0) ? 1.0 / num : 0;
    int i;
    
    for (i = 0; i <= num; i++) {
        int x = x0 + delta * i * (x1 - x0);
        int y = y0 + delta * i * (y1 - y0);
        
        if (0 <= x && x < WIDTH && 0 <= y && y < HEIGHT) {
            canvas[x][y] = '#';
        }
        
    }
}

void draw_circle(int param[])
{
    int x0 = param[0], y0 = param[1], r = param[2];
    int n = ceil(2 * M_PI * r);
    int i;
    
    if (r == 0)
        return ;
    
    for (i = 1; i < n; i++) {
        int x = x0 + r * cos((double)i / r);
        int y = y0 + r * sin((double)i / r);
        
        if (0 <= x && x < WIDTH && 0 <= y && y < HEIGHT) {
            canvas[x][y] = '#';
        }
    }
    
}

void draw_rectangle(int param[])
{
    int x0 = param[0], y0 = param[1], x1 = param[2], y1 = param[3];
    int point[4][4] = {
        {x0, y0, x0, y1},
        {x0, y1, x1, y1},
        {x1, y1, x1, y0},
        {x1, y0, x0, y0},
    };
    int i;
    
    for (i = 0; i < 4; i++) {
        draw_line(point[i]);
    }
}


void exe_command(void (*func)(int []), int n)
{
    int *param = (int *)malloc(sizeof(int) * n);
    int i;
    
    for (i = 0; i < n; i++) {
        param[i] = atoi(strtok(NULL, " "));
    }
    
    func(param);
}

void undo() {
    int i;
    
    free(history[--g_history_n]);
    
    if (g_history_n > 0) {
        free(history[--g_history_n]);
        init_canvas();
        
        for (i = 0; i < g_history_n; i++) {
            interpret_command(history[i]);
        }
    }
}

int interpret_command(const char *command)
{
    char buf[BUFSIZE];
    strcpy(buf, command);
    
    char *s = strtok(buf, " ");
    char c = tolower(s[0]);
    
    switch (c) {
        case 'l':
            exe_command(draw_line, 4);
            break;
        case 'c':
            exe_command(draw_circle, 3);
            break;
        case 'r':
            exe_command(draw_rectangle, 4);
            break;
        case 'u':
            undo();
            break;
        case 'q':
            return COM_QUIT;
        default:
            break;
    }
    return COM_OTHER;
}

int main()
{
    int com, i;
    const char *canvas_file = "canvas.txt";
    FILE *fp;
    char buf[BUFSIZE];
    
    if ((fp = fopen(canvas_file, "w")) == NULL) {
        fprintf(stderr, "error: cannot open %s.\n", canvas_file);
        return 1;
    }
    
    init_canvas();
    print_canvas(fp);
    
    while (1) {
        printf("%d > ", g_history_n);
        fgets(buf, BUFSIZE, stdin);
        
        history[g_history_n] = (char*)malloc(sizeof(char) * strlen(buf));
        strcpy(history[g_history_n], buf);
        if (++g_history_n >= HISTORY_SIZE) break;
        
        com = interpret_command(buf);
        if (com == COM_QUIT) break;
        print_canvas(fp);
    }
    
    fclose(fp);
    
    if ((fp = fopen("history.txt", "w")) != NULL) {
        for (i = 0; i < g_history_n; i++) {
            fprintf(fp, "%s", history[i]);
            free(history[i]);
        }
        fclose(fp);
    }
}
