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

int execute_sls() {
    int ret_val = 0;
    DIR *dirp;
    struct dirent *dp;
    dirp = opendir(".");
    if(dirp == NULL) {
        ret_val = 1;
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
                return cmd;
        }while(1);
}

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
                        if(strstr(token[cmd_pos],">") != NULL) {
                                output_red = 1;
                                break;
                        }
                }
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
        }

        return EXIT_SUCCESS;
}
