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

/*---------
 * Macros
--------- */

#define BUF_MAX 1024
#define MAX_STR_WIDTH 8

/*------------------
 * Type definitions
------------------ */

typedef struct question_s {
    unsigned int num_operands;
    unsigned int number[5];
    unsigned int timeout;
    unsigned int answer;
} question_t;

/*------------------
 * Global variables
------------------ */

static volatile bool exit_flag = false;
char scores[BUF_MAX];
char *fifo_name;
unsigned int score = 0;

/* ---------------------
 * Function definitions
--------------------- */

// SIGINT handler
void handler(int signo) {
    exit_flag = true;
}

void parse_question(question_t* question, char* buf) {
    // parse question
    int i = 0;
    char* token;
    char token_buffer[7][MAX_STR_WIDTH];
    // get number of operands
    token = strtok(buf, "/");
    question->num_operands = (uint)strtol(token, NULL, 10);
    // get timeout value
    token = strtok(NULL, "/");
    question->timeout = (uint)strtol(token, NULL, 10);
    // get question answer
    token = strtok(NULL, "/");
    question->answer = (uint)strtol(token, NULL, 10);
    // extract other tokens (Operand1/.../Operand5)
    token = strtok(NULL, "/");
    while (token != NULL) {
        strncpy(token_buffer[i++], token, MAX_STR_WIDTH);
        token = strtok(NULL, "/");
    }
    // convert operands to integers and store to question struct
    for (i = 0; i < question->num_operands; i++) {
            question->number[i] = (uint)strtol(token_buffer[i], NULL, 10);
    }
}

int check_answer(char *buf, unsigned int answer) {
    unsigned int user_answer;
    
    user_answer = (unsigned int)strtol(buf, NULL, 10);
    return (user_answer == answer);
}

/*------------------ 
 * Main function
------------------ */

int main(int argc, char*argv[]) 
{
    int fd_r, fd_w;
    int i;
    int question_count = 1;
    char buf[BUF_MAX];
    struct sigaction sa;
    question_t question;
    pid_t contestant_id;
    int answer;
    sigset_t set;
    
    fifo_name = argv[1];
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // check for proper usage
    if (argc != 2) {
        printf("Usage: \"%s <FIFO file name>\"\n", argv[0]);
        return 1;
    }

    // open FIFO file for reading
    fd_r = open(fifo_name, O_RDONLY);

    if (fd_r == -1) {
        fprintf(stderr, "No such instance of \"Shop Wisely!\" exists, terminating...\n");
        return 1;
    }

    // open FIFO file for writing
    fd_w = open(fifo_name, O_WRONLY);

    if (fd_w == -1) {
        perror("open() in main()");
        return 1;
    }

    // check for SIGINT
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction() in main()");
        close(fd_r);
        close(fd_w);
        return 1;
    }

    while (1) {
        // fork a child process
        contestant_id = fork();

        // check for fork error
        if (contestant_id == -1) {
            perror("fork() in main()");
            close(fd_r);
            close(fd_w);
            return 1;
        } 
        // child process
        else if (contestant_id == 0) {
            // print welcome message
            puts("Welcome to Shop Wisely!");
            // print contestant ID
            printf("Your ID is %d\n", (int)getpid());
            // send PID to gamemaster
            snprintf(buf, BUF_MAX, "%d", (int)getpid());
            if (lseek(fd_w, 0, SEEK_SET) == -1) {
                perror("lseek() in main()");
                close(fd_r);
                close(fd_w);
                return 1;
            }
            if (write(fd_w, buf, strlen(buf) + 1) == -1) {
                perror("write () in main()");
                close(fd_r);
                close(fd_w);
                return 1;
            } 
            // contestant loop
            while (exit_flag == false) {
                // print question number
                printf("Question %d\n", question_count++);
                // read from FIFO file
                // reposition fd_r to beginning of file
                if (lseek(fd_r, 0, SEEK_SET) == -1) {
                    perror("lseek() in main()");
                    close(fd_r);
                    close(fd_w);
                    return 1;
                }
                if (read(fd_r, buf, BUF_MAX) == -1) {
                    perror("read() in main()");
                    close(fd_r);
                    close(fd_w);
                    return 1;
                }
                // parse question
                parse_question(&question, buf);
                // print question
                printf("For %d seconds\n", question.timeout);
                for (i = 0; i < question.num_operands; i++) {
                    printf("%d\n", question.number[i]);
                }
                // ask user input for answer
                puts("Your answer: ");
                fgets(buf, BUF_MAX, stdin);
                // write answer to FIFO file
                if (lseek(fd_w, 0, SEEK_SET) == -1) {
                    perror("lseek() in main()");
                    close(fd_r);
                    close(fd_w);
                    return 1;
                }
                if (write(fd_w, buf, strlen(buf) + 1) == -1) {
                    perror("write () in main()");
                    close(fd_r);
                    close(fd_w);
                    return 1;
                }
                // check answer
                if (check_answer(buf, question.answer)) {
                    puts("CORRECT!");
                } else {
                    puts("WRONG!\nTerminating...");
                    // wait for SIGUSR1
                    // add SIGUSR1 to signal mask
                    sigemptyset(&set);
                    if (sigaddset(&set, SIGUSR1) == -1) {
                        perror("sigaddset() in main()");
                        close(fd_r);
                        close(fd_w);
                        return 1;
                    }
                    if (sigwait(&set, NULL) == -1) {
                        perror("sigwait() in main()");
                        close(fd_r);
                        close(fd_w);
                        return 1;
                    }
                    // terminate
                    exit(0);
                }
            }
        }
        // parent process
        else {
            // wait for child process to terminate
            wait(NULL);
        }
    }
}