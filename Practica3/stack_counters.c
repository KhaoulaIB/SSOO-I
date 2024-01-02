#include <pthread.h>
#include "my_lib.h"
#define NUM_THREADS 10
#define  N 1000000

//mutex global
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
//funciones
void *worker(void *ptr);

//Preparar la pila

//Crear los hilos



struct my_stack *pila;
//semaforo global
sem_t semaforo;

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
    // Esperar a que el semáforo esté disponible
    sem_wait(&semaforo);    
    pthread_mutex_lock(&mutex); 
    int *data = (int *) mallorc(sizeof(int));
        if (!data){
            perror("Memoria insuficiente");
            pthread_exit(NULL);//salir de la función
        }
    data = my_stack_pop(pila);
    pthread_mutex_unlock(&mutex);
    (*data)++; 
    pthread_mutex_lock(&mutex);
    my_stack_push(pila,data);
    pthread_mutex_unlock(&mutex);
    // Liberar el semáforo para indicar que la sección crítica ha terminado
        sem_post(&semaforo);    
    }

    pthread_exit(NULL);//salir de la función
}


