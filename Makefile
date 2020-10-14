# Generate executable - Refer to sshell_ref CSIF executable
sshell: sshell.o
	gcc -Wall -Wextra -Werror -o sshell sshell.o
# Generate objects files from C files
sshell.o: sshell.c
	gcc -Wall -Wextra -Werror -c -o sshell.o sshell.c
# Clean generated files
clean:
	rm -f sshell sshell.o