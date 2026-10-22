//child.c
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>

void summ(char buf[], int32_t len){
	char strd[512];
	double a1 = 0.0;
	int32_t l = 0;
	for(int32_t i = 0; i< len; i++) {
		if(buf[i]==' ') {
			strd[l++] = '\0';
			char* end;
			double a2 = strtod(strd, &end);
			
			if(*end != '\0') {
				{
					char msg[1024];

					int32_t n = snprintf(msg, sizeof(msg), "Error! Found in input: %s\n", end);
					if(n > sizeof(msg) || n == -1) 
					{
						char msg2[] = "Error! Output overflow!\n";
						write(STDOUT_FILENO, msg2, sizeof(msg2));
					}
					return;
					write(STDOUT_FILENO, msg, n);
				}
				return;
			}
			if(isinf(a2)) {
				{
				char msg[] = "Error! Number is too large!\n";
				write(STDOUT_FILENO, msg, sizeof(msg));
				}
				return;
			}
			a1+=a2;

			if(isinf(a1)) {
				{
				char msg[] = "Error! Summary overflow!\n";
				write(STDOUT_FILENO, msg, sizeof(msg));
				}
				
				return;
			}
			strd[0] = '\0';
			l = 0;
		}
		else{
			strd[l++] = buf[i];
		}
	}

	char buff[1024];
	int32_t err = snprintf(buff, sizeof(buff), "%f\n", a1);
	if(err > sizeof(buff) || err == -1) 
	{
		{
			char msg[] = "Error! Output overflow!\n";
			write(STDOUT_FILENO, msg, sizeof(msg));
		}
		return;
	}

	write(STDOUT_FILENO, buff, err);

}

int main(int argc, char **argv){
	
    if(argc != 2) {
        char msg[] = "error1: failed to open requested file\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    }
    
	ssize_t bytes;

	pid_t pid = getpid();

	int32_t file = open(argv[1], O_RDONLY | O_CREAT , 0600);
	if (file == -1) {
		const char msg[] = "error2: failed to open requested file\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	}
	char c;
	int32_t len = 0;
	char buf[1024];


    while (bytes = read(file, &c, sizeof(char))) {
		if (bytes < 0) {
			const char msg[] = "error: failed to read from stdin\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
			break;
		}


		if(c == '\n') {
			buf[len++] = ' ';
			summ(buf, len);
			buf[0] = '\0';
			len = 0;
		}
		else {
			buf[len++] = c;
		}
		

	}

	
	if (bytes == 0) {
		const char term = '\0';
		write(STDOUT_FILENO, &term, sizeof(term));
	}

	close(file);

}