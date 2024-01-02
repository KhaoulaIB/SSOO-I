//programa independiente para reconstruir la pila desde un fichero
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main(int argc, char* argv[]) {
    // Verificar si se proporcionó el nombre del fichero
    if (argc != 2) {
        fprintf(stderr, "Sintaxis incorrecta. Uso: %s <nombre_del_fichero>\n", argv[0]);
        return EXIT_FAILURE;
    }
    struct my_stack *stack;//crear una pila que contedrá los datos del fichero    
    // Reconstruir la pila en memoria desde el fichero
   stack = my_stack_read(argv[1]);
    //comprobar que la pila no esté vacía
    if (!stack){
        fprintf(stderr, "Fichero vacío\n", argv[1]);
        return EXIT_FAILURE;
    }
    
    int stack_len = my_stack_len(stack);
    printf("Stack length: %d\n", stack_len);

    int sum = 0;
    int min = INT_MAX;
    int max = INT_MIN;

    for (int i = 0; i < stack_len; ++i) {
        int* element = my_stack_pop(stack);
        printf("%d ", *element);

        sum += *element;
      
        if (*element < min) {
            min = *element;
        }

        if (*element > max) {
            max = *element;
        }
    }

    double media = (double)sum / stack_len;

    printf("\nSum: %d Min: %d Max: %d Average: %lf\n", sum, min, max, media);

    return EXIT_SUCCESS;
}
