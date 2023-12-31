
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <stddef.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#define PROMPT "$ "
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define delimitadores " \t\n\r"

#define DEBUGN1 0
#define DEBUGN2 0
#define DEBUGN3 0
#define DEBUGN4 1
#define DEBUGN5 1
#define SIGCHLD 17
#define SIGINT  2
#define SIGKILL 9
#define SIGSTOP 19
#define SIGTERM 15
#define SIGTSTP 20
#define SUCCES 0
#define FAILURE -1
#define TRUE    1   
#define FALSE 0
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

int is_background(char **args);
int execute_line(char *line); 
int jobs_list_add(pid_t pid, char estado, char *cmd);
int  jobs_list_remove(int pos);
void ctrlz(int signum);
void reaper(int signum);
void ctrlc (int signum);
char *read_line(char *line);
int internal_fg(char **args);
int jobs_list_find(pid_t pid);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_jobs(char **args);
int internal_fg(char **args);
int internal_bg(char **args);
int internal_source(char **args);
void BorrarChar(char * args, char c);


struct info_job{
    pid_t pid;
    char estado; 
    char cmd[COMMAND_LINE_SIZE];

};


int n_job =0;
static struct info_job jobs_list [N_JOBS]; 
static char mi_shell[COMMAND_LINE_SIZE];
void initialize_jobs(){
    jobs_list[0].pid = 0;
    jobs_list[0].estado = 'N';
    memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
}

/*!*****************************************************************************
 * @brief El main de nuesto shell
 * Lea las lineas de comando y las ejecuta.
 * @return 0 
 *******************************************************************************/
int main(int argc, char *argv[]) {

char *line = (char *) malloc(sizeof(COMMAND_LINE_SIZE));
  initialize_jobs();
  strcpy(mi_shell,argv[0]);
        signal(SIGINT, ctrlc); 
        signal(SIGCHLD, reaper);               
        signal(SIGTSTP, ctrlz);

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


/*!*****************************************************************************
 * @brief Ejecuta la linea de comando. 
 *Si es un comando externo lo ejecuta un proceso hijo.
 *@param line : linea a ejecutar
 *@return 0 si la ejecución fue correcta
 *******************************************************************************/
int execute_line(char *line) {
   char **args = malloc(sizeof(char *) * ARGS_SIZE);
    char tmp[COMMAND_LINE_SIZE] = ""; 
    if (args ==NULL){
        fprintf(stderr,ROJO_T "No hay suficiente memoria dinamica!\n"RESET);
    }

    //parsea la línea en argumentos
    if (parse_args(args, line) > 0) {
            #if DEBUGN2
            check_internal(args);
            #endif

            //reconstruir la linea de comando
    for (int i = 0; args[i] != NULL; i++) {
            strcat(tmp, args[i]);
            if (args[i + 1] != NULL)
            {
                strcat(tmp, " ");
            }
        }

        if(check_internal(args) == 0){
            bool background = is_background(args);

            //ejecuta el comando externo usando un proceso hijo
            pid_t pid = fork();

            if(pid == 0){
                //Asociar la acción por defecto a SIGCHLD.
                signal(SIGCHLD,SIG_DFL);
                // ignorar la señal SIGINT
                signal(SIGINT,SIG_IGN);
                // ignorar la señal SIGTSTP
                signal(SIGTSTP,SIG_IGN);

                execvp(args[0], args);
               // printf(ROJO_T"%s : comando inexistente\n"RESET,args[0]);
                perror(ROJO_T"execvp");
                printf(RESET);
                exit(EXIT_FAILURE);
            }else if(pid > 0){
                //proceso en background
                if (background==1){

                jobs_list_add(pid,'E',tmp);

                 #if DEBUGN4
                //visualización pids del padre e hijo
                fprintf(stderr, GRIS_T"[execute_line()→ PID padre: %i (%s)]\n" RESET, getpid(), mi_shell);
                fprintf(stderr, GRIS_T"[execute_line()→ PID hijo: %i (%s)]\n" RESET, pid, jobs_list[n_job].cmd);
                #endif
                #if DEBUGN5
                fprintf(stderr,"[%i] %d \t%c \t%s \n", n_job, jobs_list[n_job].pid, jobs_list[n_job].estado, jobs_list[n_job].cmd);
                #endif
              
                #if DEBUGN3
                int status;
                wait(&status);
                #endif
                //proceso en foreground
                } else{ 
                 //actualizar los datos de jobs_list[0]       
                jobs_list[0].pid = pid;
                jobs_list[0].estado = 'E';
                strcpy(jobs_list[0].cmd,tmp);

                #if DEBUGN4
                //visualización pids del padre e hijo
                fprintf(stderr, GRIS_T"[execute_line()→ PID padre: %i (%s)]\n" RESET, getpid(), mi_shell);
                fprintf(stderr, GRIS_T"[execute_line()→ PID hijo: %i (%s)]\n" RESET, pid, jobs_list[0].cmd);
                #endif

                while (jobs_list[0].pid>0){
                pause(); }
               
               #if DEBUGN3
                //restablece datos del proceso de foreground
                initialize_jobs();
                #endif
                }
                
            }else{
                //error para el proceso hijo
                perror("fork");
                return EXIT_FAILURE;
            
        }   
            }
    }  
    //liberar memoria
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
 * @brief Manejador propio para la señal SIGINT (Ctrl+C) 
 *@param signum numero de la señal
 *@return 0
 *******************************************************************************/

void ctrlc(int signum) {
    signal(SIGINT,ctrlc);
    #if DEBUGN4
    char mensaje[3000];
    fprintf(stderr, "\n[ctrlc()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n" RESET,
           getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
#endif
#if DEBUGN5
    fprintf(stderr,"\n[ctrlc()→ recibida señal 2 (SIGINT)]\n"RESET);
#endif

    if (jobs_list[0].pid > 0) { //Hay proceso en foreground?
        if (strcmp(jobs_list[0].cmd, mi_shell)) { // y no es la mini_shell
            //eniviaremos la señal SIGTERM
           kill(jobs_list[0].pid, SIGTERM);
           
            //y lo notificamos
        #if DEBUGN4
         sprintf(mensaje,GRIS_T "[ctrlc()→ Señal %i (SIGTERM) enviada a %d (%s) por %d (%s)]\n" RESET, SIGTERM, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
        write(2, mensaje, strlen(mensaje)); //2 es el flujo stderr
        #endif

        }
        else
        { // Si es la mini_shell, no se debe abortar
#if DEBUGN4

        sprintf(mensaje,GRIS_T "[ctrlc()→ Señal %i (SIGTERM) no enviada por %d (%s) debido a que el proceso en foreground es el shell]\n" RESET, SIGTERM, getpid(), mi_shell);
        write(2, mensaje, strlen(mensaje)); 
#endif
        }
    } else {    //No hay proceso en foregroud
#if DEBUGN4
        sprintf(mensaje, GRIS_T "[ctrlc()→ Señal %i (SIGTERM) no enviada por %d (%s) debido a que no hay proceso en foreground]\n" RESET, SIGTERM, getpid(), mi_shell);
        write(2, mensaje, strlen(mensaje)); 
#endif
    }
    // Limpiamos nuestro flujo de salida
    printf("\n");
    fflush(stdout);
}




/*!*****************************************************************************
 * @brief Función que maneja la terminación de procesos hijos. 
 *Si es un comando externo lo ejecuta un proceso hijo.
 *@param signum número de la señal
 *******************************************************************************/

void reaper(int signum){
    signal(SIGCHLD, reaper);
    pid_t ended;
    int status;

   char mensaje[1200];
    while ((ended = waitpid(-1, &status, WNOHANG)) > 0){
        
        if (ended == jobs_list[0].pid) { 
            // si el proceso esta en foreground
            
            if (WIFEXITED(status)){ 
                #if DEBUGN4    //proceso finalizado solo
                sprintf(mensaje,GRIS_T "[reaper()→ Proceso hijo %d (%s) finalizado con exit code %d]\n"RESET, ended, jobs_list[0].cmd, WEXITSTATUS(status));
            write(2, mensaje, strlen(mensaje)); //2 es el flujo stderr
            #endif
                //Proceso finalizado debido a una señal
                }else if (WIFSIGNALED(status)){
                    #if DEBUGN4
            sprintf(mensaje,GRIS_T "[reaper()→ Proceso hijo %d (%s) finalizado por señal %d]\n"RESET, ended, jobs_list[0].cmd, WTERMSIG(status));
            write(2, mensaje, strlen(mensaje));
            #endif 
            }
            
            //rastreamos los datos de jobs_list[0]
            memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
            jobs_list[0].pid = 0;
            jobs_list[0].estado = 'F';
      
        }else if (ended>0){//proceso en background
           int index = jobs_list_find(ended);
            
         if (WIFEXITED(status)){
            //proceso que ha finalizado solo
            #if DEBUGN4
            sprintf(mensaje,GRIS_T "\n[reaper()→ Proceso hijo %d (%s) en background finalizado con exit code %d]\n"RESET, ended, jobs_list[index].cmd, WEXITSTATUS(status));
            write(2, mensaje, strlen(mensaje)); 
            #endif
                //si ha finalizado debido a una señal
                }else if (WIFSIGNALED(status)){
                    #if DEBUGN4
            sprintf(mensaje,GRIS_T "\n[reaper()→ Proceso hijo %d (%s) en background finalizado por señal %d]\n"RESET, ended, jobs_list[index].cmd, WTERMSIG(status));
            write(2, mensaje, strlen(mensaje)); 
            #endif
            }

            sprintf(mensaje, "\nTerminado PID %d (%s) en jobs_list[%d] con status %d\n", ended, jobs_list[index].cmd, index, status);
            write(2, mensaje, strlen(mensaje)); 
                //eliminar el proceso de la lista
            jobs_list_remove(index);
        }else {
            perror(ROJO_T"Error");
            printf(RESET);
            
        }
    }

}



/*!*****************************************************************************
 * @brief Imprime la lista de procesos
 * @param   args    lista de argumentos
 * @return 0 si no se ha producido nigun error. 
 *******************************************************************************/
int internal_jobs(char **args) {

     #if DEBUGN1
    printf(GRIS_T"[internal_jobs() → Esta función mostrará el PID de los procesos que no estén en foreground]\n"RESET);
    #endif
    int index = 1;
    while (index<=n_job){
        printf("[%d] %d \t%c \t%s \n", index, jobs_list[index].pid, jobs_list[index].estado, jobs_list[index].cmd);
        index++;

    }
    
    return EXIT_SUCCESS;
}


  /*!*****************************************************************************
 * @brief manejador proprio de la señal SIGTSTP.
 * @param signum : número de señal que recibe 
 *******************************************************************************/

void ctrlz(int signum) {
    //asociamos de nuevo la señal al manejador ctrlz
    signal(SIGTSTP,ctrlz);
  
    char msg [3000];    
  
    #if DEBUGN4
    fprintf(stderr, GRIS_T"\n[ctrlz()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n" RESET,
    getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
    #endif
    #if DEBUGN5
    fprintf(stderr,GRIS_T"[ctrlz()→ recibida señal 20 (SIGTSTP)]\n"RESET);
     #endif       
    if (jobs_list[0].pid > 0) {
        if (strcmp(jobs_list[0].cmd,mi_shell)){//no es nuestra shell
          kill(jobs_list[0].pid, SIGSTOP);
           
            #if DEBUGN4
            sprintf(msg,GRIS_T "[ctrlz()→ Señal %d enviada a %d (%s) por %d (%s)]\n" RESET, SIGSTOP, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
            write(2, msg, strlen(msg)); 
            #endif
            jobs_list[0].estado = 'D';
            //añadir el proceso a la lista con estado detenido
            jobs_list_add(jobs_list[0].pid, 'D', jobs_list[0].cmd);
            //mostrar la info del proceso
            sprintf(msg,"\n[%i] %i \t%c \t%s \n", n_job, jobs_list[n_job].pid, jobs_list[n_job].estado, jobs_list[n_job].cmd);
            write(2, msg, strlen(msg));
           //rastrear los datos de jobs_list[0]
            memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
            jobs_list[0].pid = 0;
            jobs_list[0].estado = 'F';
        } else {
            #if DEBUGN4    
            sprintf(msg,GRIS_T "[ctrlz()→ Señal %i no enviada por %d (%s) debido a que su proceso en foreground es el shell]\n"RESET,  SIGSTOP, getpid(), mi_shell);
            write(2, msg, strlen(msg)); 
            #endif
        }
    } else {
        #if DEBUGN4
         sprintf(msg,GRIS_T "[ctrlz()→ Señal %d no enviada por %d (%s) debido a que no hay proceso en foreground]\n" RESET,  SIGSTOP, getpid(), mi_shell);
            write(2, msg, strlen(msg)); 
            #endif
    }

    printf("\n");
    fflush(stdout);
}


/*!*****************************************************************************
 * @brief Función que determina si un proceso es de primer plano.
 * @param   args    lista de argumentos
 * @return TRUE(1) si el proceso es de segundo plano. 
*******************************************************************************/

int is_background(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (args[i] != NULL && strcmp(args[i], "&") == 0) {
            args[i] = NULL;
            return TRUE;
        }
    }
    return FALSE;
}



/*!*****************************************************************************
 * @brief Busca el proceso a partir de su pid y devuelve su posición.
 * @param   pid  el pid del proceso a buscar
 * @return  devuelve el indice del proceso o -1 si no lo encuentra.
*******************************************************************************/
int jobs_list_find(pid_t pid){
        for (int i =1; i<=n_job; i++){
            if (jobs_list[i].pid==pid){
               return i;
               
            }
        }

    return -1;

}

/*!*****************************************************************************
 * @brief Añade un nuevo proceso a jobs_list[].
 * @param   pid  el pid del proceso a buscar
 * @param   estado  estado del proceso
 * @param   cmd    comando del proceso   
 * @return TRUE si la agregación del proceso fue correcta. 
*******************************************************************************/
int jobs_list_add(pid_t pid, char estado, char *cmd){
    if(n_job<N_JOBS){//no hemos llegado al maximo ed Numero de Jobs permitidos
    n_job++;
    jobs_list[n_job].estado=estado;
    jobs_list[n_job].pid=pid;
    strcpy(jobs_list[n_job].cmd, cmd);
    sleep(0.5); //esperar hasta que agrega el proceso

        return TRUE;
    }else {
      fprintf(stderr, " Se ha alcanzado el número máximo de procesos permitidos!");
        return FALSE;  
    }

}

/*!*****************************************************************************
 * @brief Elimina un proceso de jobs_list[].
 * Mueve el ultimo proceso a la posicion del proceso a eliminar.
 * @param   pos indice del proceso a eliminar 
 * @return TRUE si la eliminación fue exsitosa.
*******************************************************************************/
int  jobs_list_remove(int pos){
    if (pos<N_JOBS && pos>0 ){  //porque solo se sale del shell por ctrlz
    
    if (pos == n_job){//el proceso a eliminar es el ultimo
    jobs_list[pos].pid=0;
    jobs_list[pos].estado='F';  //finalizado
    memset(jobs_list[pos].cmd,'\0',COMMAND_LINE_SIZE);
    }else{
    strcpy(jobs_list[pos].cmd,jobs_list[n_job].cmd);
    jobs_list[pos].pid=jobs_list[n_job].pid;
    jobs_list[pos].estado=jobs_list[n_job].estado;}
   
      
     n_job--;
    return TRUE;

    }else {

       
    fprintf(stderr,ROJO_T"Posición incorrecta!. No se ha eliminado ningúno proceso"RESET);
    return FALSE;
    
    }
}


 




