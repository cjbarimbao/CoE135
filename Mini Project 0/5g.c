#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    int id = fork();

    if(id < 0) {
        perror("fork in main() " );
        exit(1);
    } else if(id == 0) {
        printf("Hello, I am child process (PID: %d).\n", (int)getpid());
    } else {
        printf("Hello, I am parent process of %d (PID: %d).\n", id, (int)getpid());
    }

    return 0;
}