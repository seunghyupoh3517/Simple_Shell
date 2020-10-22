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
#define PATH_MAX 4096 
#define TOKEN_MAX 32
#define PIPE_ARG_MAX 4
#define MAX_ARGS 16

// 1. Fix the bug in multiple pipes -> fprintf(stderr, concataenate the argumetns and exit values) - 10/20
// 3. Running tester.sh & Test it on sshell.ref & Based on instruction - 10/21
// 4. Clean the code according coding style - consistency , keep small sniphet of comments , check the coding style - 10/21
// 5. Finalize it - 10/22

enum {exit_cmd = 0, pwd_cmd = 1, cd_cmd = 2, sls_cmd = 3};
enum {read_fd = 0, write_fd = 1};

int execute_pwd() {
    char cwd[PATH_MAX];
    int ret_val = 0;
    if(getcwd(cwd, sizeof(cwd)) != NULL)
        fprintf(stdout, "%s\n", cwd);
    else {
        fprintf(stderr,"Error: pwd failed\n");
        return 1;
    }

    return ret_val;   
}

int execute_cd(char * path) {
    int ret_val;
    ret_val = chdir(path);
    if(ret_val != 0) {
        fprintf(stderr, "Error: cannot cd into directory\n");
        return 1;
    }

    return ret_val;
}

int execute_sls() {
    int ret_val = 0;
    int depth = 0;
    DIR *dirp;
    struct dirent *dp;
    dirp = opendir(".");

    if(dirp == NULL) {
        fprintf(stderr, "Error: cannot open directory\n");
        return 1;
    }
    
    while((dp = readdir(dirp)) != NULL) {
        struct stat sb;
        if(dp->d_name[0] != '.') {
                stat(dp->d_name, &sb);
                fprintf(stdout, "%s (%lld bytes)\n", dp->d_name, sb.st_size);
                depth++;
        }
    }
    if(depth == 0) 
        fprintf(stdout,"empty (0 bytes)\n");

        closedir(dirp);
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

struct commands{
        char **arguments;
        char ***args;
        char *arg1[TOKEN_MAX];
        char *arg2[TOKEN_MAX];
        char *arg3[TOKEN_MAX];
        char *arg4[TOKEN_MAX];
};

char **parse_cmd(struct commands *obj, char *cmd, int* size){
        char *s_token;
        char **token = malloc(sizeof(char*) * TOKEN_MAX);
        
        if(!token){
                perror("ERROR: Malloc");
                exit(EXIT_FAILURE);
        }

        int pointer = 0;
        int size_tokens = 0;
        s_token = strtok(cmd, " \t\r\n"); 
        while(s_token != NULL){
                token[pointer] = s_token;
                
                pointer++;
                s_token = strtok(NULL, " \t\r\n");
                size_tokens++;
        }

        token[pointer] = "\0"; //  "\0": string literal holding '\0' plus second one as a terminator
        obj->arguments = token;
        *size = size_tokens;
 
        return token;
}

int execute_cmd(char **args){
    int built_cmd = -1;
        /* Error handling*/
        if(args[0] == NULL)
                return EXIT_FAILURE;

        /* Built in commands */
        if (!strcmp(args[0], "exit")) {  
                fprintf(stderr, "Bye...\n");
                if(args[2])
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
        }

        /* Regular commands */
        pid_t pid;
        pid = fork();
        if(pid==0){ 
                if(execvp(args[0], args) == -1) {
                        fprintf(stderr, "Error: command not found\n");
                        exit(EXIT_FAILURE);
                }          
                return EXIT_SUCCESS;
        } 
        else if(pid > 0){ 
                int status;
                waitpid(pid, &status, 0);
                if(WEXITSTATUS(status) == -1 || WEXITSTATUS(status) == 1) {
                        
                        return EXIT_FAILURE;
                }
                        
                return EXIT_SUCCESS;
        } 
        else{ 
                perror("fork");
                exit(EXIT_FAILURE);
        }


        return EXIT_SUCCESS;
}

void helper_pipe(char ***pipe, char *piping_cmd){
        char *s_token;
        char **token = malloc(sizeof(char*) * TOKEN_MAX);
        char *arg1[TOKEN_MAX] = {NULL,};
        char *arg2[TOKEN_MAX] = {NULL,};
        char *arg3[TOKEN_MAX] = {NULL,};
        char *arg4[TOKEN_MAX] = {NULL,};
        if(!token){
                perror("ERROR: Malloc");
                exit(EXIT_FAILURE);
        }
        
        int pointer = 0;
        s_token = strtok(piping_cmd, " \t\r\n");
        while(s_token != NULL){
                token[pointer] = s_token;
                pointer++;
                s_token = strtok(NULL, " \t\r\n");
        }
        token[pointer++] = "\0"; //  "\0": string literal holding '\0' plus second one as a terminator


        int nextIndex = 0;
        for(int i=0;sizeof(token) / sizeof(int);i++){
                if(token[i] != NULL){
                                if(strcmp(token[i], "|") != 0){
                                        arg1[i] = token[i];
                                }else{
                                        arg1[i] = "\0";
                                        nextIndex = i+1;
                                        break;
                                }
                }
        }

        int arg2Index = 0;
        for(int i=nextIndex;sizeof(token) / sizeof(int);i++){
                if(token[i] != NULL && (strcmp(token[i], "\0") != 0) ){
                        if(strcmp(token[i], "|") != 0){
                                        arg2[arg2Index] = token[i];
                                        arg2Index++;
                                }else{
                                        arg2[arg2Index] = "\0";
                                        nextIndex = i+1;
                                        break;
                                }
                }else{
                        nextIndex = i+1;
                        break;
                }
        }
        int arg3Index = 0;
        for(int i=nextIndex;sizeof(token) / sizeof(int);i++){
                if(token[i] != NULL && (strcmp(token[i], "\0") != 0) ){
                                if(strcmp(token[i], "|") != 0){
                                        arg3[arg3Index] = token[i];
                                        arg3Index++;
                                }else{
                                        arg3[arg3Index] = "\0";
                                        nextIndex = i+1;
                                        break;
                                }
                }else{
                        nextIndex = i+1;
                        break;
                }
        }

        int arg4Index = 0;
        for(int i=nextIndex;sizeof(token) / sizeof(int);i++){
                if(token[i] != NULL && (strcmp(token[i], "\0") != 0) ){
                                if(strcmp(token[i], "|") != 0){
                                        arg4[arg4Index] = token[i];
                                        arg4Index++;
                                }else{
                                        arg4[arg4Index] = "\0";
                                        nextIndex = i;
                                        break;
                                }
                }else{
                        nextIndex = i+1;
                        break;
                }
        }

        printf("%s\n", *arg1);
        printf("%s\n", *arg2);
        printf("%s\n", *arg3);
        printf("%s\n", *arg4);

        char **temp_pipe[] = {arg1, arg2, arg3, arg4, NULL};
        pipe = temp_pipe;
 }
        /*
        char *arg1[TOKEN_MAX] = {NULL,};
        char *arg2[TOKEN_MAX] = {NULL,};
        char *arg3[TOKEN_MAX] = {NULL,}; 
        char *arg4[TOKEN_MAX] = {NULL,};

        char *s_token;
        char *p_token;
        char **token = malloc(sizeof(char*) * TOKEN_MAX);
        
        if(!token){
                perror("ERROR: Malloc");
                exit(EXIT_FAILURE);
        }

        //int pointer = 0;

        p_token = strstr(piping_cmd, "|");
        s_token = strtok(piping_cmd, " \t\r\n");         
        while((s_token != NULL) && !(p_token)){
                *pip->arg1 = s_token;
                s_token = strtok(NULL, " \t\r\n");
        }       
        pip->args[0] = &pip->arg1[0];

        printf("DEBUG\n");
        p_token = strstr(piping_cmd, "|");
        s_token = strtok(piping_cmd, " \t\r\n"); 
        while((s_token != NULL) && !(p_token)){
                *pip->arg2 = s_token;
                s_token = strtok(NULL, " \t\r\n");
        }
        pip->args[1] = &pip->arg2[0];

        p_token = strstr(piping_cmd, "|");
        s_token = strtok(piping_cmd, " \t\r\n"); 
        while((s_token != NULL) && !(p_token)){
                *pip->arg3 = s_token;
                s_token = strtok(NULL, " \t\r\n");
        }
        pip->args[2] = &pip->arg3[0];

        p_token = strstr(piping_cmd, "|");
        s_token = strtok(piping_cmd, " \t\r\n"); 
        while((s_token != NULL) && !(p_token)){
                *pip->arg4 = s_token;
                s_token = strtok(NULL, " \t\r\n");
        }
        pip->args[3] = &pip->arg4[0];
        pip->args[4] = NULL;

        pipe = pip->args;
        //token[pointer++] = "\0"; //  "\0": string literal holding '\0' plus second one as a terminator
        //pip->arguments = token;

*/
/*
        pointer = 0;
        char *p_token;
        char **temp_token = malloc(sizeof(char*) * TOKEN_MAX);
        int check = 0;
        p_token = strtok(*pip->arguments, "|");
        while(p_token != NULL){
                temp_token[pointer] = p_token;
                if(check == 0){
                        *pip->arg1 = *temp_token;
                        check++;
                } else if(check == 1){
                        *pip->arg2 = *temp_token;
                        check++;
                } else if(check == 2){
                        *pip->arg3 = *temp_token;
                        check++;
                } else if(check == 3){
                        *pip->arg4 = *temp_token;
                        check++;
                }
                pointer++;
                p_token = strtok(NULL, "|");
        }
        temp_token[pointer++] = "\0";
        
        char **temp_pipe[] = {pip->arg1, pip->arg2, pip->arg3, pip->arg4, NULL};
        pipe = temp_pipe;

        free(token);
        free(temp_token);
        //free(temp_pipe);
*/
 

int *execute_pipe(char ***obj, int *size, int pipe_count){
        /* Collect child exit status */
        static int exit_status[PIPE_ARG_MAX];
        int exit_pos = 0;
        pid_t childpid;
        int fd[2];
        int fdd = 0;
        
        /*
        while (*obj->args != NULL) 
        {       
                switch(pipe_count){
                        case 1:



                                break;
                        case 2:



                                break;
                        case 3: 



                                break;

                }
        */
        int i;
        for(i = 0; i< pipe_count; i++){
                pipe(fd);
                /* fork() error occurred */
                if ((childpid = fork()) == -1)
                {
                        fprintf(stderr, "Process Fork Failed");
                        exit_status[exit_pos] = EXIT_FAILURE;
                        exit_pos++;
                }
                else if (childpid == 0)
                { 
                        dup2(fdd, read_fd); 
                        if (*(obj + 1) != NULL)
                                dup2(fd[1], write_fd); 
                        close(fd[0]);
                        exit_status[exit_pos]  = execvp((*obj)[0],  *obj); 
                        exit_pos++;
                        exit(EXIT_FAILURE);
                } 
                else
                {       
                        //close(fd[0]);
                        //int status;
                        //waitpid(childpid, &status, 0);
                        wait(NULL);
                        close(fd[1]);
                        fdd = fd[0]; 
                        obj++;
                }
        }

        close(fd[0]);
        close(fd[1]);
        *size = exit_pos;

        return exit_status;
}

int output_redirection(char** args, int cmd_pos, int size) {
        char* path = malloc(sizeof(char) * PATH_MAX);
        char** new_args = malloc(sizeof(char*) * size);
        char program_name[strlen(args[0])-2];
        int fd = 0, ret = 0;
        if(!path || !new_args){
                perror("ERROR: Malloc");
                exit(EXIT_FAILURE);
        }
        if(args[cmd_pos+2] == NULL) {
                fprintf(stderr,"Error: no output file\n");
                return 2;
        }
        if(cmd_pos == 0) {
                fprintf(stderr,"Error: missing command\n");
                return 2;
        }
        /*Open file with appropriate macro*/
        if(strlen(args[cmd_pos]) > 1)
                fd = open(args[cmd_pos+1], O_RDWR|O_CREAT|O_APPEND, 0600);
        else
                fd = open(args[cmd_pos+1], O_RDWR| O_CREAT | O_TRUNC, 0600);

        if (fd == -1) {
                fprintf(stderr, "Error: Cannot open output file\n");
                return 2;
        }

        /* Redirect output & execute command */
        int std_out = dup(STDOUT_FILENO);
        if(dup2(fd, STDOUT_FILENO) == -1) {
                fprintf(stderr,"Error: Cannot redirect output\n");
                return 1;
        }
        if(strstr(args[0],"./") != NULL) { //if not regular command, find path
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
        /* Variable Initiation */
        char cmd[CMDLINE_MAX]; 
        char **token;
        char *piping_cmd;
        char *print_cmd;
        struct commands command;
        
        while (1) {
                int retval;
                int size;
                int size_exit;
                int output_red = 0;
                int pipe_count = 0;
                char* nl;

                /* Print Prompt */
                printf("sshell$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);
                
                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }
                piping_cmd = strdup(cmd);
                strtok(piping_cmd, "\n");
                print_cmd = strdup(cmd);
                strtok(print_cmd, "\n");

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                /* Parse the commands */
                token = parse_cmd(&command, cmd, &size);
                if(size > MAX_ARGS) {
                        fprintf(stderr, "Error: too many process arguments\n");
                        continue;
                }
                /* Check whether Pipe or Output Redirection or Regular Command  */
                int cmd_pos, pipe_pos = -1;;
                char *pipe_dilimiter = "|";
                for(cmd_pos = 0; cmd_pos < size; cmd_pos++){
                        if(!strcmp(token[cmd_pos], pipe_dilimiter)){
                                pipe_count++;
                                *command.arguments[cmd_pos] = '\0';
                                pipe_pos = cmd_pos;
                        }
                        if((pipe_count) && (cmd_pos = size - 1))
                                *command.arguments[cmd_pos] = '\0';
                } 
                cmd_pos = 0;
                for(cmd_pos = 0; cmd_pos < size; cmd_pos++) {
                        if(strstr(token[cmd_pos], ">") != NULL) {
                                output_red = 1;
                                break;
                        }
                }
                if(pipe_count !=0 && output_red == 1 && cmd_pos < pipe_pos) {
                        fprintf(stderr,"Error: mislocated ouput redirection\n");
                        continue;
                }
                if((pipe_pos == 0) || (pipe_pos+1) >= size){
                        fprintf(stderr,"Error: missing command\n");
                        continue;
                }

                /* Execute commands corresponding to the types */
                int retpipe[PIPE_ARG_MAX]; 
                //struct commands pip;
                if(pipe_count){
                        char **pipe_cmds[TOKEN_MAX];
                        
                        helper_pipe(pipe_cmds, piping_cmd);
                        
                        *retpipe = *execute_pipe(pipe_cmds, &size_exit, pipe_count);
                }
                if(output_red)
                        retval = output_redirection(token,cmd_pos,size);
                else
                        retval = execute_cmd(token);
                
                /* Decide action correspond to returned exit cod */
                if((retval == 0 || retval == 1) && (!pipe_count) )
                        fprintf(stderr, "+ completed '%s' [%d]\n", 
                        print_cmd, retval);
                else if(pipe_count){
                        fprintf(stderr, "+ completed '%s' ",   
                        print_cmd);
                        int i;
                        
                        for(i = 0; i < size_exit; i++)
                                fprintf(stderr, "[%d]", 
                                retpipe[i]);
                        fprintf(stderr, "\n");
                }
                else if(retval == -1)
                        break;

                free(print_cmd);
                free(token);
        }
        free(print_cmd);
        free(token);


        return EXIT_SUCCESS;
}