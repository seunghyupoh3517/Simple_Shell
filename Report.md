# Simple Shell Report

### Read & Parse Commands
Read the commands from Standard input into _char* cmd_ and return the string.
With the returned C string, we parse the string into the struct object, _struct commands_ with double pointer variable, _char **arguments_ in it.
- We dynmaically allocate the memory of max token size,  _TOKEN MAX_ to _** token_.
- We parse the string using _*strtok_ with delimiters, _\t\r\n_ to capture and remove white spaces between each argument in command string.
- While parsing throughout the string, we count the size of arugments, numbers of argument in the commands, and direct the pointer to the size to counted numbers.
- We assign parsed command string whihc is tokenized into each argument to data structure's _char ** arguments_ and return it.

### Executing Commands
Unless the command arugment indicates _output redirection_ or _piping arguments_, the commands gets to executed through regualr _int execute cmd(char ** args)_. The execution commands models the behavior of _system()_ in low level to use for implementing realistic shell and return the return value, the corresponding exit value.
- We first differentiate if the arguments are one of the built in commands; then, if so, we detect which built in command is called by using _built cmd_, tagging integer value and execute the _execute builtin commands(args, built cmd)_ 
- With the rest of the commands, we simulate _fork()+exec()+wait()_ method: create a  child process, execute child process the speicified command while the parent process waits until child process ends executing and the parent process collects the exit value.
- _execvp_ is used to execute process in order to execute the complete paths of the commands, implicitly search accrdoing to _$PATH_ enviromnet variable.
- We return the collected single exit value.

### Pipeline Commands
Once the command string is parsed and the string is detected as cotaining the _pipe dilimiter_, "|", we count numbers of pipe in order to check how many arguments is input-ed. Then we use the separate pipe functions to execute the pipe using the raw command input line to begin with. As we already have checked whether there is pipeline sign in the input commands or not, we can decide to run pipe command functions.
- In order to maintain the several processes with several commands keywords and following arguments, we use  _char*[]_ argument of _CMDLINE_MAX_ in struct data strucute.
- We are utilizing same input with different, dynamic memory allocation.
- We first parse the command string with pipe, '|' delimiter, parse it using _strchr(),_ returning the first pattern of char pointed by the str and then pass it to the while loop which has _execute pipe()_ associated and run until the next pointer points to NULL or the count has already reached the pipe_count. From the parse, we find a pair of command and following arguments for every iteration. 
- Then we first remove the leading white space, _char* leading space()_ using _isspace_ and read, parse into command and folllowing arguments and sepearte the cases into possible scenarios utilizing three integer flag variables, child, first, last.
- If it's the first process, we dup2, output to input to next process, first process to the next on.
- Else if it's the process in between, neither first nor last flag raised, dup2 from child flag value to next process's input, read the current process's output and pass on to next process - technically, processing both operations from first, last processes.
- Else it's the last process to, read the output from prior process and run it.
- Lastly, we collect all the exit status using _int retpipe[PIPE ARG MAX];_ and _int *checker_.

### Built in Commands
Once one of the built in commands is detected then we call *execute_builtin_commands* with the appropriate enum element. Inside the *execute_builtin_commands* function, we find the command that was passed and move to another function to execute it.
1- *execute_pwd* function for pwd command:
In this function, we do a system call to get the current directory path using *getcwd*. If we get the current directory we print it, otherwise we print an error message saying that it failed.
2- *execute_cd* function for cd command, we pass the path that we want to change into:
In this function, we do a system call to change the directory we are at to whatever *path* is. We do this using *chdir*. After *chdir* returns, we check if it succeeded depending on its return value.
3- *execute_sls* function for the extra feature command sls:
In this function, we use the data type DIR to save the current directory and we use the struct dirent to get specific information about the directory. Here we used three system calls; *opendir*, *readdir*, *stat*, and *closedir*. First we open the directory and check if it opened correctly, if not we print an error message and return. However, if we succeed in opening it, we continue to read every element in the directory. We use *stat* to get the size of the file and print the files in order. Finally we close the directory.

### Output Redirection
After we parse the command, we check using *strstr* if > is a substring of the command that was given by the user. If ">" is spotted then we call *output_redirection*. This function recieves two arguments; the parsed command and the position of ">".
- We allocate memory for an array of strings that will hold the new arguments for the executable or command given without ">" and the file name. 
- We open the file with apporpriate macros depending if the user asked to append to a file or not. 
- We save the STDOUT file descriptor to return all file descriptors back to normal after we redirected the output.
- We use *dup2* to change stdout from outputing to the shell to the file given.
- We check whether we're redirecting the output of a regular command or not.
- If we are redirecting the ouput of a normal executable we find the path to the executable using *realpath*. We construct the argument that will be sent to exec and we then call *execute_cmd* to execute the executable.
- If we are redirecting the output of a regular command, we remove the output redirection symbol and the file name and call *execute_cmd*.
- Finally, we free the memory, return stdout to its original file descriptor and close all the open file descriptors.


  
### References
1- https://pubs.opengroup.org/onlinepubs/009695399/functions/ was used to understand system calls more for *execute_builtin_commands*.
2- https://pubs.opengroup.org/onlinepubs/000095399/functions/realpath.html and https://www.geeksforgeeks.org/dup-dup2-linux-system-call/ helped with output redirection. 
3- https://stackoverflow.com/questions/12784766/check-substring-exists-in-a-string-in-c for strings.
4- http://www.tutorialdost.com/C-Programming-Tutorial/22-C-Structure-Function.aspx#:~:text=Structure%20can%20be%20passed%20to,value%20or%20by%20references%20%2F%20addresses to grasp better understanding of Strucuture object & Functions in C
5- https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm was used to understand application of strtok(), tokenizing the string with the delimiters
6- https://www.geeksforgeeks.org/making-linux-shell-c/ was used to understand how piping works
7- https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way to remove leading white space every time

###### By *Gharam Alsaedi* & *Seunghyup Alex Oh*
