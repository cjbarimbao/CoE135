#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int id = fork();
    if(id < 0) {
        perror("fork in main() ");
        exit(1);
    } else if(id == 0) {
        execlp("ls", "ls", "-l", NULL);
    } else {
        int wc = wait(NULL);
        printf("Hello, I am parent of %d (wc:%d) (pid:%d)\n", id, wc, (int) getpid());
    }
    return 0;
}