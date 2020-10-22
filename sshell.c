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
                free(args);
                        
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
                if(execvp(args[0], args) == -1)
                        return EXIT_FAILURE;
                return EXIT_SUCCESS;
        } 
        else if(pid > 0){ 
                int status;
                waitpid(pid, &status, 0);
                if(WEXITSTATUS(status) == -1)
                        return EXIT_FAILURE;
                return EXIT_SUCCESS;
        } 
        else{ 
                perror("fork");
                exit(EXIT_FAILURE);
        }

        return EXIT_SUCCESS;
}

void helper_pipe(char ***pipe, struct commands *obj, int size){
        // char** arg1 = malloc(sizeof(char*) * TOKEN_MAX);
        char *arg1[TOKEN_MAX];
        char *arg2[TOKEN_MAX];
        char *arg3[TOKEN_MAX];
        char *arg4[TOKEN_MAX];
        int divider = 0;
        int arg_pos = 0;
        int cmd_pos;
        
        for(cmd_pos = 0; cmd_pos < size; cmd_pos++){
                if(divider == 0)
                {
                        arg1[arg_pos++] = obj->arguments[cmd_pos];
                        arg_pos++;
                }
                if(divider == 1)
                {
                        arg2[arg_pos] = obj->arguments[cmd_pos];
                        arg_pos++;
                }

                if(divider == 2)
                {
                        arg3[arg_pos] = obj->arguments[cmd_pos];
                        arg_pos++;
                }
                if(divider == 3)
                {
                        arg4[arg_pos] = obj->arguments[cmd_pos];
                        arg_pos++;
                }   
                if(obj->arguments[cmd_pos] == NULL){
                        divider++;
                        arg_pos = 0;
                }
        }
        char **pipe_cmds[] = {arg1, arg2, arg3, arg4, NULL};
        pipe = pipe_cmds;
 }

int *execute_pipe(char ***obj, int *size){
        /* Collect child exit status */
        static int exit_status[PIPE_ARG_MAX];
        int exit_pos = 0;
        pid_t pid;
        int fd[2];
        int fd_in = 0;

        while (*obj != NULL) 
        {
                pipe(fd);
                if ((pid = fork()) == -1){
                        exit(EXIT_FAILURE);
                }
                
                else if (pid == 0){ 
                        dup2(fd_in, 0); 
                        if (*(obj + 1) != NULL)
                                dup2(fd[1], 1); 
                        close(fd[0]);
                        exit_status[exit_pos]  = execvp((*obj)[0], *obj); 
                        exit_pos++;
                        exit(EXIT_FAILURE);
                } 
                else{
                        wait(NULL);
                        close(fd[1]);
                        fd_in = fd[0]; 
                        obj++;
                }
        }

        *size = exit_pos;
        return exit_status;
}

int output_redirection(char** args, int cmd_pos) {
        char* path = malloc(sizeof(char) * PATH_MAX);
        char** new_args = malloc(sizeof(char*) * TOKEN_MAX);
        char program_name[strlen(args[0])-2];
        int fd = 0, ret = 0;
        if(!path || !new_args){
                perror("ERROR: Malloc");
                exit(EXIT_FAILURE);
        }
        if(args[cmd_pos+1] == NULL) {
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
        /* Variable Initiation */
        char cmd[CMDLINE_MAX]; 
        char **token;
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
                char *print_cmd = strdup(cmd);
                strtok(print_cmd, "\n");

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                /* Parse the commands */
                token = parse_cmd(&command, cmd, &size);
<<<<<<< HEAD
                      
=======
                if(size > MAX_ARGS) {
                        fprintf(stderr, "Error: too many process arguments\n");
                        continue;
                }
>>>>>>> 1208b3d2ddbc0dd711e39df7d8c5f5b7e7dba7de
                /* Check whether Pipe or Output Redirection or Regular Command  */
                int cmd_pos, pipe_pos = -1;;
                char *pipe_dilimiter = "|";
                for(cmd_pos = 0; cmd_pos < size; cmd_pos++){
                        if(!strcmp(token[cmd_pos], pipe_dilimiter)){
                                pipe_count++;
                                *command.arguments[cmd_pos] = '\0';
                                pipe_pos = cmd_pos;
                        }
<<<<<<< HEAD
                }

=======
                        if((pipe_count) && (cmd_pos = size - 1))
                                *command.arguments[cmd_pos] = '\0';
                } 
                cmd_pos = 0;
>>>>>>> 1208b3d2ddbc0dd711e39df7d8c5f5b7e7dba7de
                for(cmd_pos = 0; cmd_pos < size; cmd_pos++) {
                        if(strstr(token[cmd_pos], ">") != NULL) {
                                output_red = 1;
                                break;
                        }
                }
<<<<<<< HEAD

=======
                
                if(pipe_count !=0 && output_red == 1 && cmd_pos < pipe_pos) {
                        fprintf(stderr,"Error: mislocated ouput redirection\n");
                        continue;
                }
>>>>>>> 1208b3d2ddbc0dd711e39df7d8c5f5b7e7dba7de
                /* Execute commands corresponding to the types */
                int retpipe[PIPE_ARG_MAX]; 
                if(pipe_count){
                        char **pipe_cmds[TOKEN_MAX];
                        helper_pipe(pipe_cmds, &command, size);
                        *retpipe = *execute_pipe(pipe_cmds, &size_exit);
                }
                if(output_red)
                        retval = output_redirection(token,cmd_pos);
                else
                        retval = execute_cmd(token);
                
                /* Decide action correspond to returned exit cod */
                if((retval == 0) && (!pipe_count))
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
        }

        return EXIT_SUCCESS;
}
