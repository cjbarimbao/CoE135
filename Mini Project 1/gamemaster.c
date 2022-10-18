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

static volatile bool exit_flag = false;
unsigned int current_index = 0;
process_t process_list[100];
char scores[BUF_MAX];
char *fifo_name;

/*---------------------
 * Function definitions
--------------------- */

void handler(int signo) {
    exit_flag = true;
}

void termination_sequence(void) {
    FILE *fp;
    // open scores file for writing
    fp = fopen("scores.txt", "w");
    if (fp == NULL) {
        perror("fopen() in main()");
        unlink(fifo_name);
        _exit(EXIT_FAILURE);
    }
    // write scores to file
    fputs(scores, fp);
    puts("Goodbye! All results are in scores.txt");
    exit_flag = false;
    fclose(fp);
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

    // Format of question: "No. of operands/Timeout value/Answer/Operand1/Operand2/.../Operand5"
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

int read_data(char *buf, char *fifo_name) {
    int status;
    int fd_r;
    // open FIFO file for reading
    fd_r = open(fifo_name, O_RDONLY);

    if (fd_r == -1) {
        if (exit_flag) {
            termination_sequence();
        }
        fprintf(stderr, "Error %d open() in read_data(): %s\n", errno, strerror(errno));
        unlink(fifo_name);
        _exit(EXIT_FAILURE);
    }

    // read data from FIFO file
    status = read(fd_r, buf, BUF_MAX);
    if (status == -1) {
        fprintf(stderr, "Error %d in read() read_data(): %s\n", errno, strerror(errno));
        close(fd_r);
        unlink(fifo_name);
        _exit(EXIT_FAILURE);
    }

    close(fd_r);
    
    return status;
}

void write_data(char *buf, char *fifo_name) {
    int fd_w;
    // open FIFO file for writing
    fd_w = open(fifo_name, O_WRONLY);

    if (fd_w == -1) {
        fprintf(stderr, "Error %d in write_data(): %s\n", errno, strerror(errno));
        unlink(fifo_name);
        _exit(EXIT_FAILURE);
    }

    // write data to FIFO file
    if (write(fd_w, buf, strlen(buf) + 1) == -1) {
        fprintf(stderr, "Error %d in write_data(): %s\n", errno, strerror(errno));
        close(fd_w);
        unlink(fifo_name);
        _exit(EXIT_FAILURE);
    }

    close(fd_w);
}

int wait_answer(char *buf, char *fifo_name, question_t *question) {
    // returns 0 if timeout, 1 if answer is received within time limit
    int status;
    int fd_r;
    fd_set readfds;
    struct timeval tv;

    // open FIFO file for reading
    fd_r = open(fifo_name, O_RDONLY);

    if (fd_r == -1) {
        fprintf(stderr, "Error %d in wait_answer(): %s\n", errno, strerror(errno));
        unlink(fifo_name);
        _exit(EXIT_FAILURE);
    }

    // set timeout value
    tv.tv_sec = question->timeout;
    tv.tv_usec = 0;

    // set file descriptor
    FD_ZERO(&readfds);
    FD_SET(fd_r, &readfds);
    puts("Waiting for answer...");
    // wait for data from FIFO file
    status = select(fd_r + 1, &readfds, NULL, NULL, &tv);
    
    if (status == -1) {
        fprintf(stderr, "Error %d in wait_answer(): %s\n", errno, strerror(errno));
        close(fd_r);
        unlink(fifo_name);
        _exit(EXIT_FAILURE);
    } else if (status == 0) {
        close(fd_r);
        return 0;
    } else {
        // read data from FIFO file
        if (FD_ISSET(fd_r, &readfds)) {
            if (read(fd_r, buf, BUF_MAX) == -1) {
                fprintf(stderr, "Error %d in wait_answer(): %s\n", errno, strerror(errno));
                close(fd_r);
                unlink(fifo_name);
                _exit(EXIT_FAILURE);
            }
        }

        close(fd_r);
        return 1;
    }
}

/*----------------
 * main function
---------------- */

int main(int argc, char *argv[]) 
{
    int i;
    int question_count;
    char buf[BUF_MAX];
    struct sigaction sa;
    question_t question;
    pid_t contestant_id;
    
    fifo_name = argv[1];
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // check for proper usage
    if (argc != 2) {
        printf("Usage: \"%s <FIFO file name>\"\n", argv[0]);
        return 1;
    }
    
    // create FIFO file for game master to read from
    if (mkfifo(fifo_name, 0666) == -1) {
        fprintf(stderr, "Error %d in main(): %s\n", errno, strerror(errno));
        if (errno == EEXIST) {
            unlink(fifo_name);
        }
        return 1;
    }

    // seed random number generator
    srand(time(NULL));
    puts("Welcome to the \"Shop Wisely!\" Host Program!");
    
    // check for SIGINT
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction() in main()");
        return 1;
    }

    while (1) {
        // start of loop when previous contestant is kicked
        // get ID of contestant
        if (read_data(buf, fifo_name) == 0) {
            continue;
        }
        // convert to integer
        contestant_id = (int)strtol(buf, NULL, 10);
        // store ID
        store_ID(contestant_id);
        printf("Contestant with ID: %d has joined!\n", contestant_id);
        // reset question count
        question_count = 1;

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
        write_data(buf, fifo_name);
        // wait for response
        wait_for_response:
        if (wait_answer(buf, fifo_name, &question)) {
            // check for termination request
            if (strcmp(buf, "request") == 0) {
                puts("Contestant issued a termination request");
                read_data(buf, fifo_name);

                if (strcmp(buf, "terminate") == 0) {
                    puts("Client left goodbye!");
                    goto update_score;
                } else {
                    goto wait_for_response;
                } 
                
            } else {
                if (check_answer(buf, question.answer)) {
                    puts("CORRECT!");
                    // update score of contestant
                    process_list[current_index-1].score++;
                    goto next_question;
                } else {
                    puts("WRONG!");
                }
            } 
        } else {
            // When no data is read from the pipe
            puts("TIMED OUT!");
        }
        printf("The answer is %d\n", question.answer);
        printf("Terminating contestant %d\n", process_list[current_index-1].id);
        // add delay to avoid race conditions
        sleep(1);
        // kill contestant process
        if (kill(process_list[current_index-1].id, SIGUSR1) == -1) {
            perror("kill() in main()");
            unlink(fifo_name);
            return 1;
        }
        update_score:
        snprintf(buf, BUF_MAX, "%d -> %d\n", process_list[current_index-1].id, process_list[current_index-1].score);
        strncat(scores, buf, BUF_MAX - strlen(scores) - 1);

        // wait for connection from  contestant
        puts("Waiting for connection...");
    }
    return 0;
}