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
} question_t;

/*------------------
 * Global variables
------------------ */

/* ---------------------
 * Function definitions
--------------------- */

void parse_question(question_t* question, char* buf) {
    // parse question
    int i = 0; 
    char* token;
    char token_buffer[7][MAX_STR_WIDTH];
    // get number of operands
    token = strtok(buf, "/");
    puts(token);
    question->num_operands = (uint)strtol(token, NULL, 10);
    // get timeout value
    token = strtok(NULL, "/");
    puts(token);
    question->timeout = (uint)strtol(token, NULL, 10);
    // extract other tokens (Operand1/.../Operand5)
    token = strtok(NULL, "/");
    while (token != NULL) {
        puts(token);
        strncpy(token_buffer[i++], token, MAX_STR_WIDTH);
        token = strtok(NULL, "/");
    }
    // convert operands to integers and store to question struct
    for (i = 0; i < question->num_operands; i++) {
            question->number[i] = (uint)strtol(token_buffer[i], NULL, 10);
    }
}


/*------------------ 
 * Main function
------------------ */

int main(void) 
{
    int i;
    int question_count = 1;
    char buf[BUF_MAX] = "3/10/6971/567/12";
    question_t question;
   // print question number
   printf("Question %d\n", question_count++);
   // parse question
   parse_question(&question, buf);
   printf("Number of operands: %d\nTimeout: %d\n", question.num_operands, question.timeout);
    for (i = 0; i < question.num_operands; i++) {
         printf("Operand %d: %d\n", i+1, question.number[i]);
    }
    
    return 0;
}