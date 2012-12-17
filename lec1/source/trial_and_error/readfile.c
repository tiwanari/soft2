#include <stdio.h>
#include <string.h>

#define BUFSIZE 1000

int main()
{
  char buf[BUFSIZE];
  FILE *fp;
  const char *filename = "tmp.txt";
  
  if ((fp = fopen(filename, "r")) == NULL) {
    printf("error: can't open %s\n", filename);
    return 1;
  }

  while (fgets(buf, BUFSIZE, fp) != NULL) {
    size_t len = strlen(buf) - 1;
    printf("%d\n", len);
  }

  fclose(fp);
}
