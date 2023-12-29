#include <pthread.h>
#include "my_lib.h"
#define NUM_THREADS 10
#define  N 1000000

//mutex global
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
//funciones
void *worker(void *ptr);

struct my_stack *pila;

/**
*Metodo que suma 1 al último valor de la pila.
Asegura la exclusión mutua.
*@param ptr puntero a void. No lo usamos
*/
void *worker(void *ptr){
    if (!pila){
        perror("out of memory");
    }
    //añadir control de errores
    for (int i = 0; i<N; i++){

    pthread_mutex_lock(&mutex); 
    int *data =my_stack_pop(pila);
    pthread_mutex_unlock(&mutex);
    (*data)++; 
    pthread_mutex_lock(&mutex);
    my_stack_push(pila,data);
    pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);//salir de la función
}


