#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#define CMDLINE_MAX 512
#define PATH_MAX 4096 
#define TOKEN_MAX 32
#define PIPE_ARG_MAX 4
#define MAX_ARGS 16

enum {exit_cmd = 0, pwd_cmd = 1, cd_cmd = 2, sls_cmd = 3};
enum {read_fd = 0, write_fd = 1};

static char* args[CMDLINE_MAX];
int execute_pipe(int child, int first, int last)
{       printf("DEGUG ex1\n");
        pid_t pid;
	int fd[2];
	pipe(fd);	
	pid = fork();

        printf("DEGUG ex5\n");
	if (pid == 0) {
		if (first == 1 && last == 0 && child == 0){
			dup2( fd[write_fd], STDOUT_FILENO );
		} 
                else if (first == 0 && last == 0 && child != 0) {
			dup2(child, STDIN_FILENO);
			dup2(fd[write_fd], STDOUT_FILENO);
		} 
                else {
			dup2( child, STDIN_FILENO );
		}
                printf("DEGUG ex2\n");
		if (execvp( args[0], args) == -1)
			exit(EXIT_FAILURE); 
	}
        printf("DEGUG ex3\n");
	if (child != 0) 
		close(child);
	close(fd[write_fd]);
        printf("DEGUG ex4\n");
	if (last == 1)
		close(fd[read_fd]);
 
	return fd[read_fd];
}

char* leadingspace(char* s)
{
	while (isspace(*s)) ++s;
	return s;
}

 void split(char* cmd)
{
	cmd = leadingspace(cmd);
        printf("%s\n", cmd);
	char* next = strchr(cmd, ' ');
        printf("%s\n", next);
	int i = 0;
 	printf("DEGUG split1\n");
	while(next != NULL) {
		next[0] = '\0';
		args[i] = cmd;
		i++;
		cmd = leadingspace(next + 1);
		next = strchr(cmd, ' ');
	}
	if (cmd[0] != '\0') {
		args[i] = cmd;
		next = strchr(cmd, '\n');
		i++; 
	}
        printf("DEGUG split2\n");
	args[i] = NULL;
}

int exe(char* cmd, int child, int first, int last)
{
	split(cmd);
        printf("DEGUG run1\n");
	if (args[0] != NULL) {
                printf("DEGUG  run2\n");
		//cmd = leadingspace(cmd);
		return execute_pipe(child, first, last);
	}
	return EXIT_SUCCESS;
}

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
                fprintf(stdout, "%s (%ld bytes)\n", dp->d_name, sb.st_size);
                // ld
                depth++;
        }
    }
    if(depth == 0) {
        fprintf(stdout,"empty (0 bytes)\n");
    }
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

        token[pointer] = NULL; //  "\0": string literal holding '\0' plus second one as a terminator
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

int output_redirection(char** args, int cmd_pos, int size) {
        char* path = malloc(sizeof(char) * PATH_MAX);
        char** new_args = malloc(sizeof(char*) * size);
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
                int retval = 0;
                int size;
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
                token = parse_cmd(&command, piping_cmd, &size);
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
                
                int child = 0;
                int first = 1;
                int checker = 0;
                int retpipe[PIPE_ARG_MAX]; 
                //struct commands pip;
                if(pipe_count){
                        char *p_cmd = cmd; 
                        char *next = strchr(p_cmd, '|'); 

                        while(next!=NULL && checker < pipe_count){
                                *next = '\0';
                                child  = exe(p_cmd, child, first, 0);
                                retpipe[checker] = child;
                                checker++;
                                printf("DEGUG main\n");
                                p_cmd = next + 1;
			        next = strchr(cmd, '|'); 
			        first = 0;
		        }

                        retpipe[checker] = child;
		        child = exe(p_cmd, child, first, 1);
		        wait(NULL);
                }
                else if(output_red)
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
                        
                        for(i = 0; i < pipe_count; i++)
                                fprintf(stderr, "[%d]", 
                                retpipe[i]);
                        fprintf(stderr, "\n");
                }
                else if(retval == -1)
                        break;

                free(print_cmd);
                free(token);
                free(piping_cmd);
        }
        free(print_cmd);
        free(token);
        free(piping_cmd);

        return EXIT_SUCCESS;
}