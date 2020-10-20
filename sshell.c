#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#define CMDLINE_MAX 512
<<<<<<< HEAD
#define PATH_MAX 4096 
#define TOKEN_MAX 32

enum {exit_cmd = 0, pwd_cmd = 1, cd_cmd = 2, sls_cmd = 3};
//enum {READ = 0, WRITE = 1};

int execute_pwd() {
    char cwd[PATH_MAX];
    int ret_val = 0;
    if(getcwd(cwd, sizeof(cwd)) != NULL) {
        fprintf(stdout, "%s\n", cwd);
        }
    else {
        ret_val = 1;
        perror("Error: pwd failed");
    }
    return ret_val;   
}

int execute_cd(char * path) {
    int ret_val;
    ret_val = chdir(path);
    if(ret_val != 0) {
        perror("Error: cannot cd into directory");
        ret_val = 1;
    }
    return ret_val;
}

=======
#define PATH_MAX 4096 // Linux defines PATH_MAX 4096 bytes
#define TOKEN_MAX 32

enum { exit_cmd = 0, pwd_cmd = 1, cd_cmd = 2, sls_cmd = 3};

int execute_cd(char * path) {
    int ret_val;
    ret_val = chdir(path);
    if(ret_val != 0) {
        perror("Error: cannot cd into directory");
        ret_val = 1;
    }
    return ret_val;
}
>>>>>>> 0653b79fd8b27da5799def0e38729e79f5e51a27
int execute_sls() {
    int ret_val = 0;
    DIR *dirp;
    struct dirent *dp;
    dirp = opendir(".");
    if(dirp == NULL) {
        ret_val = 1;
<<<<<<< HEAD
    }
    while((dp = readdir(dirp)) != NULL) {
        struct stat sb;
        stat(dp->d_name, &sb);
        fprintf(stdout, "%s (%lld bytes)\n", dp->d_name, sb.st_size);
    }
    return ret_val;
}

int execute_builtin_commands(char** args, int cmd_num){
    int ret_val;
    switch(cmd_num) {
        case 1:
            ret_val = execute_pwd();
            break;
        case 2:
            ret_val = execute_cd(args[1]);
            break;
        case 3:
            ret_val = execute_sls();
            break;
        default:
            ret_val = 0;
            break;
    }
    return ret_val;
}

char *read_cmd(void){
        char *cmd = malloc(sizeof(char) * CMDLINE_MAX);
        if(!cmd){
                perror("ERROR: Malloc");
                exit(EXIT_FAILURE);
        }

        int offset;
        do{
                offset = read(STDIN_FILENO, cmd, CMDLINE_MAX);
                if (!isatty(STDIN_FILENO)) { 
                        printf("%s", cmd);
                        fflush(stdout);
                }
                if(offset == -1) 
                        exit(EXIT_FAILURE);
        
                cmd[offset] = '\0';
=======
    }
    while((dp = readdir(dirp)) != NULL) {
        struct stat sb;
        stat(dp->d_name, &sb);
        fprintf(stdout, "%s (%lld bytes)\n", dp->d_name, sb.st_size);
    }
    return ret_val;
}
int execute_pwd() {
    char cwd[PATH_MAX];
    int ret_val = 0;
    if(getcwd(cwd, sizeof(cwd)) != NULL) {
        fprintf(stdout, "%s\n", cwd);
        }
    else {
        ret_val = 1;
        perror("Error: pwd failed");
    }
    return ret_val;   
}

int execute_builtin_commands(char** args, int cmd_num){
    int ret_val;
    switch(cmd_num) {
        case 1:
            ret_val = execute_pwd();
            break;
        case 2:
            ret_val = execute_cd(args[1]);
            break;
        case 3:
           ret_val = execute_sls();
            break;
        default:
            ret_val = 0;
            break;
    }
    return ret_val;
}
char *read_cmd(void){
        // memory allocation
        char *cmd = malloc(sizeof(char) * CMDLINE_MAX);
        //int cmd_len = strlen(cmd);
        
        
        /* Print command line if stdin is not provided by terminal */
        if (!isatty(STDIN_FILENO)) { // isatty(fd) test whether a fd is open fd refers to a terminal
                printf("%s", cmd);
                fflush(stdout);
        }

        if(!cmd){
                //fprintf(stderr, "ERROR: malloc");
                perror("ERROR: malloc");
                exit(EXIT_FAILURE);
        }

        // reading the cmd line 
        // fgets(cmd, CMDLINE_MAX, stdin);
        int offset;
        do{
                offset = read(STDIN_FILENO, cmd, CMDLINE_MAX);
                //printf("%s%d\n", cmd, offset);
                if(offset == -1) {// at the end of the txt
                        //fprintf(stderr, "ERROR: read cmd_line");
                        perror("ERROR: read cmd_line");
                        exit(EXIT_FAILURE);
                }
                // reset cursor pointer to NULL
                cmd[offset] = '\0';
                
>>>>>>> 0653b79fd8b27da5799def0e38729e79f5e51a27
                return cmd;
        }while(1);
}

<<<<<<< HEAD
struct commands{
        char **arguments;
        //char *argument;
        // i.e. char *argument1 = {date '\0'} char 
        // *argument2 = {tr 2 1 '\0'}
        // '\0' orginally |
        // char **arguments = {argument1, argument2} = {{{date '\0'}, {tr 2 1 '\0'} }
        // any difference between char **arguments = {date '\0' tr 2 1 '\0' } ?
};
//void divide_argument(struct commands *obj){
//      char **i_arugments = obj->arguments;
//      char *argument1
//      char *argument2 
//}
=======
// parsing the cmd_line using string tokenization 
// ---------------------------------------------- USE struct object 

/*
#define delimiter = " \t\r\n";
*/
struct parsed_token{
        char **tokens;
        char *tok;
        //int length;
        //int pointer;
};
// ---------------------

void assign_token(struct parsed_token *p_tokens, char *p_tok){
        p_tokens->tok=p_tok;
}

char **parse_cmd(char *cmd, int* size){ //struct parsed_token *p_tokens, 
        // memory allocation
        char *s_token;
        char **token = malloc(sizeof(char*) * TOKEN_MAX);
        int size_tokens = 0;
        if(!token){
                //fprintf(stderr, "ERROR: malloc");
                perror("ERROR: malloc");
                exit(EXIT_FAILURE);
        }

        // ref: https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
        int pointer = 0;
        //printf("debug1, %s\n",cmd);
        s_token = strtok(cmd, " \t\r\n"); // common delimiter - escape character
        //struct parsed_token s_tokens = *p_tokens;
        while(s_token != NULL){
                // ---------------------
                
                //printf("debug, %s\n", s_token);
                //assign_token(&s_tokens,s_token);
                //printf("debug2, %s\n",p_tokens->tok);
                
                // ---------------------
               

                token[pointer] = s_token;
                pointer++;
                s_token = strtok(NULL, " \t\r\n");
                size_tokens++;
        }
        token[pointer] = NULL;
        *size = size_tokens;
        return token;
}

int execute_cmd(char **args){
    int built_cmd = -1;
        if(args[0] == NULL)
                return EXIT_FAILURE;
        // ---------------------------- Check whether it is Builtin command or not
        /* Builtin command */
        // execute built in - return (return status)
        if (!strcmp(args[0], "exit")) {  // string compare, equal == 0
                fprintf(stderr, "Bye...\n");
                free(args[0]);
                fprintf(stderr, "+ completed 'exit' [%d]\n",
                        0);
                return EXIT_FAILURE;
        }
        if(!strcmp(args[0], "pwd")) {
            built_cmd = pwd_cmd;
        }
        if(!strcmp(args[0], "cd")) {
            built_cmd = cd_cmd;   
        }
        if(!strcmp(args[0], "sls")) {
            built_cmd = sls_cmd; 
        }
        if(built_cmd != -1) {
            return execute_builtin_commands(args,built_cmd);
            built_cmd = -1;
        }

       



        pid_t pid;
        pid = fork();
        if(pid==0){ // child process
                if(execvp(args[0], args)==-1)
                        return EXIT_FAILURE;
                return EXIT_SUCCESS;
        } else if(pid > 0){ // parent
                int status;
                waitpid(pid, &status, 0);
                if(WEXITSTATUS(status) == -1)
                        return EXIT_FAILURE;
                return EXIT_SUCCESS;
        } else{ // forking error
                perror("fork");
                exit(1);
        }

        return EXIT_SUCCESS;
}

// pipe three arguments maximum
// ref: https://www.geeksforgeeks.org/making-linux-shell-c/
// int execute_pipe(char* )
// void execute_pipe(char **args1, char**args2){
//
//} (A | B ) = (A' | C)

int output_redirection(char** args, int cmd_pos) {
        char* path = malloc(sizeof(char) * PATH_MAX);
        char** new_args = malloc(sizeof(char*) * TOKEN_MAX);
        char program_name[strlen(args[0])-2];
        int fd = 0, ret = 0;

        /*Open file with appropriate macro*/
        if(strlen(args[cmd_pos]) > 1)
                fd = open(args[cmd_pos+1], O_RDWR|O_CREAT|O_APPEND, 0600);
        else
                fd = open(args[cmd_pos+1], O_RDWR| O_CREAT | O_TRUNC, 0600);

        if (fd == -1) {
                perror("Error: Cannot open file");
                return 1;
        }

        /* Redirect output & execute command */
        int std_out = dup(STDOUT_FILENO);
        if(dup2(fd, STDOUT_FILENO) == -1) {
                perror("Error: Cannot redirect output");
                ret = 1;
                return ret;
        }
        if(strstr(args[0],"./") != NULL) { //Check if executable isnt a regular command and find the path to it
                char* cmd = args[0];
                unsigned long j = 0;
                for(unsigned long s = 2; s  < strlen(args[0]); s++) {
                        program_name[j] = cmd[s];
                        j++;
                }
                program_name[j] = '\0';
                realpath(program_name, path);
                new_args[0] = path;
                ret = execute_cmd(new_args);
        }
        else {
                int j = 0;
                for(int l = 0; l < cmd_pos; l++) {
                        new_args[j] = args[l];
                                j++;
                }
                ret = execute_cmd(new_args);
        }
        
        /* return everything to normal and free memory*/
        fflush(stdout); 
        dup2(std_out, STDOUT_FILENO);
        close(fd);
        close(std_out);
        free(new_args);
        free(path);

        return ret;
}
int main(void) 
{
        //char cmd[CMDLINE_MAX]; 
        //struct parsed_token *p_tokens = NULL;
        char *cmd; 
        char **token;
        while (1) {
                //char *nl;
                int retval;
                int output_red = 0;
                int size;

>>>>>>> 0653b79fd8b27da5799def0e38729e79f5e51a27

char **parse_cmd(struct commands *obj, char *cmd, int* size){
        char *s_token;
        char **token = malloc(sizeof(char*) * TOKEN_MAX);
        if(!token){
                perror("ERROR: Malloc");
                exit(EXIT_FAILURE);
        }

<<<<<<< HEAD
        int pointer = 0;
        int size_tokens = 0;
        s_token = strtok(cmd, " \t\r\n"); 
        while(s_token != NULL){
                token[pointer] = s_token;
                pointer++;
                s_token = strtok(NULL, " \t\r\n");
                size_tokens++;
        }
        token[pointer] = NULL;

        obj->arguments = token;
        *size = size_tokens;

        return (obj->arguments);
}

int execute_cmd(char **args){
    int built_cmd = -1;
        if(args[0] == NULL)
                return EXIT_FAILURE;

        if (!strcmp(args[0], "exit")) {  
                fprintf(stderr, "Bye...\n");
                free(args[0]);
                fprintf(stderr, "+ completed 'exit' [%d]\n",
                        0);
                return -1;
        }
        if(!strcmp(args[0], "pwd")) {
            built_cmd = pwd_cmd;
        }
        if(!strcmp(args[0], "cd")) {
            built_cmd = cd_cmd;   
        }
        if(!strcmp(args[0], "sls")) {
            built_cmd = sls_cmd; 
        }
        if(built_cmd != -1) {
            return execute_builtin_commands(args,built_cmd);
            built_cmd = -1; //?
        }

        pid_t pid;
        pid = fork();
        if(pid==0){ 
                if(execvp(args[0], args) == -1)
                        return EXIT_FAILURE;
                return EXIT_SUCCESS;
        } else if(pid > 0){ 
                int status;
                waitpid(pid, &status, 0);
                if(WEXITSTATUS(status) == -1)
                        return EXIT_FAILURE;
                return EXIT_SUCCESS;
        } else{ 
                perror("fork");
                exit(1);
        }

        return EXIT_SUCCESS;
}
/*
int helper_pipe(int i_fd, int o_fd, struct commands *obj){
        pid_t pid = 0;
        if((pid == fork()) == 0){
                if(i_fd != 0){
                        dup2(i_fd, 0);
                        close(i_fd);
                }

                if(o_fd != 1){
                        dup2(o_fd, 1);
                        close(o_fd);
                }
                return execvp(obj->arguments[0], (char * const *) obj->arguments);
        }
        return pid;
}

int execute_pipe(struct commands *obj, int pipe_count){
        int fd[2];
        int f_fd = 0;
        int i = 0;
        for (i = 0; i < pipe_count; i++){
                pipe(fd);
                helper_pipe(f_fd, fd[1], obj + i);
                close(fd[1]);
                f_fd = fd[0];
        }
        if(f_fd != 0)
                dup2(f_fd, 0);
        return execvp(obj->arguments[i], (char * const *)obj->arguments[i]);
}
*/

int execute_pipe(struct commands *obj){
        int fd[2];
        pid_t pid;
        int fd_in = 0;
        int retval = 0;
        while (*obj->arguments != NULL) // to take multiple arguments
        {
                pipe(fd);
                if ((pid = fork()) == -1){
                        exit(EXIT_FAILURE);
                } else if (pid == 0){
                        dup2(fd_in, 0); //change the input according to the old one 
                        if (*(obj->arguments + 1) != NULL)
                                dup2(fd[1], 1); // change the output to the std output
                        close(fd[0]);
                        retval  = execvp((obj->arguments)[0], (char * const *) obj->arguments); // might need to recalle execute_cmd
                        exit(EXIT_FAILURE);
                } else{
                        wait(NULL); // until child has terminated
                        close(fd[1]);
                        fd_in = fd[0]; //input data for the next command
                        obj->arguments++;
        }
    }
        return retval;
}


int output_redirection(char** args, int cmd_pos) {
        char* path = malloc(sizeof(char) * PATH_MAX);
        char** new_args = malloc(sizeof(char*) * TOKEN_MAX);
        char program_name[strlen(args[0])-2];
        int fd = 0, ret = 0;

        /*Open file with appropriate macro*/
        if(strlen(args[cmd_pos]) > 1)
                fd = open(args[cmd_pos+1], O_RDWR|O_CREAT|O_APPEND, 0600);
        else
                fd = open(args[cmd_pos+1], O_RDWR| O_CREAT | O_TRUNC, 0600);

        if (fd == -1) {
                perror("Error: Cannot open file");
                return 1;
        }

        /* Redirect output & execute command */
        int std_out = dup(STDOUT_FILENO);
        if(dup2(fd, STDOUT_FILENO) == -1) {
                perror("Error: Cannot redirect output");
                ret = 1;
                return ret;
        }
        if(strstr(args[0],"./") != NULL) { //Check if executable isnt a regular command and find the path to it
                char* cmd = args[0];
                unsigned long j = 0;
                for(unsigned long s = 2; s  < strlen(args[0]); s++) {
                        program_name[j] = cmd[s];
                        j++;
                }
                program_name[j] = '\0';
                realpath(program_name, path);
                new_args[0] = path;
                ret = execute_cmd(new_args);
        }
        else {
                int j = 0;
                for(int l = 0; l < cmd_pos; l++) {
                        new_args[j] = args[l];
                                j++;
                }
                ret = execute_cmd(new_args);
        }
        
        /* return everything to normal and free memory*/
        fflush(stdout); 
        dup2(std_out, STDOUT_FILENO);
        close(fd);
        close(std_out);
        free(new_args);
        free(path);

        return ret;
}
int main(void) 
{
        char *cmd; 
        char **token;
        struct commands command;
        
        while (1) {
                int retval;
                int size;
                int output_red = 0;
                int pipe_count = 0;

                printf("sshell$ ");
                fflush(stdout);

                cmd = read_cmd();
                token = parse_cmd(&command, cmd, &size);
                //printf("debug\n");
                int cmd_pos;
                char *pipe_dilimiter = "|";
                for(cmd_pos = 0; cmd_pos < size; cmd_pos++){
                        if(!strcmp(token[cmd_pos],pipe_dilimiter)){
                                pipe_count++;
                                *command.arguments[cmd_pos] = '\0'; 
                        }
                } 
                //printf("debug\n");
                //int i;
                //for(i=0;i<size;i++)
                //        printf("%s\n", command.arguments[i]);
                
                if(pipe_count)
                        retval = execute_pipe(&command);
                else
                        retval = execute_cmd(token);
                
                // Will have to think about the input with both output direction and pipe - maybe, put this in execute_cmd()
                // jporquet@pc10:~/ $ echo -e "echo Hello\nexit\n" | ./sshell >& your_output
                cmd_pos = 0;
                for(cmd_pos = 0; cmd_pos < size; cmd_pos++) {
=======
                /* Get command line */
                //fgets(cmd, CMDLINE_MAX, stdin); // reads a line from stdin stream stores into cmd as string
                cmd = read_cmd();

                /* Print command line if stdin is not provided by terminal */
                //if (!isatty(STDIN_FILENO)) { // isatty(fd) test whether a fd is open fd refers to a terminal
                //        printf("%s", cmd);
                //        fflush(stdout);
                //}

                /* Remove trailing newline from command line */
                //nl = strchr(cmd, '\n'); // returns pointer to first occurrnece of char,\n
                //if (nl)
                //        *nl = '\0'; // termination of a char string

                        
                /* Regular command */
                //retval = system(cmd);
                


                token = parse_cmd(cmd,&size); //p_tokens
                int cmd_pos;
                for( cmd_pos = 0; cmd_pos < size; cmd_pos++) {
>>>>>>> 0653b79fd8b27da5799def0e38729e79f5e51a27
                        if(strstr(token[cmd_pos],">") != NULL) {
                                output_red = 1;
                                break;
                        }
                }
<<<<<<< HEAD
                if(output_red == 1)
                        retval = output_redirection(token,cmd_pos);
//              else
//                      retval = execute_cmd(token);
                
                if(retval == 0)
                        fprintf(stderr, "+ completed '%s' [%d]\n", // will need to print out entire cmd
                        &cmd[0], retval);
                else if(retval == -1)
                        break;
                else
                        perror("Error: ");
=======
                
                if(output_red == 1)
                        retval = output_redirection(token,cmd_pos);
                else
                        retval = execute_cmd(token); // system(cmd);

                if(retval == 0)
                        fprintf(stderr, "+ completed '%s' [%d]\n",
                        cmd, retval);
                else
                        break; 
                // ------------------------------------
                // Error Handling - display next sshell
>>>>>>> 0653b79fd8b27da5799def0e38729e79f5e51a27
        }

        return EXIT_SUCCESS;
}
