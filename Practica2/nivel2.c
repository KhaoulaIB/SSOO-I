
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#define PROMPT "$ "
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define DEBUGN1 0
#define DEBUGN2 1
#define SUCCES 0
#define FAILURE -1
#define delimitadores " \t\n\r"
#define RESET "\033[0m"
#define NEGRO_T "\x1b[30m"
#define NEGRO_F "\x1b[40m"
#define GRIS_T "\x1b[90m"
#define ROJO_T "\x1b[31m"
#define VERDE_T "\x1b[32m"
#define AMARILLO_T "\x1b[33m"
#define AZUL_T "\x1b[34m"
#define MAGENTA_T "\x1b[35m"
#define CYAN_T "\x1b[36m"
#define BLANCO_T "\x1b[97m"
#define NEGRITA "\x1b[1m"


char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);

int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
int internal_fg(char **args);
int internal_bg(char **args);
void BorrarChar(char * palabra, char c);


/*!*****************************************************************************
 * @brief El main de nuesto shell
 * lea las lineas de comando y las ejecuta.
 * @return 0 
 *******************************************************************************/
int main() {

char *line = (char *) malloc(sizeof(COMMAND_LINE_SIZE));
    while (1) {
        if (read_line(line)) {
            execute_line(line);
         
        }
    }
    
    return 0;

    
}




/*!*****************************************************************************
 * @brief Imprime el prompt de nuestro shell.
 * Utiliza USER para obtener el nombre de usuario.
 * Y getcwd para el directorio de trabajo actual.
 *******************************************************************************/
void imprimir_prompt() {
    char *user = getenv("USER");
    char *pwd = getcwd(NULL, 0);
    printf(ROJO_T NEGRITA "%s" RESET ":" AMARILLO_T NEGRITA "%s" RESET NEGRITA "%s" RESET , user,pwd,PROMPT);
    free(pwd);
}


/*!*****************************************************************************
 * @brief Función para leer una línea desde la consola.
 * Remplaza '\n' con '\0' al final y maneja el caso de Ctrl+D. 
 * @param line   linea de comandos
 * @return  puntero a la linea
 *******************************************************************************/
char *read_line(char *line) {
    imprimir_prompt();
    fflush(stdout);
    sleep(0.5); // Esperar 0.5 segundos
    if (fgets(line, COMMAND_LINE_SIZE, stdin) == NULL) {
        printf("\r");
        if (feof(stdin)){
        // Ctrl+D fue presionado
        #if DEBUGN2
        printf("\nAdiós!\n");
        #endif
        exit(EXIT_SUCCESS);
        }  
    }
        // Reemplazar el carácter '\n' por '\0'
     size_t length = strlen(line);
        if (line[length-1] == '\n') {
            line[length-1] = '\0';
        }
            return line;
}



/*!*****************************************************************************
 * @brief Ejecuta la linea de comando. 
 @param line : linea a ejecutar
 @return 0
 *******************************************************************************/

int execute_line(char *line) {
   char **args = malloc(sizeof(char *) * ARGS_SIZE);
    if (parse_args(args, line) > 0) {  
          //parsea la línea en argumentos
        check_internal(args);
    }
    memset(line, '\0', COMMAND_LINE_SIZE);
    free(args);
    return 0;
}


/*!*****************************************************************************
 * @brief Trocea la línea de comando en tokens y los guarda en args.
 *Ignora los comentarios, y el ultimo token lo pone a NULL. 
 *@param line   linea de comando a trocear
 *@param args   Lista para guardar los argumentos 
 *@return número de tokens sin contar el NULL
 *******************************************************************************/

int parse_args(char **args, char *line) {
    int numTokens = 0;

    args[numTokens] = strtok(line, delimitadores);
#if DEBUGN1
    fprintf(stderr, GRIS_T "[parse_args()→ token %i: %s]\n" RESET , numTokens, args[numTokens]);
#endif
    while (args[numTokens] && args[numTokens][0] != '#')
    { 
        numTokens++;
        args[numTokens] = strtok(NULL, delimitadores);
#if DEBUGN1
        fprintf(stderr,  GRIS_T "[parse_args()→ token %i: %s]\n" RESET , numTokens, args[numTokens]);
#endif
    }
    if (args[numTokens]) {
        args[numTokens] = NULL; // por si el último token es el símbolo comentario
#if DEBUGN1
        fprintf(stderr,  GRIS_T"[parse_args()→ token %i corregido: %s]\n" RESET, numTokens, args[numTokens]);
#endif
    }
    return numTokens;
}


/*!*****************************************************************************
 * @brief Cambia el directorio de trabajo.
 *Se tractan 3 casos 
 * cd → se traslada a HOME.
 * cd + argumento → se traslada al directorio
 * cd + argumentos→se obteniene la ruta completa. 
 * @param args Lista de argumentos 
 * @return 0 si el cambio fue correcto. 1 en otro caso.
 *******************************************************************************/

int internal_cd(char **args) {
     #if DEBUGN1
    printf(GRIS_T NEGRITA"[internal_cd() → Esta función cambiará de directorio]\n"RESET);
    #endif
    char *home_dir = getenv("HOME");
    char *target_dir;

    if (args[1] == NULL) {      
        // Si no hay argumentos, cambiar al directorio HOME
        target_dir = home_dir;
    } else if (args[1] && args[2] == NULL) {
        // Si hay un argumento, cambiar al directorio especificado
        target_dir = args[1];
    } else {
        // Caso avanzado: se han proporcionado más de 1 argumento
        int total_length = 0;
        for (int i = 1; args[i]; i++) {
            total_length += strlen(args[i]);
        }

        target_dir = (char *)malloc(total_length + 1);
        target_dir[0] = '\0';  // Inicializar la cadena 

        // Concatenar los argumentos para formar la ruta completa
        for (int i = 1; args[i]; i++) {
            strcat(target_dir, args[i]);
            if (args[i + 1]) {
                strcat(target_dir, " ");
            }
        }

        int size = strlen(target_dir)-1;
        //caso de comillas dobles 
        if (target_dir[0] == '\"' && target_dir[size] == '\"') {
             BorrarChar(target_dir,'\"');
        //caso de comillas simples 
        }else if  (target_dir[0] == '\'' && target_dir[size] == '\'') {
          
          BorrarChar(target_dir,'\'');
        //caso de '\' 
        } else if (strchr(target_dir, '\\') != NULL) {
           BorrarChar(target_dir,'\\');
        } else {
            fprintf(stderr,ROJO_T NEGRITA"cd: Too much arguments\n"RESET);
            return EXIT_FAILURE;
        }
    
    }

    if (chdir(target_dir) != 0) {
        perror(ROJO_T"chdir() error");
        return EXIT_FAILURE;
    }
    
    char current_dir[ARGS_SIZE];
    if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
        #if DEBUGN2
        fprintf(stderr,GRIS_T NEGRITA"[internal_cd()→ PWD: %s]\n"RESET, current_dir);
        #endif
    } else {
        perror("getcwd");
    }

    return EXIT_SUCCESS;
}


/*!*****************************************************************************
 * @brief Borra el caracter c de la palabra.
 *@param palabra palabra a tractar   
 *@param  c caracter a borrar de la palabra  
 *
 *******************************************************************************/
void BorrarChar(char * palabra, char c){
    int index = 0;
    int tmp = 0;
    while (palabra[index]){

        if (palabra[index]!=c){
            palabra[tmp]=palabra[index];
            tmp++;
        }
        index++;
    }
    palabra[tmp]='\0';//MARCAR FIN DE CADNA

}

/*!*****************************************************************************
 * @brief Asigna valores a variablescd de entorno.
 * Obtiene el nombre de la variable y su valor y hace la asignación.
 * Se hace control de errores de sintaxis y de asignación.
 * @param args Lista de argumentos 
 * @return  0 si la asignación fue exsitosa. 1 en caso contario.
 *******************************************************************************/

int internal_export(char **args) {
    #if DEBUGN1
    printf(GRIS_T NEGRITA"[internal_export() → Esta función asignará valores a variablescd de entorno]\n"RESET);
    #endif
    if (!args[1]) {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return EXIT_FAILURE;
    }
   char * nombre= (char *)malloc(sizeof(COMMAND_LINE_SIZE));
    char * valor = (char *)malloc(sizeof(COMMAND_LINE_SIZE));

    nombre = strtok(args[1],"=");
    valor = strtok(NULL,delimitadores);

     #if DEBUGN2
    fprintf(stderr,GRIS_T"[internal_export()→ nombre: %s]\n"RESET,nombre);
    fprintf(stderr,GRIS_T"[internal_export()→ valor: %s]\n"RESET,valor);
    #endif
    if (!nombre || !valor){
         fprintf(stderr, ROJO_T "Error de sintaxis. Uso: export Nombre=Valor\n"RESET);
        return EXIT_FAILURE;
    }
   
    #if DEBUGN2
    fprintf(stderr,GRIS_T "[internal_export()→ antiguo valor para %s: %s]\n" RESET, nombre, getenv(nombre));
    #endif
    if (setenv(nombre, valor, 1) < 0) {
        #if DEBUGN2
        fprintf(stderr, "Error al cambiar el nombre de la variable de entorno\n");
        #endif
        return EXIT_FAILURE;
    }
    #if DEBUGN2
    fprintf(stderr,GRIS_T "[internal_export()→ nuevo valor para %s: %s]\n" RESET, nombre, getenv(nombre));
    #endif
    return EXIT_SUCCESS;
}



/*!*****************************************************************************
 * @brief Ejecuta un fichero de líneas de comandos.
 * En este nivel, la función simplemente imprime su funcionamiento.
 * @param args Lista de argumentos (actualmente no utilizada).
 * @return 0
 *******************************************************************************/

int internal_source(char **args){
    #if DEBUGN1
    printf(GRIS_T NEGRITA"[internal_source() → Esta función ejecutará un fichero de líneas de comandos]\n"RESET);
    #endif
    return 0;
}

/*!*****************************************************************************
 * @brief Muestra el PID de procesos en background(segundo plano)
 * @param args Lista de argumentos (actualmente no utilizada).
 * @return 0
 *******************************************************************************/

int internal_jobs(char **args){
    #if DEBUGN1
    printf(GRIS_T NEGRITA"[internal_jobs() → Esta función mostrará el PID de los procesos que no estén en foreground]\n"RESET);
    #endif
    return 0;
}

/*!*****************************************************************************
 * @brief Envia a segundo plano el trabajo indicado con su índice.
 * En este nivel, la función simplemente imprime su funcionamiento.
 * @param args Lista de argumentos (actualmente no utilizada).
 * @return 0
 *******************************************************************************/

int internal_bg(char **args){
   #if DEBUGN1 
   printf(GRIS_T NEGRITA"[internal_bg() →  Esta función envia a segundo plano el trabajo indicado con su Índice"RESET);
   #endif
    return 0; 
}

/*!*****************************************************************************
 * @brief Envia a primer plano el trabajo indicado con su índice.
 * En este nivel, la función simplemente imprime su funcionamiento.
 * @param args Lista de argumentos (actualmente no utilizada).
 * @return 0
 *******************************************************************************/

int internal_fg(char **args){
   #if DEBUGN1
   printf(GRIS_T NEGRITA"[internal_fg() →  Esta función envia a primer plano el trabajo indicado con su índice"RESET);
   #endif
    return 0; 
}



/*!*****************************************************************************
 * @brief Comprueba si args[0] es comando interno o externo.
 * En los comandos internos llama a la función que le corresponde.
 * @param args  lista de argumentos 
 * @return 1 si el comando es interno. O en caso contrario.
 *******************************************************************************/

int check_internal(char **args){
   int internal = 1;
  
    if (strcmp(args[0], "cd") == 0){ internal_cd(args);}
    else if (strcmp(args[0], "export") == 0){ internal_export(args);}
    else if (strcmp(args[0], "source") == 0){internal_source(args);}
    else if (strcmp(args[0], "jobs") == 0){internal_jobs(args);}
    else if (strcmp(args[0], "fg") == 0){internal_fg(args);}
    else if (strcmp(args[0], "bg") == 0){internal_bg(args);}
    else if (strcmp(args[0], "exit") == 0){
        #if DEBUGN1
        printf("\n Bye Bye\n");
        #endif
        exit(0);
    }else{internal=0;}
    return internal;

}






