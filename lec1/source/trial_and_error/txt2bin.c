#include <stdio.h>

int main(void)
{
    FILE* fp;
    int max;
    int cell[32];
    int c;
    int i, j;
    
    fp = fopen("intput.txt", "r");
    fscanf(fp, "%d", &max);
    
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            if ((c = getc(fp)) != EOF) {
                cell[i] |= ((c == '#') ? 1 : 0) << j;
            }
            else
                break;
        }
    }
    fclose(fp);
    
    fp = fopen("output.txt", "wb");
    fwrite(&max, sizeof(max), 1, fp);
    fwrite(cell, sizeof(cell), 1, fp);
    fclose(fp);
    
    return 0;
}