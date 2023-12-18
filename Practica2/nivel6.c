#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <stddef.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#define PROMPT "$ "
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define DEBUGN1 0
#define DEBUGN2 0
#define DEBUGN3 0
#define DEBUGN4 0
#define DEBUGN5 0
#define SIGCHLD 17
#define SIGINT  2
#define SIGKILL 9
#define SIGSTOP 19
#define SIGTERM 15
#define SIGTSTP 20
#define SIGCONT 18
#define SUCCES 0
#define FAILURE -1
#define N_JOBS 64
#define TRUE  1   
#define FALSE 0
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
int is_output_redirection(char **args);


struct info_job{
    pid_t pid;
    char estado; // ‘N’, ’E’, ‘D’, ‘F’ (‘N’: Ninguno, ‘E’: Ejecutándose y ‘D’: Detenido, ‘F’: Finalizado) 
    char cmd[COMMAND_LINE_SIZE];

};

int BACKGROUND  = 1;

int n_job =0;
static struct info_job jobs_list [N_JOBS]; 
static char mi_shell[COMMAND_LINE_SIZE];//variable global para guardar el nombre del minishell
void initialize_jobs(){
    jobs_list[0].pid = 0;
    jobs_list[0].estado = 'N';
    memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
}
/*!*****************************************************************************
 * @brief El main de nuesto shell
 * lea las lineas de comando y las ejecuta.
 * @return 0 
 *******************************************************************************/
int main(int argc, char *argv[]) {

char line[COMMAND_LINE_SIZE];
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


int execute_line(char *line) {
   char **args = malloc(sizeof(char *) * ARGS_SIZE);
    char tmp[COMMAND_LINE_SIZE] = "";  // Reinicializa tmp en cada iteración
    if (args ==NULL){
        fprintf(stderr,ROJO_T "No hay suficiente memoria dinamica!\n"RESET);
    }

    //parsea la línea en argumentos
    if (parse_args(args, line) > 0) {
        //aqui se modifica la linea
            #if DEBUGN2
            check_internal(args);
            #endif

            //reconstruir la linea de comando a partir de los tokens de args
    for (int i = 0; args[i] != NULL; i++) {
            strcat(tmp, args[i]);
            if (args[i + 1] != NULL)
            {
                strcat(tmp, " ");
            }
        }
       // printf(NEGRO_T"Linea de comando : %s" RESET,tmp);

        if(check_internal(args) == 0){
            bool background = is_background(args);

            //si no es un comando interno, ejecuta el comando externo usando un proceso hijo
            pid_t pid = fork();

            if(pid == 0){
                //Asociar la acción por defecto a SIGCHLD.
                signal(SIGCHLD,SIG_DFL);
                signal(SIGINT,SIG_IGN);
                signal(SIGTSTP,SIG_IGN);
                is_output_redirection(args);
                execvp(args[0], args);
                perror("execvp");
                printf(ROJO_T"%s : comando inexistente\n"RESET,args[0]);
                exit(EXIT_FAILURE);
            }else if(pid > 0){
                //proceso en background
                if (background==1){
                    BACKGROUND=1;   //reinciar el valor
                 #if DEBUGN5
                    printf(GRIS_T"Es un proceso en backgroud\n"RESET);
                    #endif       
                //printf("la linea de coadno que se copiara en cmd %s",tmp);
                jobs_list_add(pid,'E',tmp);
                printf("[%i] %i \t%i \t%s \n", n_job, jobs_list[n_job].pid, jobs_list[n_job].estado, jobs_list[BACKGROUND].cmd);
                //BACKGROUND++;
              
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
                printf(GRIS_T"[execute_line()→ PID padre: %i (%s)]\n"RESET,getpid(), mi_shell);
                printf(GRIS_T"[execute_line()→ PID hijo: %i (%s)]\n"RESET,pid,jobs_list[0].cmd);
                #endif

                //esperamos a que llegué una señal
                while (jobs_list[0].pid>0){
                pause(); }
               
               #if DEBUN3
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
        // Concatenar los argumentos para formar la ruta completa
        int total_length = 0;
        for (int i = 1; args[i]; i++) {
            total_length += strlen(args[i]);
        }

        target_dir = (char *)malloc(total_length + 1);
        target_dir[0] = '\0';  // Inicializar la cadena para evitar problemas con strcat

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
        //caso de barra invertida
        } else if (strchr(target_dir, '\\') != NULL) {
           BorrarChar(target_dir,'\\');
        } else {
            printf(GRIS_T"cd: Too much arguments\n"RESET);
            return EXIT_FAILURE;
        }
    
    }

    if (chdir(target_dir) != 0) {
        perror(ROJO_T"chdir() error");
        return EXIT_FAILURE;
    }
    
    char current_dir[ARGS_SIZE];
    if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
        printf(GRIS_T"[internal_cd()→ PWD: %s]\n"RESET, current_dir);
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
void BorrarChar(char * args, char c){
    int index = 0;
    int tmp = 0;
    while (args[index]){

        if (args[index]!=c){
            args[tmp]=args[index];
            tmp++;
        }
        index++;
    }
    args[tmp]='\0';//MARCAR FIN DE CADENA

}


 /*!*****************************************************************************
 * @brief Asigna valores a variablescd de entorno.
 * Obtiene el nombre de la variable y su valor y hace la asignación.
 * Se hace control de errores de sintaxis y de asignación.
 * @param args Lista de argumentos 
 * @return  0 si la asignación fue exsitosa. 1 en caso contario.
 *******************************************************************************/

int internal_export(char **args) {

    if (!args[1]) {
        fprintf(stderr, ROJO_T "Sintaxis incorrecta. Uso: %s NOMBRE=VALOR\n", args[0]);
        return EXIT_FAILURE;
    }
    

    int indexequal = -1;
    for (int i = 0; i < strlen(args[1]); i++) {
        if (args[1][i] == '=') {//sale cuando encuentra el primer igual
            indexequal = i;
            break;
        }
    }
     if (indexequal == -1) { //si no existe el caracter '='
        fprintf(stderr, ROJO_T "Sintaxis incorrecta. Uso: %s NOMBRE=VALOR\n", args[0]);
        return EXIT_FAILURE;
    }
   

    // Obtener el nombre de la variable
    char nombre[COMMAND_LINE_SIZE]; 
    strncpy(nombre, args[1], indexequal);
    nombre[indexequal] = '\0'; // marcar el fin de la cadena
    
    // Obtener el valor
    char valor[COMMAND_LINE_SIZE]; 
    int valor_length = strlen(args[1]) - indexequal - 1;
    strncpy(valor, args[1] + indexequal + 1, valor_length);
    valor[valor_length] = '\0'; 



    #if DEBUGN2
    printf(GRIS_T"[internal_export()→ nombre: %s]\n",nombre);
    printf(GRIS_T"[internal_export()→ valor: %s]\n",valor);
    printf(GRIS_T "[internal_export()→ antiguo valor para %s: %s]\n" RESET, nombre, getenv(nombre));
    #endif
    if (setenv(nombre, valor, 1) < 0) {
        #if DEBUGN2
        fprintf(stderr, "Error al cambiar el nombre de la variable de entorno\n");
        #endif
        return EXIT_FAILURE;
    }
    #if DEBUGN2
    printf(GRIS_T "[internal_export()→ nuevo valor para %s: %s]\n" RESET, nombre, getenv(nombre));
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
    printf(GRIS_T"[internal_source() → Esta función ejecutará un fichero de líneas de comandos]\n"RESET);
    #endif
   
    if (args[1] == NULL) {
        fprintf(stderr, ROJO_T"Error de sintaxis. Uso: %s <nombre_fichero>\n" RESET, args[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(args[1], "r");
    if (file == NULL) {
        fprintf(stderr, ROJO_T"fopen: No such file or directory\n"RESET);
        return EXIT_FAILURE;
    }
    char *line = (char *)malloc(COMMAND_LINE_SIZE);

    // Lee el archivo línea por línea
    while (fgets(line, COMMAND_LINE_SIZE, file) != NULL) {
        
        // Elimina el salto de línea ('\n' por '\0')
        size_t length = strlen(line);
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        fflush(file);
        #if DEBUGN3
                printf(GRIS_T "\n[internal_source()→ LINE: %s]\n" RESET, line);
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

int parse_args(char **args, char *line)
{
    int numTokens = 0;
    //una possible cambio es quitar los comentarios de line antes de torcearla

    args[numTokens] = strtok(line, " \t\n\r");
#if DEBUGN1
    fprintf(stderr, GRIS_T "[parse_args()→ token %i: %s]\n" RESET, numTokens, args[numTokens]);
#endif
    while (args[numTokens] && args[numTokens][0] != '#')
    {
        numTokens++;
        args[numTokens] = strtok(NULL, " \t\n\r");
#if DEBUGN1
        fprintf(stderr, GRIS_T "[parse_args()→ token %i: %s]\n" RESET, numTokens, args[numTokens]);
#endif
    }
    if (args[numTokens] )
    {
        args[numTokens] = NULL; // por si el último token es el símbolo comentario
#if DEBUGN1
        fprintf(stderr, GRIS_T "[parse_args()→ token %i corregido: %s]\n" RESET, numTokens, args[numTokens]);
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
   int internal = 0;
  
    if (strcmp(args[0], "cd") == 0){
        internal=1;
        internal_cd(args);
    }
    else if (strcmp(args[0], "export") == 0)
    {

        internal_export(args);
        internal=1;
    }
    else if (strcmp(args[0], "source") == 0)
    {
        internal_source(args);
        internal=1;
    }
    else if (strcmp(args[0], "jobs") == 0)
    {
        internal_jobs(args);
        internal=1;
    }
    else if (strcmp(args[0], "fg") == 0)
    {
        internal_fg(args);
        internal=1;
    }
    else if (strcmp(args[0], "bg") == 0)
    {
        internal_bg(args);
        internal=1;
    }
    else if (strcmp(args[0], "exit") == 0){
        #if DEBUGN2
        printf("\n Bye Bye\n");
        #endif
        internal=1;
            exit(0);
    }

    return internal;

}



int is_output_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0){ 
            // Verificar que haya un token después de '>'
            if (args[i + 1] != NULL) {
                // Abrir el archivo para la redirección de salida
                int fd = open(args[i + 1], O_RDWR | O_CREAT | O_TRUNC, 0666);
                if (fd == -1) {
                    perror(ROJO_T"Error al abrir el archivo de salida");
                    exit(EXIT_FAILURE);
                }

                // Redirigir la salida estándar al archivo
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    perror(ROJO_T"Error al redirigir la salida estándar");
                    exit(EXIT_FAILURE);
                }

                // Cerrar el descriptor de archivo abierto
                close(fd);

                // Eliminar el token '>' y el nombre del archivo de los argumentos
                args[i] = NULL;
                args[i + 1] = NULL;

                return EXIT_SUCCESS;
            } else {
                fprintf(stderr,ROJO_T "Error: '>' debe estar seguido por un nombre de archivo.\n"RESET);
                exit(EXIT_FAILURE);
            }
        }
    }

    return EXIT_FAILURE;
}


/*!*****************************************************************************
 * @brief Envia a segundo plano el trabajo indicado con su índice.
 * @param args Lista de argumentos 
 * @return 0 si el proceso fue exitoso. 1 en caso contrario.
 *******************************************************************************/

int internal_bg(char **args){

    //da error y no sé por qué
    /*if(args[1] == NULL){
        fprintf(stderr, ROJO_T "Uso: bg <número de trabajo>\n" RESET);
        return EXIT_FAILURE;
    }*/

    int pos = atoi(args[1]);
    if(pos == 0 || pos > n_job ){
        fprintf(stderr, ROJO_T "bg: no existe ese trabajo\n" RESET);
        return EXIT_FAILURE;
    }
    if(jobs_list[pos].estado == 'E'){
        fprintf(stderr, ROJO_T "bg: el trabajo ya está en segundo plano\n" RESET);
        return EXIT_FAILURE;
    }

    //cambia el estado 'E' y añade "&" al cmd
    jobs_list[pos].estado = 'E';
    strcat(jobs_list[pos].cmd, " &");
    //envia señal SIGCONT al proceso
    kill(jobs_list[pos].pid,SIGCONT);

    //visualización por pantalla información sobre el trabajo
    printf("[%d] %d \t%c \t%s \n", pos, jobs_list[pos].pid, jobs_list[pos].estado, jobs_list[pos].cmd);
    
    return EXIT_SUCCESS;
}

/*!*****************************************************************************
 * @brief Envia a primer plano el trabajo indicado con su índice.
 * @param args Lista de argumentos
 * @return 0 si el proceso fue exitoso. 1 en caso contrario.
 *******************************************************************************/

int internal_fg(char **args){
   #if DEBUGN1
   printf(GRIS_T"fg\n"RESET);
   #endif

   if (args[1] == NULL){
     fprintf(stderr, ROJO_T"\nError fg"RESET);
    return EXIT_FAILURE; 
   }

    int pos = atoi(args[1]);
    if (pos>n_job || pos == 0){
        printf(GRIS_T"No existe este trabajo\n"RESET);
        return EXIT_FAILURE;
    }else{

        if(jobs_list[pos].estado=='D'){
            //enviamos la señal SIGCONT
            kill(jobs_list[pos].pid,SIGCONT);
            #if DEBUGN5
            printf(GRIS_T"\nSeñal 18 (SIGCONT) enviada al proceso con PID %i\n"RESET,jobs_list[pos].pid);
            #endif
        }
            BorrarChar(jobs_list[pos].cmd,'&');
            //copiar los datos a job_list[0]
            jobs_list[0].estado=jobs_list[pos].estado;
            jobs_list[0].pid=jobs_list[pos].pid;
            strcpy(jobs_list[0].cmd,jobs_list[pos].cmd);
            jobs_list_remove(pos);
            printf("%s\n",jobs_list[0].cmd);   

             while (jobs_list[0].pid) 
            {
                pause();
            }

            return EXIT_SUCCESS;        
        }

    
  
    fprintf(stderr,ROJO_T"Error args"RESET);
    return EXIT_FAILURE;

   
   
}

/*!*****************************************************************************
 * @brief Manejador propio para la señal SIGINT (Ctrl+C) 
 *Si es un comando externo lo ejecuta un proceso hijo.
 *@param line : linea a ejecutar
 *@return 0
 *******************************************************************************/

void ctrlc(int signum) {
    signal(SIGINT,ctrlc);
#if DEBUGN4
    printf(GRIS_T "\n[ctrlc()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n" RESET,
           getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
#endif
#if DEBUGN5
    fprintf(stderr,"\n[ctrlc()→ recibida señal 2 (SIGINT)]\n"RESET);
    #endif
    if (jobs_list[0].pid > 0) { //Hay proceso en foreground?
        if (strcmp(jobs_list[0].cmd, mi_shell) != 0) { // y no es la mini_shell
           kill(jobs_list[0].pid, SIGTERM); //eniviaremos la señal SIGTERM
            //y lo notificamos
#if DEBUGN4
                fprintf(stderr, GRIS_T "[ctrlc()→ Señal %i enviada a %d (%s) por %d (%s)]\n" RESET, SIGTERM, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
                
                #endif
        } else{ // Sí es la mini_shell, no se debe abortar
#if DEBUGN4
            fprintf(stderr, GRIS_T "[ctrlc()→ Señal %i no enviada por %d (%s) debido a que el proceso en foreground es el shell]\n" RESET, SIGTERM, getpid(), mi_shell);
#endif
        }
    } else{
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
 *Si es un comando externo lo ejecuta un proceso hijo.
 *@param signum número de la señal
 *******************************************************************************/
void reaper(int signum){
    signal(SIGCHLD, reaper);
    pid_t ended;
    int status;
    char mensaje[1200];
    while ((ended = waitpid(-1, &status, WNOHANG)) > 0){
        if (ended == jobs_list[0].pid) { // si el proceso esta en foreground
            #if DEBUGN4
            if (WIFEXITED(status)){     //proceso finalizado solo
                sprintf(mensaje,GRIS_T "[reaper()→ Proceso hijo %d (%s) finalizado con exit code %d]\n"RESET, ended, jobs_list[0].cmd, WEXITSTATUS(status));
            write(2, mensaje, strlen(mensaje)); //2 es el flujo stderr
                //Proceso finalizado debido a una señal
                }else if (WIFSIGNALED(status)){
            printf(GRIS_T"[reaper()→ recibida señal 17 (SIGCHLD)]\n"RESET);
            sprintf(mensaje,GRIS_T "[reaper()→ Proceso hijo %d (%s) finalizado por señal %d]\n"RESET, ended, jobs_list[0].cmd, WTERMSIG(status));
            write(2, mensaje, strlen(mensaje)); 
            }
            #endif
            //rastreamos los datos de jobs_list[0]
            memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
            jobs_list[0].pid = 0;
            jobs_list[0].estado = 'F';
      
        }else if (ended>0){//proceso en background
           int index = jobs_list_find(ended);
            #if DEBUGN4
         if (WIFEXITED(status)){
            sprintf(mensaje,GRIS_T "\n[reaper()→ Proceso hijo %d (%s) en background finalizado con exit code %d]\n"RESET, ended, jobs_list[index].cmd, WEXITSTATUS(status));
            write(2, mensaje, strlen(mensaje)); 
                //si ha finalizado debido a una señal
                }else if (WIFSIGNALED(status)){
            sprintf(mensaje,GRIS_T "[reaper()→ Proceso hijo %d (%s) en background finalizadopor señal %d]\n"RESET, ended, jobs_list[index].cmd, WTERMSIG(status));
            write(2, mensaje, strlen(mensaje)); 
            }
        #endif
            sprintf(mensaje, "Terminado PID %d (%s) en jobs_list[%d] con status %d\n", ended, jobs_list[index].cmd, index, status);
            write(2, mensaje, strlen(mensaje)); 
           // fprintf("Proceso en background con PID %d ha terminado.\n", ended);  
           // printf("Terminado PID %d (%s) en jobs_list[%d] con status %d\n", ended, jobs_list[index].cmd, index, status);
            //#endif

            jobs_list_remove(index);
        }else if (ended == -1){
            perror("Error");
            
        }
         fflush(stdout);
    }
}



/*!*****************************************************************************
 * @brief Muestra el PID de procesos en background(segundo plano)
 * @param args Lista de argumentos (actualmente no utilizada).
 * @return 0
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
    pid_t foreground_pid = jobs_list[0].pid;
    #if DEBUGN4
    fprintf(stderr, GRIS_T"\n[ctrlz()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n" RESET,
    getpid(), mi_shell, foreground_pid, jobs_list[0].cmd);
    #endif
    #if DEBUGN5
    fprintf(stderr,GRIS_T"[ctrlz()→ recibida señal 20 (SIGTSTP)]\n"RESET);
    #endif
            
    if (foreground_pid > 0) {
        if (strcmp(jobs_list[0].cmd,mi_shell)!=0){//no es nuestra shell
         kill(foreground_pid, SIGSTOP);
            #if DEBUGN4
            sprintf(msg,GRIS_T "[ctrlz()→ Señal %d enviada a %d (%s) por %d (%s)]\n" RESET, SIGSTOP, foreground_pid, jobs_list[0].cmd, getpid(), mi_shell);
            write(2, msg, strlen(msg)); 
           // printf(GRIS_T "[ctrlz()→ Señal %d enviada a %d (%s) por %d (%s)]\n" RESET, SIGSTOP, foreground_pid, jobs_list[0].cmd, getpid(), mi_shell);
            #endif
            jobs_list[0].estado = 'D';
            jobs_list_add(foreground_pid, 'D', jobs_list[0].cmd);
            printf("\n[%i] %i \t%c \t%s \n", n_job, jobs_list[0].pid, jobs_list[0].estado, jobs_list[0].cmd);
            //BACKGROUND++;
           //rastrear los datos de jobs_list[0]
            memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
            jobs_list[0].pid = 0;
            jobs_list[0].estado = 'F';
        } else {
                #if DEBUGN5
            sprintf(msg,GRIS_T "[ctrlz()→ Señal %i no enviada por %d (%s) debido a que su proceso en foreground es el shell]\n"RESET,  SIGSTOP, getpid(), mi_shell);
            write(2, msg, strlen(msg)); 
            #endif
            //fprintf(stderr,GRIS_T"[ctrlz()→ Señal %i no enviada por %d (%s) debido a que el proceso en foreground es el shell]\n" RESET, SIGSTOP, getpid(), mi_shell);                
        }
    } else {
         #if DEBUGN5
         sprintf(msg,GRIS_T "[ctrlz()→ Señal %d no enviada por %d (%s) debido a que no hay proceso en foreground]\n" RESET,  SIGSTOP, getpid(), mi_shell);
            write(2, msg, strlen(msg)); 
       // fprintf(stderr, GRIS_T "[ctrlz()→ Señal %d no enviada por %d (%s) debido a que no hay proceso en foreground]\n" RESET, SIGSTOP, getpid(), mi_shell);
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
            args[i] = NULL;//ponemos a null el valor anterior que es espacio
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




 


 


