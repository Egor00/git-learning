//parent.c
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

static char CHILD_PROGRAM_NAME[] = "child";

int main(int argc, char **argv) {
	if (argc != 2) {
		char msg[] = "Usage: ./parent path_to_input\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_SUCCESS);
	}

	// NOTE: Get full path to the directory, where program resides
	char progpath[1024];
	{
		// NOTE: Read full program path, including its name
		ssize_t len = readlink("/proc/self/exe", progpath, sizeof(progpath) - 1);
		if (len == -1) {
			const char msg[] = "error: failed to read full program path\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
		}

		// NOTE: Trim the path to first slash from the end
		while (progpath[len] != '/')
			--len;

		progpath[len] = '\0';
	}

	// NOTE: Open pipes
	int client_to_server[2]; // AB
	if (pipe(client_to_server) == -1) {
		const char msg[] = "error: failed to create pipe\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	}

	int server_to_client[2]; // BA
	if (pipe(server_to_client) == -1) {
		const char msg[] = "error: failed to create pipe\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	}

	// NOTE: Spawn a new process
	
	const pid_t child = fork();

	switch (child) {
	case -1: { // NOTE: Kernel fails to create another process
		const char msg[] = "error: failed to spawn new process\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	} break;

	case 0: { // NOTE: We're a child, child doesn't know its pid after fork
		{
			pid_t pid = getpid(); // NOTE: Get child PID

			char msg[] = "I'm a child\n";
			write(STDOUT_FILENO, msg, sizeof(msg));
		}

		close(client_to_server[1]);
		close(server_to_client[0]);

		dup2(client_to_server[0], STDIN_FILENO);
		close(client_to_server[0]);

		dup2(server_to_client[1], STDOUT_FILENO);
		close(server_to_client[1]);

		{
			char path[1024];
			int i = 0;

			while (progpath[i] != '\0' && i < sizeof(path) - 2) {
    			path[i] = progpath[i];
    			i++;
			}

			path[i++] = '/';
	
			int j = 0;
			while (CHILD_PROGRAM_NAME[j] != '\0' && i < sizeof(path) - 1) {
    			path[i++] = CHILD_PROGRAM_NAME[j++];
			}

			path[i] = '\0';
			// NOTE: args[0] must be a program name, next the actual arguments
			// NOTE: `NULL` at the end is mandatory, because `exec*`
			//       expects a NULL-terminated list of C-strings
			
			char *const args[] = {CHILD_PROGRAM_NAME, argv[1], NULL};
			
			int32_t status = execv(path, args);
			
			if (status == -1) {
				const char msg[] = "error: failed to exec into new exectuable image\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
			}
			
		}
		
		
	} break;

	default: { // NOTE: We're a parent, parent knows PID of child after fork
		{
			pid_t pid = getpid(); // NOTE: Get parent PID

			char msg[] = "I'm a parent, my child has PID that i don't wanna share\n";
			write(STDOUT_FILENO, msg, sizeof(msg));
		}

		close(client_to_server[0]);
		close(server_to_client[1]);

		

		int input = open(argv[1], O_RDONLY);
		if (input == -1) {
    		const char msg[] = "error: failed to open input file\n";
    		write(STDERR_FILENO, msg, sizeof(msg) - 1);
    		exit(EXIT_FAILURE);
		}

		char buf[4096];
		ssize_t bytes;


		while ((bytes = read(input, buf, sizeof(buf))) > 0) {
    		ssize_t pos = 0;

    		while (pos < bytes) {
        		ssize_t n = write(client_to_server[1], buf + pos, bytes - pos);
        		if (n <= 0)
            		exit(EXIT_FAILURE);
        		pos += n;
    		}
		}

		close(input);
		close(client_to_server[1]);

		if (bytes < 0)
    		exit(EXIT_FAILURE);

		while ((bytes = read(server_to_client[0], buf, sizeof(buf))) > 0) {
    		write(STDOUT_FILENO, buf, bytes);
		}

		close(server_to_client[0]);
		wait(NULL);

		
	} break;
	}

}