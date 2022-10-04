#include <stdio.h>

int main(void) {
    char msg[100] = {};
    
    fgets(msg, sizeof(msg), stdin);
    printf("User input: %s", msg);
}