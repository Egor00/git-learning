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
<<<<<<< HEAD
	int32_t ls = 0;
	for(int32_t i = 0; i< len; i++) {
		if(buf[i]==' ') {
			errno = 0;
			double a2 = strtod(strd, NULL);
			if(errno == ERANGE) {
=======
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
>>>>>>> 468caa3 (labOS1)
				{
				char msg[] = "Error! Number is too large!\n";
				write(STDOUT_FILENO, msg, sizeof(msg));
				}
				return;
			}
<<<<<<< HEAD
			if(a1-(DBL_MAX-a2) > 0.00001) {
=======
			a1+=a2;

			if(isinf(a1)) {
>>>>>>> 468caa3 (labOS1)
				{
				char msg[] = "Error! Summary overflow!\n";
				write(STDOUT_FILENO, msg, sizeof(msg));
				}
				
				return;
			}
<<<<<<< HEAD
			a1+=a2;
			strd[0] = '\0';
			ls = 0;
		}
		else{
			strd[ls++] = buf[i];
=======
			strd[0] = '\0';
			l = 0;
		}
		else{
			strd[l++] = buf[i];
>>>>>>> 468caa3 (labOS1)
		}
	}

	char buff[1024];
<<<<<<< HEAD
	int32_t l = 0;

	int negative = (a1 < 0.0);
		double value = fabs(a1);

	if (value >= 9223372036854775808.0) {
    	const char msg[] = "Error! Output transformation overflow!\n";
    	write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    	return;
	}

/* Округляем абсолютное значение до шести знаков. */
	double scaled = floor(value * 1000000.0 + 0.5);

	if (!isfinite(scaled)) {
    	const char msg[] = "Error! Output transformation overflow!\n";
    	write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    	return;
	}

/* Для обычных значений разделяем целую и дробную части. */
	long long whole = (long long)(scaled / 1000000.0);
	long long frac = (long long)fmod(scaled, 1000000.0);

	if (negative && (whole != 0 || frac != 0))
    	buff[l++] = '-';

	char tmp[32];
	int n = 0;

	do {
    	tmp[n++] = '0' + (int)(whole % 10);
    	whole /= 10;
	} while (whole > 0);

	while (n > 0)
    	buff[l++] = tmp[--n];

	buff[l++] = '.';

	long long divisor = 100000;

	for (int i = 0; i < 6; i++) {
    	buff[l++] = '0' + (int)((frac / divisor) % 10);
    	divisor /= 10;
	}

	buff[l++] = '\n';
	write(STDOUT_FILENO, buff, l);

    /*char buff[1024];
    int32_t l = 0;
	double fraction = fabs(fabs(a1) - fabs(floor(a1)));
	long long frac = (long long)(fraction * 1000000.0 + 0.5);
	long long whole = (long long)a1;
    if (whole < 0) {
        buff[l++] = '-';
    }
	if(fabs(a1) > (double)LLONG_MAX) {
		{
		char msg[] = "Error! Output transformation overflow!\n";
		write(STDOUT_FILENO, msg, sizeof(msg));
		}
		return;
	}
    

    char tmp[1024];
    int n = 0;

    do {
        tmp[n++] = '0' + whole % 10;
        whole /= 10;
    } while (whole > 0);

    while (n > 0)
        buff[l++] = tmp[--n];
	
    buff[l++] = '.';

	tmp[0] = '\0';
    do {
        tmp[n++] = '0' + frac % 10;
        frac /= 10;
    } while (frac > 0);

	while (n > 0)
        buff[l++] = tmp[--n];

    buff[l++] = '\n';
    write(STDOUT_FILENO, buff, l);*/
=======
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
>>>>>>> 468caa3 (labOS1)

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
		
<<<<<<< HEAD
		// NOTE: Transform data
		

		/*{
			// NOTE: Log to file
			int32_t written = write(file, buf, bytes);
			if (written != bytes) {
				const char msg[] = "error: failed to write to file\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
			}

			{
				const char msg[] = "Server received: ";
				write(STDERR_FILENO, msg, sizeof(msg) - 1);
			}

			// NOTE: Echo back to client
			written = write(STDOUT_FILENO, buf, bytes);
			if (written != bytes) {
				const char msg[] = "error: failed to echo\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
			}
		}*/
=======

>>>>>>> 468caa3 (labOS1)
	}

	
	if (bytes == 0) {
		const char term = '\0';
		write(STDOUT_FILENO, &term, sizeof(term));
	}

	close(file);

}