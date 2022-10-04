#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node* next;
} node_t;

void print_list(node_t* node) {
    while(node != NULL) {
        printf(" %d ", node->data);
        node = node->next;
    }
}

void free_list(node_t* node) {
    node_t* tmp;
    
    while (node != NULL) {
        tmp = node;
        node = node->next;
        free(tmp);
    }
}

int main(void){
    node_t* head = NULL;
    node_t* second = NULL;
    node_t* third = NULL;
    node_t* fourth = NULL;

    head = (node_t*)malloc(sizeof(node_t));
    second = (node_t*)malloc(sizeof(node_t));
    third = (node_t*)malloc(sizeof(node_t));

    head->data = 1;
    head->next = second;
    second->data = 2;
    second->next = third;
    third->data = 3;
    third->next = NULL;

    printf("Current list elements are:\n");
    print_list(head);

    fourth = (node_t*)malloc(sizeof(node_t));
    third->next = fourth;
    fourth->next = NULL;

    printf("\n\nEnter new integer element to append: ");
    scanf("%d", &(fourth->data));

    printf("\nUpdated list elements are:\n");
    print_list(head);
    printf("\n\n");
    
    free_list(head);
    
    return 0;
}