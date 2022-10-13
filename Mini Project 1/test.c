// Signal handler test

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

/*------------------
 * Global variables
------------------ */

static volatile bool exit_flag = false;
unsigned int score = 0;

/* ---------------------
 * Function definitions
--------------------- */

// SIGINT handler for child
void sigint(int signo) {
    exit_flag = true;
}

// SIGUSR1 handler
void terminate_contestant(int signo) {
    exit(EXIT_SUCCESS);
}


/*------------------ 
 * Main function
------------------ */

int main(void) 
{
    char buf[BUF_MAX];
    int wstatus = 0;
    struct sigaction sa, sigusr_1;
    pid_t contestant_id;
    sigset_t set;

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

            // setup sigaction for child SIGINT
            sa.sa_handler = sigint;
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
            while (true) {
                fgets(buf, BUF_MAX, stdin);
                buf[strcspn(buf, "\r\n")] = 0;
                if (exit_flag == true) {
                    printf("Are you sure you want to quit? (y/n) ");
                    fgets(buf, BUF_MAX, stdin);
                    buf[strcspn(buf, "\r\n")] = 0;
                    if (!strcmp(buf, "y")) {
                        // terminate
                        _exit(3);
                    } else {
                        // continue
                        exit_flag = false;
                        fgets(buf, BUF_MAX, stdin);
                        buf[strcspn(buf, "\r\n")] = 0;
                    }
                } 
                if (!strcmp(buf, "y")) {
                    puts("Correct!");
                } else {
                    puts("Wrong!\nTerminating...");
                    sigemptyset(&set);
                    if (sigaddset(&set, SIGUSR1) == -1) {
                        perror("sigaddset() in main()");
                        return 1;
                    }
                    if (sigwait(&set, NULL) == -1) {
                        perror("sigwait() in main()");
                        return 1;
                    }    
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
                    puts("Contestant terminated.");
                    exit(EXIT_SUCCESS);
                } 
            }
        }
    }
    return 0;
}