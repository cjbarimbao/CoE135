#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(void) {
    int fd1, fd2;
    char buf[100];

    fd1 = open("hello.txt", O_RDONLY, S_IRUSR);
    
    if(fd1 == -1) {
        perror("Error ");
        return 1;
    }

    printf("Bytes read: %d\n", (int)read(fd1, buf, sizeof(buf)));

    fd2 = open("output.txt", O_CREAT|O_RDWR, S_IRWXU);

    if(fd2 == -1) {
        perror("Error ");
        return 1;
    }
    

    printf("File 'output.txt' created. Bytes written: %d\n", (int)write(fd2, "abcde", strlen("abcde")+1));

    close(fd2);

    fd2 = open("output.txt", O_CREAT|O_RDWR, S_IRWXU);

    printf("Bytes read: %d\n", (int)read(fd2, buf, sizeof(buf)));

    printf("Buffer length is %d\nString is: %s\n", (int)strlen(buf), buf);

    close(fd1);
    close(fd2);
    return 0;
}