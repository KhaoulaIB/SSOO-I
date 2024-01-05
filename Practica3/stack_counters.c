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

/ Mutex global
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Semáforo global
sem_t semaforo;

// Funciones
void *worker(void *ptr);

// Puntero a la pila
struct my_stack *pila;

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

    // Si no existe, crearla e inicializarla
    if (!pila)
    {
        // Inicializar el semáforo
        sem_init(&semaforo, 0, 1);

        // Crear la pila (inicializada con punteros a 0’s)
        pila = my_stack_init(sizeof(int));
        for (int i = 0; i < NUM_THREADS; i++)
        {
            int *data = (int *)malloc(sizeof(int));
            if (!data)
            {
                perror("Memoria insuficiente");
                exit(EXIT_FAILURE);
            }
            *data = 0;
            my_stack_push(pila, data);
        }
    }

    // Verificar si la pila tiene menos de 10 elementos, agregar los restantes
    while (my_stack_len(pila) < 10)
    {
        int *data = (int *)malloc(sizeof(int));
        if (!data)
        {
            perror("Memoria insuficiente");
            exit(EXIT_FAILURE);
        }
        *data = 0;
        my_stack_push(pila, data);
    }

    // Crear los hilos
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    printf("Threads: %d, Iterations: %d\n", NUM_THREADS, N);
    printf("stack->size: %d\n", pila->size);
    printf("original stack length: %d\n", my_stack_len(pila));
    printf("new stack length: %d\n", my_stack_len(pila));

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_ids[i] = i;
        printf("%d) Thread %ld created\n", i, (long)pthread_self());
        pthread_create(&threads[i], NULL, worker, (void *)&thread_ids[i]);
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Volcar la pila en un fichero
    int elements_written = my_stack_write(pila, argv[1]);
    printf("Written elements from stack to file: %d\n", elements_written);

    // Liberar la memoria de la pila
    int bytes_released = my_stack_purge(pila);
    printf("Released bytes: %d\n", bytes_released);

    // Liberar el semáforo
    sem_destroy(&semaforo);

    // Finalizar el hilo principal
    printf("Bye from main\n");
    pthread_exit(NULL);

    return 0;
}

void *worker(void *ptr)
{
    int thread_id = *((int *)ptr);

    for (int i = 0; i < N; i++)
    {
        // Esperar a que el semáforo esté disponible
        sem_wait(&semaforo);

        pthread_mutex_lock(&mutex);

        // Mostrar el hilo actual y la operación que está realizando
        printf("Soy el hilo %ld ejecutando pop\n", pthread_self());

        int *data = my_stack_pop(pila);

        pthread_mutex_unlock(&mutex);

        (*data)++;

        pthread_mutex_lock(&mutex);

        // Mostrar el hilo actual y la operación que está realizando
        printf("Soy el hilo %ld ejecutando push\n", pthread_self());

        my_stack_push(pila, data);

        pthread_mutex_unlock(&mutex);

        // Liberar el semáforo para indicar que la sección crítica ha terminado
        sem_post(&semaforo);
    }

    // Salir del hilo
    pthread_exit(NULL);
}


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


