#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIDTH 70
#define HEIGHT 40
#define BUFSIZE 1000
#define HISTORY_SIZE 100

enum COM_STATE {COM_SUCCESS, COM_ERROR, COM_FEW_PARAM, COM_OTHER, COM_QUIT};
const char *ERROR_MES[] = {
    "正常に処理されました。\n",
    "パラメータが不正です。\n",
    "パラメータが足りません。\n",
    "存在しないコマンドです。\n",
    "終了します。\n"
};

typedef struct struct_diff {
    int x;
    int y;
    char before;
    struct struct_diff *next;
} DIFF;

typedef struct struct_diff_list {
    DIFF *dif;
    struct struct_diff_list *next;
} STACK;

char g_canvas[WIDTH][HEIGHT];
STACK *history;
STACK undo_history;

void make_history()
{
    STACK *n;
    
    n = (STACK *)malloc(sizeof(STACK));
    n->next = history;
    history = n;
}

void add_diff(int x, int y, char c)
{
    DIFF *p, *n;
    
    if (g_canvas[x][y] != c) {
        n = (DIFF *)malloc(sizeof(DIFF));
        n->x = x;
        n->y = y;
        n->before = g_canvas[x][y]; // 前の状態を持つ
        n->next = history->dif;
        history->dif = n;
        
        g_canvas[x][y] = c; // 変更しておく
    }
}

void delete_history()
{
    STACK *d;
    DIFF *p, *q;
    
    d = history;
    
    if (d != NULL)
    {
        p = d->dif;
        while (p != NULL) {
            q = p;
            p = p->next;
            free(q);
        }
        history = d->next;
        free(d);
    }
}

void print_canvas(FILE *fp)
{
    int x, y;
    
    fprintf(fp, "----------\n");
    
    for (y = 0; y < HEIGHT; y++) {
        for (x = 0; x < WIDTH; x++) {
            char c = g_canvas[x][y];
            fputc(c, fp);
        }
        fputc('\n', fp);
    }
    fflush(fp);
}

int init_canvas()
{
    memset(g_canvas, ' ', sizeof(g_canvas));
}

int draw_line(int param[])
{
    int x0 = param[0], y0 = param[1], x1 = param[2], y1 = param[3];
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int num = (dx >= dy) ? dx : dy;
    double delta = (num != 0) ? 1.0 / num : 0;
    int i;
    
    for (i = 0; i <= num; i++) {
        int x = x0 + delta * i * (x1 - x0) + 0.5;
        int y = y0 + delta * i * (y1 - y0) + 0.5;
        
        if (0 <= x && x < WIDTH && 0 <= y && y < HEIGHT) {
            add_diff(x, y, '#');
        }
    }
    return COM_SUCCESS;
}

int draw_circle(int param[])
{
    int x0 = param[0], y0 = param[1], r = param[2];
    int n = ceil(2 * M_PI * r);
    int i;
    
    if (r <= 0) {
        return COM_ERROR;
    }
    
    for (i = 1; i < n; i++) {
        int x = x0 + r * cos((double)i / r);
        int y = y0 + r * sin((double)i / r);
        
        if (0 <= x && x < WIDTH && 0 <= y && y < HEIGHT) {
            add_diff(x, y, '#');
        }
    }
    
    return COM_SUCCESS;
}

int draw_rectangle(int param[])
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
    
    return COM_SUCCESS;
}


int exe_command(int (*func)(int []), int n)
{
    char *text;
    int *param = (int *)malloc(sizeof(int) * n);
    int i;
    
    for (i = 0; i < n; i++) {
        text = strtok(NULL, " \n");
        if (text == NULL) {
            break;
        }
        param[i] = atoi(text);
    }
    
    if (n != i) {
        return COM_FEW_PARAM;
    }
    
    make_history(); // 変更点を保存する構造体を確保
    return func(param);
}

int undo()
{
    STACK *d;
    DIFF *p, *q;
    
    d = history;
    
    if (d != NULL)
    {
        p = d->dif;
        while (p != NULL) {
            q = p;
            p = p->next;
            g_canvas[q->x][q->y] = q->before;
            free(q);
        }
        history = d->next;
        free(d);
    }
    
    return COM_SUCCESS;
}

int interpret_command(const char *command)
{
    char buf[BUFSIZE];
    strcpy(buf, command);
    
    char *s = strtok(buf, " ");
    char c = tolower(s[0]);
    
    switch (c) {
        case 'l':
            return exe_command(draw_line, 4);
        case 'c':
            return exe_command(draw_circle, 3);
        case 'r':
            return exe_command(draw_rectangle, 4);
        case 'u':
            return undo();
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
    const char *history_file = "history.txt";
    FILE *fp_can, *fp_his;
    char buf[BUFSIZE];
    
    if ((fp_can = fopen(canvas_file, "w")) == NULL) {
        fprintf(stderr, "error: cannot open %s.\n", canvas_file);
        return 1;
    }
    
    init_canvas();
    print_canvas(fp_can);
    
    if ((fp_his = fopen(history_file, "w")) == NULL) {
        fprintf(stderr, "error: cannot open %s.\n", canvas_file);
        return 1;
    }
    
    while (1) {
        printf("> ");
        fgets(buf, BUFSIZE, stdin);
        
        com = interpret_command(buf);
        if (com != COM_SUCCESS) {
            printf("%s", ERROR_MES[com]);
            if (com == COM_QUIT) break;
            if (com == COM_FEW_PARAM || com == COM_ERROR)
                delete_history();
        }
        else {
            fprintf(fp_his, "%s", buf);
        }
        print_canvas(fp_can);
    }
    fclose(fp_can);
    fclose(fp_his);
    
    return 0;
}
