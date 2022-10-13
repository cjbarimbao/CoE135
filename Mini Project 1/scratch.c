#include <stdio.h>
#include <string.h>

int main() {
    char buf[100];
    fgets(buf, 100, stdin);
    puts(buf);
    printf("%d\n", strncmp(buf, "Correct\n", 7));
    buf[strcspn(buf, "\r\n")] = 0;
    puts(buf);
    printf("%d\n", strncmp(buf, "Correct", 7));

  return 0;
}