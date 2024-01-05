//programa independiente para reconstruir la pila desde un fichero
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "my_lib.h"
#define NUM_THREADS 10

int main(int argc, char* argv[]) {
    // Verificar si se proporcionó el nombre del fichero
    if (!argv[1]) {
        fprintf(stderr, "USAGE: %s <stack_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    // Reconstruir la pila en memoria desde el fichero
   struct my_stack *stack = my_stack_read(argv[1]);
    //comprobar que la pila no esté vacía
    if (!stack){
        perror("Couldn't open stack file s");
        return EXIT_FAILURE;
    }
    
    int stack_len = my_stack_len(stack);
    printf("Stack length: %d\n", stack_len);
    int items = 0;    
    int sum = 0;
    int min = INT_MAX;
    int max = INT_MIN;

    for (int i = 0; i < NUM_THREADS; ++i) {    //solo se imprimiran los primeros 10 elementos
        int *element = my_stack_pop(stack);
        printf("%d\n", *element);

        sum += *element;
        items++;
      
        if (*element < min) {
            min = *element;
        }

        if (*element > max) {
            max = *element;
        }
    }

    int media = sum / items;

    printf("\nItems: %i Sum: %d Min: %d Max: %d Average: %i\n", items, sum, min, max, media);
    //liberar memoria
    my_stack_purge(stack);
    return EXIT_SUCCESS;
}
