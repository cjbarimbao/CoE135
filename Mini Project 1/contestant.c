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
#include <sys/wait.h>
#include <errno.h>

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
char *fifo_name;
unsigned int score = 0;

/* ---------------------
 * Function definitions
--------------------- */

// SIGINT handler
void handler(int signo) {
    exit_flag = true;
}

// SIGUSR1 handler
void terminate_contestant(int signo) {
    exit(EXIT_SUCCESS);
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

void read_data(char *buf, char *fifo_name) {
    int fd_r;
    // open FIFO file for reading
    fd_r = open(fifo_name, O_RDONLY);

    if (fd_r == -1) {
        fprintf(stderr, "Error %d in read_data(): %s\n", errno, strerror(errno));
        _exit(EXIT_FAILURE);
    }

    // read data from FIFO file
    if (read(fd_r, buf, BUF_MAX) == -1) {
        fprintf(stderr, "Error %d in read_data(): %s\n", errno, strerror(errno));
        close(fd_r);
        _exit(EXIT_FAILURE);
    }

    close(fd_r);
}

void write_data(char *buf, char *fifo_name) {
    int fd_w;
    // open FIFO file for writing
    fd_w = open(fifo_name, O_WRONLY);

    if (fd_w == -1) {
        fprintf(stderr, "Error %d in write_data(): %s\n", errno, strerror(errno));
        _exit(EXIT_FAILURE);
    }

    // write data to FIFO file
    if (write(fd_w, buf, strlen(buf) + 1) == -1) {
        fprintf(stderr, "Error %d in write_data(): %s\n", errno, strerror(errno));
        close(fd_w);
        _exit(EXIT_FAILURE);
    }

    close(fd_w);
}

/*------------------ 
 * Main function
------------------ */

int main(int argc, char*argv[]) 
{
    int i;
    int wstatus = 0;
    int question_count = 1;
    char buf[BUF_MAX];
    struct sigaction sa, sigusr_1;
    question_t question;
    pid_t contestant_id;
    sigset_t set;
    
    fifo_name = argv[1];

    // check for proper usage
    if (argc != 2) {
        printf("Usage: \"%s <FIFO file name>\"\n", argv[0]);
        return 1;
    }

    // check if file exists
    if (access(fifo_name, F_OK) == -1) {
        printf("No such instance of \"Shop Wisely!\" exists, terminating...\n");
        return 1;
    }

    while (1) {
        // fork a child process
        contestant_id = fork();

        // check for fork error
        if (contestant_id == -1) {
            perror("fork() in main()");
            return 1;
        } 
        // child process
        else if (contestant_id == 0) {

            // setup sigaction for SIGINT
            sa.sa_handler = handler;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            // setup sigaction for SIGUSR1
            sigusr_1.sa_handler = terminate_contestant;
            sigemptyset(&sigusr_1.sa_mask);
            sigusr_1.sa_flags = 0;

            // check for SIGINT
            if (sigaction(SIGINT, &sa, NULL) == -1) {
                perror("sigaction() in main()");
                return 1;
            }

            // check for SIGUSR1
            if (sigaction(SIGUSR1, &sigusr_1, NULL) == -1) {
                perror("sigaction() in main()");
                return 1;
            }

            // print welcome message
            puts("Welcome to Shop Wisely!");
            // print contestant ID
            printf("Your ID is %d\n", (int)getpid());
            // send PID to gamemaster
            snprintf(buf, BUF_MAX, "%d", (int)getpid());
            write_data(buf, fifo_name);
            // contestant loop
            while (true) {
                // print question number
                printf("Question %d\n", question_count++);
                // read from FIFO file
                read_data(buf, fifo_name);
                // parse question
                parse_question(&question, buf);
                // print question
                printf("For %d seconds\n", question.timeout);
                for (i = 0; i < question.num_operands; i++) {
                    printf("%d\n", question.number[i]);
                }
                // ask user input for answer
                printf("Your answer: ");
                fgets(buf, BUF_MAX, stdin);
                buf[strcspn(buf, "\r\n")] = 0;
                if (exit_flag == true) {
                    printf("Are you sure you want to quit? (y/n) ");
                    fgets(buf, BUF_MAX, stdin);
                    buf[strcspn(buf, "\r\n")] = 0;
                    if (!strcmp(buf, "y")) {
                        // print scores
                        printf("Your score for this round is %d\n", score);
                        // terminate
                        _exit(3);
                    } else {
                        // continue
                        exit_flag = false;
                        fgets(buf, BUF_MAX, stdin);
                        buf[strcspn(buf, "\r\n")] = 0;
                    }
                }
                write_data(buf, fifo_name);
                // check answer
                if (check_answer(buf, question.answer)) {
                    puts("CORRECT!");
                    // update score
                    score++;
                } else {
                    puts("WRONG!\nTerminating...");
                    // wait for SIGUSR1
                    // add SIGUSR1 to signal mask
                    sigemptyset(&set);
                    if (sigaddset(&set, SIGUSR1) == -1) {
                        perror("sigaddset() in main()");
                        return 1;
                    }
                    if (sigwait(&set, NULL) == -1) {
                        perror("sigwait() in main()");
                        return 1;
                    }
                    // print scores
                    printf("Your score for this round is %d\n", score);
                    // terminate
                    exit(EXIT_SUCCESS);
                }
            }
        }
        // parent process
        else {
            // print parent ID
            printf("Parent ID: %d\n", (int)getpid());
            // setup sigaction for parent SIGINT
            sa.sa_handler = SIG_IGN;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            // check for SIGINT
            if (sigaction(SIGINT, &sa, NULL) == -1) {
                perror("sigaction() in main()");
                return 1;
            }
            // wait for child process to terminate
            wait(&wstatus);
            if (WIFEXITED(wstatus)) {
                if (WEXITSTATUS(wstatus) == 3) {
                    puts("TERMINATED.");
                    exit(EXIT_SUCCESS);
                } 
            }
        }
    }
}