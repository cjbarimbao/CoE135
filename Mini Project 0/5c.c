#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int i;
    int *a;

    a = (int*)calloc(7, sizeof(int));
    
    printf("Array elements:\n");
        
    for(i = 0; i < 7; i++) {
        printf("%d ", a[i]);
    }
    printf("\n\n");
    
    a = (int*)realloc(a, 8);
    
    printf("Enter integer value of 8th element: ");
    scanf("%d", &a[7]);
    printf("\nNew array elements:\n");
        
    for(i = 0; i < 8; i++) {
        printf("%d ", a[i]);
    }
    
    printf("\n");
    
    free(a);
    return 0;
}