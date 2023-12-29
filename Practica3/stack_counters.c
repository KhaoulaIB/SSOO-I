#include <pthread.h>
#include "my_lib.h"
#define NUM_THREADS 10
#define  N 1000000

//mutex global
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
//funciones
void *worker(void *ptr);
struct my_stack *pila;

void *worker(void *ptr){
    if (!pila){
        perror("out of memory");
    }
    //añadir control de errores
    for (int i = 0; i<N; i++){

    pthread_mutex_lock(&mutex); 
    int tmp =my_stack_pop();
    pthread_mutex_unlock(&mutex);
    tmp++; 
    pthread_mutex_lock(&mutex);
    my_stack_push(pila,tmp);
    pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);//salir de la función
}


