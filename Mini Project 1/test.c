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

typedef struct process_s {
    pid_t id;
    unsigned int score;
} process_t;

/*------------------
 * Global variables
------------------ */

unsigned int current_index = 0;
process_t process_list[100];
char scores[BUF_MAX];
char *fifo_name;

/*---------------------
 * Function definitions
--------------------- */

void handler(int signo) {
    int fd = open("scores.txt", O_CREAT|O_RDWR, S_IRWXU);
    
    if (fd == -1) {
        write(STDERR_FILENO, "Error opening \"scores.txt\"\n", 27);
        _exit(EXIT_FAILURE);
    }
    
    write(STDOUT_FILENO, "Goodbye! All results are in scores.txt\n", 39);
    unlink(fifo_name);
    _exit(EXIT_SUCCESS);
}

void store_ID(int id) {
    process_list[current_index].id = id;
    process_list[current_index].score = 0;
    current_index++;
}

question_t generate_question(void) {
    question_t question;
    int i;
    double unit;

    question.answer = 0;
    // determine how many integers to generate
    question.num_operands = (rand() % 4) + 2;
    // generate random integers between 10-9999
    for (i = 0; i < question.num_operands; i++) {
        question.number[i] = (rand() % 9990) + 10;
        unit = pow(10, floor(log10(question.number[i])));
        question.answer += (unsigned int)(round(question.number[i] / unit) * unit);
    }
    
    question.timeout = (5*(question.num_operands - 1));
    
    return question;
}

void write_question(question_t question, char *buf) {
    int i;
    char strings[MAX_STR_WIDTH];

    snprintf(buf, BUF_MAX, "%d/%d/%d/", question.num_operands, question.timeout, question.answer);
    for (i = 0; i < question.num_operands; i++) {
        snprintf(strings, MAX_STR_WIDTH, "%d/", question.number[i]);
        strncat(buf, strings, BUF_MAX - strlen(buf) - 1);
    }
}

int check_answer(char *buf, unsigned int answer) {
    unsigned int user_answer;
    
    user_answer = (unsigned int)strtol(buf, NULL, 10);
    printf("Contestant answered %d\n", user_answer);

    return (user_answer == answer);
}

/*----------------
 * main function
---------------- */

int main(int argc, char *argv[]) {
    int fd_r, fd_w;
    int i;
    int question_count = 1;
    char buf[BUF_MAX];
    struct timeval tv;
    struct sigaction sa;
    question_t question;
    pid_t contestant_id;
    fd_set readfds;
    
    fifo_name = argv[1];
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // proper usage
    if (argc != 2) {
        printf("Usage: \"%s <FIFO file name>\"\n", argv[0]);
        return 1;
    }
    
    // make fifo file
    if (mkfifo(fifo_name, 0666) == -1) {
        perror("mkfifo() in main()");
        return 1;
    }

    // open fifo file for game master
    fd_r = open(fifo_name, O_RDONLY);

    if (fd_r == -1) {
        perror("open() in main()");
        unlink(fifo_name);
        return 1;
    }

    fd_w = open(fifo_name, O_WRONLY);

    if (fd_w == -1) {
        perror("open() in main()");
        close(fd_r);
        unlink(fifo_name);
        return 1;
    }

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction() in main()");
        close(fd_r);
        close(fd_w);
        unlink(fifo_name);
        return 1;
    }

    srand(time(NULL));
    puts("welcome to the \"Shop Wisely!\" Host Program!");
    
    while(1) {
        // start of loop when previous contestant is kicked
        // get ID of contestant
        if (lseek(fd_r, 0, SEEK_SET) == -1) {
            perror("lseek() in main()");
            close(fd_r);
            close(fd_w);
            unlink(fifo_name);
            return 1;
        }
        if (read(fd_r, buf, BUF_MAX) == -1) {
            perror("read() in main()");
            close(fd_r);
            close(fd_w);
            unlink(fifo_name);
            return 1;
        }
        // convert to integer
        contestant_id = (int)strtol(buf, NULL, 10);
        // store ID
        store_ID(contestant_id);
        printf("Contestant with ID: %d has joined!\n", contestant_id);
        // start of loop when contestant is still in the game
        next_question:
        printf("Generating Question %d\n", question_count++);
        // generate question
        question = generate_question();
        printf("%d numbers! for %d seconds!\n", question.num_operands, question.timeout);
        for (i = 0; i < question.num_operands; i++) {
            printf("%d\n", question.number[i]);
        }
        puts("Sending...");
        // prepare question in buffer
        write_question(question, buf);
        // send question to contestant
        if (lseek(fd_w, 0, SEEK_SET) == -1) {
            perror("lseek() in main()");
            close(fd_r);
            close(fd_w);
            unlink(fifo_name);
            return 1;
        }
        if (write(fd_w, buf, strlen(buf) + 1) == -1) {
            perror("write () in main()");
            close(fd_r);
            close(fd_w);
            unlink(fifo_name);
            return 1;
        } 
        // wait for response    
        tv.tv_sec = question.timeout;
        tv.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(fd_r, &readfds);
        select(fd_r+1, &readfds, NULL, NULL, &tv);

        if (FD_ISSET(fd_r, &readfds)) {
            if (lseek(fd_r, 0, SEEK_SET) == -1) {
                perror("lseek() in main()");
                close(fd_r);
                close(fd_w);
                unlink(fifo_name);
                return 1;
            }
            if (read(fd_r, buf, BUF_MAX) == -1) {
                perror("read() in main()");
                close(fd_r);
                close(fd_w);
                unlink(fifo_name);
                return 1;
            }
            if (check_answer(buf, question.answer)) {
                puts("CORRECT!");
                // update score of contestant
                process_list[current_index-1].score++;
                goto next_question;
            } else {
                puts("WRONG!");
            }
        } else {
            // When no data is read from the pipe
            puts("TIMED OUT!");
            // terminate contestant
            if (kill(process_list[current_index-1].id, SIGUSR1) == -1) {
                perror("kill() in main()");
                close(fd_r);
                close(fd_w);
                unlink(fifo_name);
                return 1;
            }
        }
        printf("The answer is %d\n", question.answer);
        printf("Terminating contestant %d\n", process_list[current_index-1].id);
        // save score
        snprintf(buf, BUF_MAX, "%d -> %d\n", process_list[current_index-1].id, process_list[current_index-1].score);
        strncat(scores, buf, BUF_MAX - strlen(scores) - 1);
    }
    return 0;
}