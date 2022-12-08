#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
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
----------*/

#define PHYSICAL_MEM 8192
#define BUF_MAX 100
#define SEGMENT_MAX 50

/*--------------------
 * Type definitions
 *-------------------*/

typedef struct {
    uint32_t base_addr;
    uint32_t limit;
    struct segment_t *next;
} segment_t;

typedef struct {
    char name[BUF_MAX];
    struct segment_t **entries;
} segment_table_t;

typedef struct {
    struct segment_table_t **tables;
} segment_tables_t;

/*--------------------
 * Global variables
 *-------------------*/

segment_t *segment_list = NULL;
segment_table_t *segment_table = NULL;
segment_tables_t *segment_tables = NULL;
void *physical_memory = NULL;
/*---------------------
 * Function declaration
 *--------------------*/

void *vmalloc(uint32_t size) {
    /* allocates size: size of the memory to be allocated
    *  returns: segment_t pointer to the allocated memory
    */
    return NULL;
}


void add_process() {
    
    int i = 0;
    int j = 0;
    char dir_name[BUF_MAX];
    char segment_names[SEGMENT_MAX][BUF_MAX];

    // ask for directory name
    printf("Enter directory name: ");
    fgets(dir_name, BUF_MAX, stdin);
    dir_name[strcspn(dir_name, "\r\n")] = 0;

    // open directory
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        fprintf(stderr, "Error %d in add_process(): %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    // read directory
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            strncpy(segment_names[i++], entry->d_name, BUF_MAX);
        }
    }
    // check if directory is empty
    if (i == 0) {
        fprintf(stderr, "Error: No segment files found in directory %s\n", dir_name);
        exit(EXIT_FAILURE);
    }
    // check if space is available on physical memory
    // allocate space on physical memory
    // call vmalloc()???

    // print segment names
    printf("%d segments detected\n", i);
    for (j = 0; j < i; j++) {
        printf("%s segment base_addr: \n", segment_names[j]);
    }

    // close directory
    closedir(dir);
    

    return;
}

void termination_sequence() {
    // free all allocated memory
    // close all open files
    // exit
    exit(EXIT_SUCCESS);
}


/*---------------
 * Main function 
 *--------------*/

int main(void)
{
    // ask for command from user
    char command[10];
    
    printf("Enter a command: ");
    fgets(command, 100, stdin);
    command[strcspn(command, "\r\n")] = 0;
    
    // parse command
    switch (command[0])
    {
    case 'a':
        /* adds a process in memory */
        add_process();
        break;
    case 'r':
        /* reads a specific byte in the process */
        break;
    case 's':
        /* access the entire contents of the segment */
        break;
    case 'd':
        /* describes the process segment table (base address and limit) */
        break;
    case 'l':
        /* lists down the processes currently running*/
        break;
    case 'i':
        /* displays free space */
        break;
    case 't':
        /* terminates a process */
        break;
    case 'q':
        /* quit */
        termination_sequence();
        break;
    default:
        puts("Couldn't parse command.");
        exit(EXIT_FAILURE);
    }

    return 0;
}