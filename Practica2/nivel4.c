#define _POSIX_C_SOURCE 200112L
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#define PROMPT "$ "
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define delimitadores " \t\n\r"

#define DEBUGN1 0
#define DEBUGN2 0
#define DEBUGN3 0
#define DEBUGN4 1

#define SIGINT 2
#define SIGKILL 9
#define SIGTERM 15
#define SIGCHLD 17
#define SIGSTOP 19
#define SIGTSTP 20
#define SUCCES 0
#define FAILURE -1
#define N_JOBS 64
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
int internal_source(char **args);
void BorrarChar(char * palabra, char c);
void ctrlc(int signum);
void reaper(int signum);



struct info_job
{
    pid_t pid;
    char estado; // ‘N’, ’E’, ‘D’, ‘F’ (‘N’: Ninguno, ‘E’: Ejecutándose y ‘D’: Detenido, ‘F’: Finalizado)
    char cmd[COMMAND_LINE_SIZE];
};

static struct info_job jobs_list[N_JOBS];

static char mi_shell[COMMAND_LINE_SIZE]; // variable global para guardar el nombre del minishell
void initialize_jobs()
{
    jobs_list[0].pid = 0;
    jobs_list[0].estado = 'N';
    memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
}

/*!*****************************************************************************
 * @brief El main de nuesto shell
 * lea las lineas de comando y las ejecuta.
 * @return 0 
 *******************************************************************************/
int main(int argc, char *argv[]){

char *line = (char *) malloc(sizeof(COMMAND_LINE_SIZE));
    initialize_jobs();
    strcpy(mi_shell, argv[0]);

    signal(SIGCHLD, reaper);//llamada al entrrador de zombies
    signal(SIGINT, ctrlc);//manejar señal de interrupción

    while (1)
    {
        if (read_line(line))
        {
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
        #if DEBUGN3
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


int execute_line(char *line)
{
    char **args = malloc(sizeof(char *) * ARGS_SIZE);
    char tmp[COMMAND_LINE_SIZE] = "";  

    // parsea la línea en argumentos
    if (parse_args(args, line) > 0)
    {
        //reconstruir la linea de comando 
    for (int i = 0; args[i] != NULL; i++)
        {
            strcat(tmp, args[i]);
            if (args[i + 1] != NULL)
            {
                strcat(tmp, " ");
            }
        }
        strcpy(jobs_list[0].cmd,tmp);

        #if DEBUGN2
        check_internal(args);
        #endif
        if (check_internal(args) == 0)
        {
            //si es un comando externo
            pid_t pid = fork();

            if (pid == 0)
            {
                // Asociar la acción por defecto a SIGCHLD
                signal(SIGCHLD, SIG_DFL);
                // ignorar la señal SIGINT
                signal(SIGINT, SIG_IGN);

                execvp(args[0], args);
                perror(ROJO_T "execvp");
                printf(RESET);
                //fprintf(stderr, ROJO_T"%s : comando inexistente\n" RESET, args[0]);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            else if (pid > 0){

                // Proceso padre
                jobs_list[0].pid = pid;
                jobs_list[0].estado = 'E';
                strcpy(jobs_list[0].cmd, tmp);
                #if DEBUGN4
                 // visualización pids del padre e hijo
                fprintf(stderr, GRIS_T"[execute_line()→ PID padre: %i (%s)]\n" RESET, getpid(), mi_shell);
                fprintf(stderr, GRIS_T"[execute_line()→ PID hijo: %i (%s)]\n" RESET, pid, jobs_list[0].cmd);
                #endif
                // espera a que el proceso hijo finalice
                #if DEBUGN3
                int status;
                wait(&status);
                #endif

                while (jobs_list[0].pid > 0) {
                pause();}

                #if DEBUGN3
                printf(GRIS_T "[execute_line()→ Proceso hijo %i (%s) finalizado con exit()]\n" RESET, pid, jobs_list[0].cmd); // mensaje de finalizacion del status
                #endif

                #if DEBUGN3
                // restablece datos del proceso de foreground
                initialize_jobs();
                #endif
            }else {// error para el proceso hijo
                perror("fork");
                return EXIT_FAILURE;
            }
        }
    }
    //liberamos la memoria
    memset(line, '\0', COMMAND_LINE_SIZE);
    free(args);
    return EXIT_SUCCESS;
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
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: export Nombre=Valor\n"RESET);
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
 * @param args Lista de argumentos 
 * @return 0 en caso de succeso.
 *******************************************************************************/


int internal_source(char **args){
    #if DEBUGN1
    printf(GRIS_T NEGRITA"[internal_source() → Esta función ejecutará un fichero de líneas de comandos]\n"RESET);
    #endif
   
    if (args[1] == NULL) {
        fprintf(stderr, ROJO_T"Error de sintaxis. Uso: %s <nombre_fichero>\n" RESET, args[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(args[1], "r");//modo lectura
    if (file == NULL) {
        fprintf(stderr, ROJO_T"fopen: No such file or directory\n"RESET);
        return EXIT_FAILURE;
    }
    char *line = (char *)malloc(COMMAND_LINE_SIZE);

    // Lee el archivo línea por línea y las ejecuta
    while (fgets(line, COMMAND_LINE_SIZE, file) != NULL) {
        
        // Elimina el salto de línea ('\n' por '\0')
        size_t length = strlen(line);
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        fflush(file);
        #if DEBUGN3
                fprintf(stderr, GRIS_T"\n[internal_source()→ LINE: %s]\n" RESET, line);
        #endif  
        if (execute_line(line) != 0) {
            fprintf(stderr, ROJO_T"Error al ejecutar la línea: %s\n"RESET, line);
            fflush(stdout);
        }
    }
    free(line);
    fclose(file);
    return EXIT_SUCCESS;
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
 * @brief Manejador propio para la señal SIGINT (Ctrl+C) 
 *@param signum : numero de señal
 *@return 0
 *******************************************************************************/


void ctrlc(int signum) {
    signal(SIGINT,ctrlc);
#if DEBUGN4
    fprintf(stderr, "\n[ctrlc()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n" RESET,
           getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
#endif


    if (jobs_list[0].pid > 0) { //Hay proceso en foreground?
        if (strcmp(jobs_list[0].cmd, mi_shell)) { // y no es la mini_shell
            //eniviaremos la señal SIGTERM
           kill(jobs_list[0].pid, SIGTERM);
            //y lo notificamos
        #if DEBUGN4
        fprintf(stderr, GRIS_T "[ctrlc()→ Señal %i enviada a %d (%s) por %d (%s)]\n" RESET, SIGTERM, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
        #endif

        }
        else
        { // Si es la mini_shell, no se debe abortar
#if DEBUGN4
            fprintf(stderr, GRIS_T "[ctrlc()→ Señal %i no enviada por %d (%s) debido a que el proceso en foreground es el shell]\n" RESET, SIGTERM, getpid(), mi_shell);
#endif
        }
    } else {    //No hay proceso en foregroud
#if DEBUGN4
        fprintf(stderr, GRIS_T "[ctrlc()→ Señal %i no enviada por %d (%s) debido a que no hay proceso en foreground]\n" RESET, SIGTERM, getpid(), mi_shell);
#endif
    }
    // Limpiamos nuestro flujo de salida
    printf("\n");
    fflush(stdout);
}


/*!*****************************************************************************
 * @brief Función que maneja la terminación de procesos hijos. 
 *@param signum : número de la señal
 *@return 0
 *******************************************************************************/

void reaper(int signum)
{
    signal(SIGCHLD, reaper);
    pid_t ended;
    int status;
     #if DEBUGN4
    fprintf(stderr,GRIS_T"\n[reaper()→ recibida señal %i (SIGCHLD)]\n"RESET,SIGCHLD);
    #endif
    while ((ended = waitpid(-1, &status, WNOHANG)) > 0){
        if (ended == jobs_list[0].pid) 
        { // si el proceso es el del primer plano
            #if DEBUGN4
            char mensaje[1200];
        if ( WIFEXITED(status)){
            // El proceso hijo terminó normalmente
            sprintf(mensaje,GRIS_T "[reaper()→ Proceso hijo %d (%s) finalizado con exit code %d]\n"RESET, ended, jobs_list[0].cmd, WEXITSTATUS(status));
            write(2, mensaje, strlen(mensaje)); //2 es el flujo stderr
            }
            else if (WIFSIGNALED(status))
            {//El proceso hijo termnió al recibir una señal
            sprintf(mensaje,GRIS_T "[reaper()→ Proceso hijo %d (%s) finalizado por señal %d]\n"RESET, ended, jobs_list[0].cmd, WTERMSIG(status));
            write(2, mensaje, strlen(mensaje)); 
            }
            #endif 
            // restablecemos los datos del proceso de foreground
            memset(jobs_list[0].cmd, '\0', strlen(jobs_list[0].cmd));
            jobs_list[0].pid = 0;
            jobs_list[0].estado = 'F';
        }else if (ended == -1){
            perror(ROJO_T"Error");
            printf(RESET);
        }
    }
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
