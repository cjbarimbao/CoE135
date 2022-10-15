#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>


int main() {
    int fd;
    char *myfifo = "fifo1";
    char buf[100];

    fd = open(myfifo, O_WRONLY);
    fgets(buf, 100, stdin);
    buf[strcspn(buf, "\r\n")] = 0;
    write(fd, buf, strlen(buf)+1);
    close(fd);
    
  return 0;
}