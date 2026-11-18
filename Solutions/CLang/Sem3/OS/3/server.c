#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>

#define SHM_SIZE 4096

typedef struct {
    uint32_t length;
    uint32_t eof;
    char data[SHM_SIZE - 2 * sizeof(uint32_t)];
} SharedData;

static void error_exit(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s path_to_input\n", argv[0]);
        return EXIT_FAILURE;
    }


    pid_t pid = getpid();

    char shm_name[128];
    char sem_empty_name[128];
    char sem_full_name[128];

    snprintf(shm_name, sizeof(shm_name), "/lab_shm_%ld", (long)pid);

    snprintf(sem_empty_name, sizeof(sem_empty_name), "/lab_empty_%ld", (long)pid);

    snprintf(sem_full_name, sizeof(sem_full_name), "/lab_full_%ld", (long)pid);

    int shm = shm_open(shm_name, O_CREAT | O_EXCL | O_RDWR, 0600);

    if (shm == -1)
        error_exit("error: failed to create SHM");

    if (ftruncate(shm, sizeof(SharedData)) == -1) {
        shm_unlink(shm_name);
        close(shm);
        error_exit("error: failed to resize SHM");
    }

    SharedData *shared = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);

    if (shared == MAP_FAILED) {
        shm_unlink(shm_name);
        close(shm);
        error_exit("error: failed to map SHM");
    }

    sem_t *sem_empty = sem_open(sem_empty_name, O_CREAT | O_EXCL, 0600, 1);

    if (sem_empty == SEM_FAILED) {
        munmap(shared, sizeof(SharedData));
        shm_unlink(shm_name);
        close(shm);
        error_exit("error: failed to create empty semaphore");
    }

    sem_t *sem_full = sem_open(sem_full_name, O_CREAT | O_EXCL, 0600, 0);

    if (sem_full == SEM_FAILED) {
        sem_close(sem_empty);
        sem_unlink(sem_empty_name);

        munmap(shared, sizeof(SharedData));
        shm_unlink(shm_name);
        close(shm);

        error_exit("error: failed to create full semaphore");
    }

    char executable_path[1024];

    ssize_t path_length = readlink("/proc/self/exe", executable_path, sizeof(executable_path) - 1);//ищет полный путь к собственному исп файлу(не ставит \0)

    if (path_length == -1) {
        sem_close(sem_empty);
        sem_close(sem_full);

        sem_unlink(sem_empty_name);
        sem_unlink(sem_full_name);

        munmap(shared, sizeof(SharedData));
        shm_unlink(shm_name);
        close(shm);

        error_exit("error: failed to get executable path");
    }

    executable_path[path_length] = '\0';

    char *last_slash = strrchr(executable_path, '/');//последнее вхождение

    if (last_slash == NULL) {
        fprintf(stderr, "error: failed to determine executable directory\n");

        sem_close(sem_empty);
        sem_close(sem_full);

        sem_unlink(sem_empty_name);
        sem_unlink(sem_full_name);

        munmap(shared, sizeof(SharedData));
        shm_unlink(shm_name);
        close(shm);

        return EXIT_FAILURE;
    }

    *last_slash = '\0';

    char client_path[1024];

    int n = snprintf(client_path, sizeof(client_path), "%s/client", executable_path);

    if (n < 0 || (size_t)n >= sizeof(client_path)) {
        fprintf(stderr, "error: client path is too long\n");

        sem_close(sem_empty);
        sem_close(sem_full);

        sem_unlink(sem_empty_name);
        sem_unlink(sem_full_name);

        munmap(shared, sizeof(SharedData));
        shm_unlink(shm_name);
        close(shm);

        return EXIT_FAILURE;
    }

    pid_t client = fork();

    if (client == -1) {
        sem_close(sem_empty);
        sem_close(sem_full);

        sem_unlink(sem_empty_name);
        sem_unlink(sem_full_name);

        munmap(shared, sizeof(SharedData));
        shm_unlink(shm_name);
        close(shm);

        error_exit("error: failed to fork");
    }

    if (client == 0) {

        char *args[] = {"client", shm_name, sem_empty_name, sem_full_name, NULL};

        execv(client_path, args);

        perror("error: failed to exec client");

        _exit(EXIT_FAILURE);
    }

    printf("%ld: I am a server, client's PID: %ld\n", (long)getpid(), (long)client);

    int input = open(argv[1], O_RDONLY);

    if (input == -1) {
        perror("error: failed to open input file");

        if (sem_wait(sem_empty) == -1)
            perror("sem_wait");

        shared->length = 0;
        shared->eof = 1;

        if (sem_post(sem_full) == -1)
            perror("sem_post");

        waitpid(client, NULL, 0);

        sem_close(sem_empty);
        sem_close(sem_full);

        sem_unlink(sem_empty_name);
        sem_unlink(sem_full_name);

        munmap(shared, sizeof(SharedData));
        shm_unlink(shm_name);
        close(shm);

        return EXIT_FAILURE;
    }

    char buffer[SHM_SIZE - 2 * sizeof(uint32_t)];

    while (1) {

        ssize_t bytes = read(input, buffer, sizeof(buffer));

        if (bytes == -1) {
            if (errno == EINTR)
                continue;

            perror("error: failed to read input file");
            break;
        }


        while (sem_wait(sem_empty) == -1) {//может быть прерван другим сигналом. если нетб то реально ошибка
            if (errno == EINTR)
                continue;

            perror("error: failed to wait for SHM");
            close(input);

            waitpid(client, NULL, 0);

            sem_close(sem_empty);
            sem_close(sem_full);

            sem_unlink(sem_empty_name);
            sem_unlink(sem_full_name);

            munmap(shared, sizeof(SharedData));
            shm_unlink(shm_name);
            close(shm);

            return EXIT_FAILURE;
        }

        if (bytes == 0) {


            shared->length = 0;
            shared->eof = 1;

            if (sem_post(sem_full) == -1)
                perror("error: failed to signal EOF");

            break;
        }

        shared->length = (uint32_t)bytes;
        shared->eof = 0;

        memcpy(shared->data, buffer, (size_t)bytes);

        if (sem_post(sem_full) == -1) {
            perror("error: failed to signal client");
            break;
        }
    }

    close(input);

    int status;

    if (waitpid(client, &status, 0) == -1) {
        perror("error: failed to wait for client");
    }


    if (sem_close(sem_empty) == -1)
        perror("sem_close empty");

    if (sem_close(sem_full) == -1)
        perror("sem_close full");

    if (sem_unlink(sem_empty_name) == -1)
        perror("sem_unlink empty");

    if (sem_unlink(sem_full_name) == -1)
        perror("sem_unlink full");

    if (munmap(shared, sizeof(SharedData)) == -1)
        perror("munmap");

    if (shm_unlink(shm_name) == -1)
        perror("shm_unlink");

    if (close(shm) == -1)
        perror("close");

    return EXIT_SUCCESS;
}