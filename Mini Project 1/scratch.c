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
    int fd, status;
    char *myfifo = "fifo1";
    char buf[100];
    fd_set readfds;
    struct timeval tv;


    /* create the FIFO (named pipe) */
    mkfifo(myfifo, 0666);
    /* write "Hi" to the FIFO */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    fd = open(myfifo, O_RDONLY);
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    status = select(fd+1, &readfds, NULL, NULL, &tv);

    if (status == -1) {
        perror("select()");
        close(fd);
        unlink(myfifo);    
        _exit(EXIT_FAILURE);
    } else if (status == 0) {
        puts("timeout");
    } else {
        read(fd, buf, 100);
        buf[strcspn(buf, "\r\n")] = 0;
        printf("Received: %s\n", buf);
    }

    close(fd);
    unlink(myfifo);

  return 0;
}