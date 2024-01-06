#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "my_lib.h"
#define NUM_THREADS 10
#define  N 1000000
#define sizeInt sizeof(int)

//Variables globale
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
struct my_stack *pila;


//funciones
void *worker(void *ptr);
void stack_init(char *file);
void rellenarPila();
void copyStack(struct my_stack* destination);


/**
*Metodo principal. Prepara la pila, crea lo hilos y muestra sus datos.
*@param args    
*@param argv 
*/

int main(int argc, char *argv[])
{
    // Verificar la sintaxis correcta del programa
    if (argc != 2)
    {
        fprintf(stderr, "USAGE: %s <stack_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    // leer la pila del fichero
    pila = my_stack_read(argv[1]);
    printf("Threads: %d, Iterations: %d\n", NUM_THREADS, N);
    //Preparar la pila
    stack_init(argv[1]);

    // Crear los hilos
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {        pthread_create(&threads[i], NULL, worker, NULL);
        printf("%d) Thread %ld created\n", i,threads[i]);
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }


    printf("\nstack content after threads iterations:\n");
    //pila auxiliar para imprimir nuestra pila compartida
    struct my_stack* aux = my_stack_init(sizeInt);

    // Volcar la pila en un fichero
    int elements_written = my_stack_write(pila, argv[1]);
    //copiamos la pila original en aux y moestramos su contenido
    aux = my_stack_read(argv[1]);
    while(my_stack_len(aux)>0){
        int * valor = malloc(sizeInt);
        valor = my_stack_pop(aux);
        printf("%i\n",*valor);
    }
    //liberamos la memoria de la pila auxiliar
    my_stack_purge(aux);

    printf("stack length: %i\n",my_stack_len(pila));

    printf("\nWritten elements from stack to file: %d\n", elements_written);
    // Liberar la memoria de la pila
    int bytes_released = my_stack_purge(pila);
    printf("Released bytes: %d\n", bytes_released);


    // Finalizar el hilo principal
    printf("Bye from main\n");
    pthread_exit(NULL);

    return EXIT_SUCCESS;
}


void stack_init(char* file){
    if (!pila){//si la pila no existe, la creamos
        pila = my_stack_init(sizeInt);
    }
    //imprimer datos iniciales de la pila
     struct my_stack *aux = my_stack_init(sizeInt); // Inicializa la pila auxiliar
        copyStack(aux);

        printf("stack->size:%i\n",pila->size);

        printf("initial stack length: %i\n",my_stack_len(pila));

        printf("original stack content:\n");
        //mostrar el contenido inicial de la pila
        while (my_stack_len(aux)>0){
            int *data =my_stack_pop(aux);
            if (!data){
            perror("Memoria dinámica insuficiente");
            return; 
            }
            printf("%i \n",*data);
        }

    //Rellenar si el numero de elemenots<10
    if (my_stack_len(pila)<NUM_THREADS){
        
        rellenarPila();
    }


}

/**
 * Copia la pila global a destination en el mismo orden.
 * 
*/
void copyStack(struct my_stack* destination) {
    // Crear una pila temporal para preservar el orden
    struct my_stack *temp_stack = my_stack_init(sizeInt);

    // Copiar elementos de la pila original a la pila temporal
    struct my_stack_node *current = pila->top;
    while (current) {
        my_stack_push(temp_stack, current->data);
        current = current->next;
    }

    // Copiar elementos de la pila temporal a la pila de destino
    while (my_stack_len(temp_stack)>0) {
        my_stack_push(destination, my_stack_pop(temp_stack));
    }

    // Liberar la memoria 
    my_stack_purge(temp_stack);
    free(current);
}


/**
 * Metodo que rellena una pila con punteros a 0 hasta 10 elementos.
 * Imprime también el contenido inicial y después de la pila.
 * 
*/

void rellenarPila(){
   
        //rellenar la pila hasta NUM_THREADS elementos
        int added =0;//elementos a añadir
        while (my_stack_len(pila)<NUM_THREADS){
           int *data =(int*) malloc(sizeInt);
            if (!data){
            perror("Memoria dinámica insuficiente");
            return; 
            }
            //rellenamos los elementos que faltan 
           * data = 0;
            my_stack_push(pila,data);//guardalos en la pila
            added++;
        }

        //imprimir la nueva pila despues del relleno
        //hacemos una copia para no perder el contenido con el pop
    struct my_stack *aux = my_stack_init(sizeInt); 
    copyStack(aux);
    printf("Number of elements added to inital stack: %i\n",added);
    printf("initial stack content for treatment:\n");
        while (my_stack_len(aux)>0){
            int *data = my_stack_pop(aux);
            if (!data){
                perror("Espacio insuficiente en la memoria dinámica");
                return;
            }
            printf("%i \n", *data);

        }
        printf("new stack length: %i\n", my_stack_len(pila));
        //liberar memoria
        my_stack_purge(aux);
}





/**
*Metodo que suma 1 al último valor de la pila.
*Asegura la exclusión mutua.
*@param ptr puntero a void. No lo usamos
*/
void *worker(void *ptr){
    if (!pila){
        perror("out of memory");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i<N; i++){
     //primer sección crítica   
    pthread_mutex_lock(&mutex); 
   // printf("Soy el hilo %ld ejecutando pop\n",pthread_self());
   int *valor = my_stack_pop(pila);
    pthread_mutex_unlock(&mutex);
    //fin de sección crítica

    (*valor)++; 

    //segunda sección crítica
    pthread_mutex_lock(&mutex);
   // printf("Soy el hilo %li ejecutando push\n",pthread_self());
    my_stack_push(pila,valor);
    pthread_mutex_unlock(&mutex);
    //fin de sección crítica
    }

    pthread_exit(NULL);//salir de la función
}

