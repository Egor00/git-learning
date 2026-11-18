#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>

#include <unistd.h>
#include <fcntl.h>
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

static void summ(const char *buf, uint32_t len)
{
    char strd[512];

    double sum = 0.0;

    uint32_t current_length = 0;

    for (uint32_t i = 0; i < len; i++) {

        if (buf[i] == ' ' || buf[i] == '\t' ||  buf[i] == '\n' || buf[i] == '\r') {

            if (current_length == 0)
                continue;

            strd[current_length] = '\0';

            char *end = NULL;

            errno = 0;

            double value = strtod(strd, &end);


            if (end == strd || *end != '\0') {

                char msg[1024];

                int n = snprintf(msg, sizeof(msg), "Error! Found in input: %s\n", strd);

                if (n < 0 || (size_t)n >= sizeof(msg)) {
                    const char error_message[] = "Error! Output overflow!\n";

                    write(STDOUT_FILENO, error_message, sizeof(error_message) - 1);

                    return;
                }

                write(STDOUT_FILENO, msg, (size_t)n);

                return;
            }

            if (errno == ERANGE || isinf(value)) {

                const char msg[] = "Error! Number is too large!\n";

                write( STDOUT_FILENO, msg, sizeof(msg) - 1 );

                return;
            }

            sum += value;

            if (isinf(sum)) {

                const char msg[] = "Error! Summary overflow!\n";

                write(STDOUT_FILENO, msg, sizeof(msg) - 1);

                return;
            }

            current_length = 0;
        }
        else {


            if (current_length >= sizeof(strd) - 1) {

                const char msg[] = "Error! Number is too long!\n";

                write(STDOUT_FILENO, msg, sizeof(msg) - 1);

                return;
            }

            strd[current_length++] = buf[i];
        }
    }

    if (current_length > 0) {

        strd[current_length] = '\0';

        char *end = NULL;

        errno = 0;

        double value = strtod(strd, &end );

        if (end == strd || *end != '\0') {

            char msg[1024];

            int n = snprintf(
                msg,
                sizeof(msg),
                "Error! Found in input: %s\n",
                strd
            );

            if (n < 0 || (size_t)n >= sizeof(msg)) {
                const char error_message[] =
                    "Error! Output overflow!\n";

                write(
                    STDOUT_FILENO,
                    error_message,
                    sizeof(error_message) - 1
                );

                return;
            }

            write(
                STDOUT_FILENO,
                msg,
                (size_t)n
            );

            return;
        }

        if (errno == ERANGE || isinf(value)) {

            const char msg[] =
                "Error! Number is too large!\n";

            write(
                STDOUT_FILENO,
                msg,
                sizeof(msg) - 1
            );

            return;
        }

        sum += value;

        if (isinf(sum)) {

            const char msg[] =
                "Error! Summary overflow!\n";

            write(
                STDOUT_FILENO,
                msg,
                sizeof(msg) - 1
            );

            return;
        }
    }

    char result[1024];

    int n = snprintf(
        result,
        sizeof(result),
        "%f\n",
        sum
    );

    if (n < 0 || (size_t)n >= sizeof(result)) {

        const char msg[] =
            "Error! Output overflow!\n";

        write(
            STDOUT_FILENO,
            msg,
            sizeof(msg) - 1
        );

        return;
    }

    if (write(
            STDOUT_FILENO,
            result,
            (size_t)n
        ) == -1) {
        perror("error: failed to write result");
    }
}

int main(int argc, char **argv)
{
    /*
     * server передаёт:
     *
     * argv[1] = SHM name
     * argv[2] = empty semaphore name
     * argv[3] = full semaphore name
     */
    if (argc != 4) {
        fprintf(
            stderr,
            "Usage: %s <shm> <empty semaphore> <full semaphore>\n",
            argv[0]
        );

        return EXIT_FAILURE;
    }

    const char *shm_name = argv[1];
    const char *sem_empty_name = argv[2];
    const char *sem_full_name = argv[3];

    /*
     * Открываем SHM, созданный server.
     */
    int shm = shm_open(
        shm_name,
        O_RDWR,
        0600
    );

    if (shm == -1)
        error_exit("error: failed to open SHM");

    /*
     * Отображаем SHM.
     */
    SharedData *shared = mmap(
        NULL,
        sizeof(SharedData),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        shm,
        0
    );

    if (shared == MAP_FAILED) {
        close(shm);
        error_exit("error: failed to map SHM");
    }

    /*
     * Открываем семафоры.
     */
    sem_t *sem_empty = sem_open(
        sem_empty_name,
        0
    );

    if (sem_empty == SEM_FAILED) {
        munmap(shared, sizeof(SharedData));
        close(shm);

        error_exit("error: failed to open empty semaphore");
    }

    sem_t *sem_full = sem_open(
        sem_full_name,
        0
    );

    if (sem_full == SEM_FAILED) {
        sem_close(sem_empty);
        munmap(shared, sizeof(SharedData));
        close(shm);

        error_exit("error: failed to open full semaphore");
    }

    printf(
        "%ld: I am a client\n",
        (long)getpid()
    );

    /*
     * Основной цикл.
     */
    while (true) {

        /*
         * Ждём, пока server запишет данные.
         */
        while (sem_wait(sem_full) == -1) {

            if (errno == EINTR)
                continue;

            perror("error: sem_wait");

            sem_close(sem_empty);
            sem_close(sem_full);

            munmap(shared, sizeof(SharedData));
            close(shm);

            return EXIT_FAILURE;
        }

        /*
         * EOF.
         */
        if (shared->eof) {

            /*
             * Освобождаем SHM.
             */
            if (sem_post(sem_empty) == -1)
                perror("error: sem_post empty");

            break;
        }

        /*
         * Защита от повреждённой длины.
         */
        if (shared->length >
            sizeof(shared->data)) {

            const char msg[] =
                "Error! Invalid shared memory data length!\n";

            write(
                STDOUT_FILENO,
                msg,
                sizeof(msg) - 1
            );

        } else {

            /*
             * Обрабатываем данные непосредственно
             * из shared memory.
             */
            summ(
                shared->data,
                shared->length
            );
        }

        /*
         * Сообщаем server, что SHM снова свободна.
         */
        if (sem_post(sem_empty) == -1) {
            perror("error: failed to signal server");
            break;
        }
    }

    /*
     * Освобождаем ресурсы.
     */
    if (sem_close(sem_empty) == -1)
        perror("sem_close empty");

    if (sem_close(sem_full) == -1)
        perror("sem_close full");

    if (munmap(shared, sizeof(SharedData)) == -1)
        perror("munmap");

    if (close(shm) == -1)
        perror("close");

    return EXIT_SUCCESS;
}