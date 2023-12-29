//@authors : Khaoula Ikkene, Martina Kalin Georgieva, Gaizka Medina Gordo 
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "my_lib.h"


/* Calcula el nº de bytes de la cadena apuntada por str,sin incluir el caracter
 nulo de terminación.
 @param str  puntero constante a la cadena
 @return size_t  el nº de bytes de &str
*/
size_t my_strlen(const char *str)
{

    int size = 0, index = 0;
    while (str[index] != '\0') // sin incluir el caracter nulo terminativo
    {
        index++; // incrementar el indice del string
        size++;  // incrementar el tamaño
    }
    return size;
}

/*
    Compara &str1 y &str2 y devuelve un entero :
    <0 si str1<str2
    >0 si str1>str2
    =0 si str1=str2
    @param str1 puntero a la primera palabra
    @param str2 puntero a la segunda palabra

    @return la resta de los primeros caracteres diferentes encontrados

*/
int my_strcmp(const char *str1, const char *str2)
{ // compara 2 strings y devuelve la resta del primer caracter diferente
    while (*str1 != '\0' && *str2 != '\0')
    {
        if (*str1 != *str2)
        { // si encuentra el primer cracter diferente
            return (*str1 - *str2);
        } // mientras tanto incrementa los punteros de las palabras
        str1++;
        str2++;
    }
    return (*str1 - *str2);
}

/*
Copia &src en la posición apuntada por dest y devuelve dest (el puntero)
@param dest puntero a la posicion de memoria donde se guardará la palabra
@param src  puntero a la palabra que se copia en la memoria
@return char  devuelve el puntero a dest
*/
char *my_strcpy(char *dest, const char *src)
{
    int n = 0;
    while (src[n] != '\0')
    { // mientras quedan caracteres a copiar
        dest[n] = src[n];
        n++;
    }
    dest[n] = '\0'; // marcar el fin de la palabra
    return dest;
}

/*
Copia n caracteres de la cadena &src en la memoria apuntada
por dest.
@param dest  puntero a la palabra destino
@param src   puntero a la palabra a copiar
@param n numero de caracteres a copiar de src en dest
@return char  puntero a la palabra de destino

*/

char *my_strncpy(char *dest, const char *src, size_t n)
{
    int i;

    // Copia hasta n caracteres de src a dest
    for (i = 0; i < n && src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }

    // Llena el resto con '\0' si es necesario
    for (; i < n; i++)
    {
        dest[i] = '\0';
    }

    return dest; // devuelve el puntero
}

/*
Concatena las palabras &src y &dest 
@param dest puntero de la palabra destino
@param src  puntero de la palabra a concatenar con &dest
@return  puntero a dest
*/

char *my_strcat(char *dest, const char *src)
{
    int index = 0;
    while (dest[index] != '\0')
    { // incrementamos el index  hasta llegar al caracter terminativo de dest
        index++;
    }
    int i = 0;
    // sobreescribe el caracter nulo
    while (src[i] != '\0')
    { // concatena caracter por caracter de la palabra src a la palabra dest
        dest[index] = src[i];
        index++;
        i++;
    }

    dest[index] = '\0'; // marcar el fin de la palabra destino

    return dest;



}

/*
    Devuelve la primera ocurrencia del caracter c. Si no existe devuelve NULL
    @param str  puntero a la palabra
    @param c el caracter a buscar en str
    @return char devuelve el puntero a la 1a ocurrencia de c

*/
char *my_strchr(const char *str, int c)
{
    while (*str != '\0')
    { // procesar todos los carcateres de la palabra
        if (*str == c)
        {
            return (char *)str; // Realiza un casting al devolver el puntero
        }
        str++; // avanzar en la palabra
    }
    return NULL;
}

/*
Inicializa la pila reservandole espacio con malloc 
@param size  tamaño que se asignara al atributo size de la pila
@return struct my_stack puntero de la pila
*/

struct my_stack *my_stack_init(int size)
{
    // Reservar espacio para variable de tipo struct my_stack
    struct my_stack *stack = malloc(sizeof(struct my_stack)); 
    if (!stack){
        fprintf(stderr,"Out of memory");
    }
    stack->size = size;
    stack->top = NULL; // el top inicialemnte apunta a NULL
    return stack;
}

/*
Inserta un nuevo elemento no nulo (stack->size>0) en la pila
@param stack puntero a la pila a la que se inserta el nuevo elemento
@ param data  puntero a la data del nodo
@return int 0 si se insertó correctamente el elemento, -1 en caso contrario.

*/

int my_stack_push(struct my_stack *stack, void *data)
{
    // Comprobación 1: Verifica si la pila no es nula y el tamaño es positivo
    if (!stack || stack->size <= 0)
    {
        fprintf(stderr,"Pila no válida ");
        return -1; 
    }

    // Reserva espacio en memoria para el nuevo nodo
    struct my_stack_node *nuevo_nodo = malloc(sizeof(struct my_stack_node));
    // Comprobación 2: Verifica si la asignación de memoria fue exitosa
    if (!nuevo_nodo)
    {
        fprintf(stderr, "Out of memory.\n");
        return -1;
    }

    // Asigna los datos al nuevo nodo
    nuevo_nodo->data = data;

    // Configura el nuevo nodo como el siguiente (top) de la pila
    nuevo_nodo->next = stack->top;

    // Actualiza el puntero 'top' de la pila para apuntar al nuevo nodo
    stack->top = nuevo_nodo;

    return 0; // Éxito: Nuevo elemento agregado a la pila
}


/*
Elimina el nodo superior de los elementos de la pila.
Y libera la memoria que ocupaba ese nodo.
Devuelve el puntero al dato eliminado.
@param stack puntero de la pila

*/
void *my_stack_pop(struct my_stack *stack)
{
    if (stack == NULL || stack->top == NULL)
    {
        fprintf(stderr,"Pila vacía");
        return NULL; 
    }

    // Guarda el puntero al nodo superior de la pila
    struct my_stack_node *top_node = stack->top;

    // Guarda el puntero a los datos del nodo superior
    void *data = top_node->data;

    // Actualiza el puntero a la parte superior de la pila
    stack->top = top_node->next;

    // Libera el nodo superior
    free(top_node);

    // Devuelve el puntero a los datos del nodo eliminado
    return data;
}

/*
Devuelve el numero de nodos de la pila
@param stack  puntero de la pila
@return contador numero de nodos de stack
*/

int my_stack_len(struct my_stack *stack)
{

 if (stack==NULL){
  return 0; //Pila vacía
 }
    // inicializar el contador
    int contador = 0;
    // puntero al elemento top del stack
    struct my_stack_node *nodo_actual = stack->top;

    while (nodo_actual)
    {
        contador++;
        // avanzar el top
        nodo_actual = nodo_actual->next;
    }

    return contador;
}

/*
Libera la memoria ocupada por los nodos, sus punteros y la pila misma.
@param stack  puntero a la pila
@return BytesLiberados  numero de bytes liberados


*/
int my_stack_purge(struct my_stack *stack)
{
    int BytesLiberados = sizeof(struct my_stack); //tamaño de la pila
    struct my_stack_node *current = stack->top;

   if(stack == NULL){ //pila vacía
    return 0;
   }
    while (current)
    {
        // Incrementa BytesLiberados por el tamaño de struct my_stack_node y el tamaño del dato
        BytesLiberados += sizeof(struct my_stack_node) + (stack->size);

        // Libera la memoria del dato
        free(my_stack_pop(stack));
        // avanzar en la pila ya que el top anterior se liberó
        current = stack->top;
    }

    // Libera la memoria de la estructura my_stack
    free(stack);

    // Devuelve el número total de bytes liberados
    return BytesLiberados;
}

/*
Almacena los datos de la pila en un fichero. En caso de error se devuelve -1.
@param stack  puntero a la pila
@param filname  nombre del fichero donde guardar la pila
@return n  el numero de elementos almacenados en el fichero
*/
int my_stack_write(struct my_stack *stack, char *filename)
{
    // Usaremos una pila auxiliar para invertir los elementos de la pila.
    struct my_stack *aux = my_stack_init(stack->size); // Inicializa la pila auxiliar
    struct my_stack_node *current = stack->top;

    while (current)
    {                                      // mientras hay elementos en la pila
        my_stack_push(aux, current->data); // Copia el elemento top en la pila aux
        current = current->next;           // Avanza al siguiente elemento en la pila original
    }

    // Abrir el fichero y  controlar los errores que puede dar esta operación
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        perror("Error al abrir el fichero");
        return -1;
    }

// escribri el tamaño de los datos en el fichero
    if (write(fd, &stack->size, sizeof(stack->size)) < 0)
    { 
        perror("Error al escribir el tamaño en el fichero");
        return -1;
    }

    int n = 0;
    while (aux->top){
        //escribir dato por dato en filname aprovechando el pop de la pila
        int bytes = write(fd, my_stack_pop(aux), stack->size);
        if (bytes < 0)
        {
            perror("Error al escribir datos");
            close(fd);
            return -1;
        }

        n++; //incrementar el numero de elementos almacenados
    }

    if (close(fd)<0){ // cerrar el fichero
     perror("Error al cerrar el fichero);
      return -1;
      }
    // librar la pila auxiliar
    free(aux);
    // Devolver el numero de bytes almacenados
    return n;
}




struct my_stack *my_stack_read(char *filename)
{

    // Abrir el fichero
    int fd = open(filename, O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        perror("Se ha prodocido un error al abrir el fichero");
        close(fd);
        return NULL;
    }
    int size;
    // Leer tamaño de datos y almacenarlo en 'size' en memoria.

    if (read(fd, &size, sizeof(int)) < 0)
    {
        perror("Error al leer el tamaño de los datos");
        close(fd);
        return NULL;
    }
    // incializar el stack
    struct my_stack *Stack = my_stack_init(size);
    if (!Stack){
        perror("No se pudo inicializar la pila");
        close(fd);
        return NULL;

    }
    // Reservar espacio para el dato con malloc
    void *data = malloc(size);
    if (data < 0)
    {
        perror("Espacio insuficiente en la memoria");
       close(fd);
        return NULL;
    }
    while (read(fd, data, size) > 0)
    {
        // agregamos el dato a la pila
       if ( my_stack_push(Stack, data)<0){
        perror("No se pudo agregar un elemento a la pila");
            close(fd);
            free(data);
            my_stack_purge(Stack);
            return NULL;

       }
        // reservamos espacio para el seguiente
        data = malloc(size);
        // verificar que no se ha producido ningun error
        if (data < 0)
        {
            perror("Espacio insuficiente en la memoria");
            return NULL;
        }
    }

    // cerrar el fichero y verificar que se ha cerrado correctamente
    if (close(fd) <0)
    {
        perror("El fichero no se ha cerrado correctamente");
        return NULL;
    }
    //liberar el espacio del puntero
    free(data);
    // devolver el puntero a la pila
    return Stack;
}

