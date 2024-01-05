#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
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


    // Verificar si la pila ya existe
    pila = my_stack_read(argv[1]);
    printf("Threads: %d, Iterations: %d\n", NUM_THREADS, N);

    stack_init(argv[1]);

    // Crear los hilos
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        //thread_ids[i] = i;
        pthread_create(&threads[i], NULL, worker, NULL);
        printf("%d) Thread %ld created\n", i,threads[i]);
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("stack content after threads iterations\n");

    // Volcar la pila en un fichero
    int elements_written = my_stack_write(pila, argv[1]);
    printf("Written elements from stack to file: %d\n", elements_written);

    // Liberar la memoria de la pila
    int bytes_released = my_stack_purge(pila);
    printf("Released bytes: %d\n", bytes_released);


    // Finalizar el hilo principal
    printf("Bye from main\n");
    pthread_exit(NULL);

    return 0;
}


void stack_init(char* file){
    pila = my_stack_read(file);
    if (!pila){//si la pila no existe, la creamos
        pila = my_stack_init(sizeInt);
        printf("size of inicialized stack :  %i\n",pila->size);

    }
    
    if (my_stack_len(pila)<NUM_THREADS){
        rellenarPila();
    }


}


void rellenarPila(){
    int pos = my_stack_len(pila);
    struct my_stack *aux = my_stack_init(sizeInt); // Inicializa la pila auxiliar
    struct my_stack_node *current = pila->top;
    //copiar los elementos de la pila en aux en el mismo orden
    while (current)
    {                                      // mientras hay elementos en la pila
        my_stack_push(aux, current->data); // Copia el elemento top en la pila aux
        current = current->next;           // Avanza al siguiente elemento en la pila original
    }


        int * data =malloc(sizeInt);
        if (!data){
            perror("Memoria dinámica insuficiente");
            return;
        }


        printf("stack->size:%i\n",pila->size);

        printf("initial stack length: %i\n",pos);

        printf("initial stack content:\n");

        //mostrar el contenido inicial de la pila
        while (my_stack_len(aux)>0){
            data =(int*) malloc(sizeInt);
            if (!data){
            perror("Memoria dinámica insuficiente");
            return; 
            }
            data = my_stack_pop(aux);
            printf("%i \n",*data);
        }


        //rellenar la pila hasta NUM_THREADS elementos
        while (my_stack_len(pila)<NUM_THREADS){
            data =(int*) malloc(sizeInt);
            if (!data){
            perror("Memoria dinámica insuficiente");
            return; 
            }
            //rellenamos los elementos que faltan 
           * data = 0;
            my_stack_push(pila,data);
        }

        //imprimir la nueva pila despues del relleno

        //hacemos una copia para no perder el contenido con el pop

        aux = my_stack_init(sizeInt);
         current = pila->top;
    //copiar los elementos de la pila ne aux en el mismo orden
    while (current)
    {                                      // mientras hay elementos en la pila
        my_stack_push(aux, current->data); // Copia el elemento top en la pila aux
        current = current->next;           // Avanza al siguiente elemento en la pila original
    }

 printf("initial stack content for treatment:\n");
        while (my_stack_len(aux)>0){
            data = (int*)malloc(sizeInt);
            if (!data){
                perror("Espacio insuficiente en la memoria dinámica");
            }
            data = my_stack_pop(aux);
            printf("%i \n", *data);

        }
        printf("new stack length: %i\n", my_stack_len(pila));
        //liberar memoria
      //  free(data);


}


/**
*Metodo que suma 1 al último valor de la pila.
Asegura la exclusión mutua.
*@param ptr puntero a void. No lo usamos
*/
void *worker(void *ptr){
    if (!pila){
        perror("out of memory");
    }
    for (int i = 0; i<N; i++){
    pthread_mutex_lock(&mutex); 
   
   // printf("Soy el hilo %ld ejecutando pop\n",pthread_self());
   int *valor = my_stack_pop(pila);
    pthread_mutex_unlock(&mutex);
    (*valor)++; 
    pthread_mutex_lock(&mutex);
   // printf("Soy el hilo %li ejecutando push\n",pthread_self());

    my_stack_push(pila,valor);
    pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);//salir de la función
}

