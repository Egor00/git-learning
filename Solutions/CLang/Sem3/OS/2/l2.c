#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define _POSIX_C_SOURCE 200809L
#define MAX_ITERATIONS 100000000


typedef struct {
    uint64_t experiments;
    uint64_t rounds;
    uint64_t score1;
    uint64_t score2;
    uint64_t wins1;
    uint64_t wins2;
    uint64_t draws;
    uint64_t seed;
} ThreadData;


static uint64_t random_next(uint64_t *state) {
    uint64_t x = *state;

    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;

    *state = x;
    return x;
}


static uint64_t roll_dice(uint64_t *seed) {
    return (random_next(seed) % 6 + 1) + (random_next(seed) % 6 + 1);
}

static double get_time_ms(void) {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

static int simulate_experiments(ThreadData *data, uint64_t seed) {
    uint64_t wins1 = 0;
    uint64_t wins2 = 0;
    uint64_t draws = 0;

    if(data->experiments > MAX_ITERATIONS) 
        return 0;

    for (uint64_t i = 0; i < data->experiments; i++) {
        uint64_t s1 = data->score1;
        uint64_t s2 = data->score2;

        uint64_t local_seed = random_next(&seed) | 1ULL; // чтобы не было 0

        for (uint64_t r = 0; r < data->rounds; r++) {
            s1 += roll_dice(&local_seed);
            s2 += roll_dice(&local_seed);
        }

        if (s1 > s2)
            wins1++;
        else if (s2 > s1)
            wins2++;
        else
            draws++;
    }

    data->wins1 = wins1;
    data->wins2 = wins2;
    data->draws = draws;
    return 1;
}


static void *worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    if(simulate_experiments(data, data->seed) == 0) {
        puts("Ошибка! Слишком много эксперимен тов!");
    }

    return NULL;
}


static double run_sequential(uint64_t experiments, uint64_t rounds, uint64_t score1, uint64_t score2, ThreadData *result) {
    ThreadData data = {.experiments = experiments, .rounds = rounds, .score1 = score1, .score2 = score2, .seed = 123456789ULL };

    double start = get_time_ms();

    if(simulate_experiments(&data, data.seed) == 0) {
        puts("Ошибка! Слишком много экспериментов!");
        return -1.0;
    };

    double end = get_time_ms();

    *result = data;

    return end - start;
}


static double run_parallel( uint64_t experiments, uint64_t rounds, uint64_t score1, uint64_t score2, int thread_count, ThreadData *result) {
    
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);

    ThreadData *data = (ThreadData*)calloc(thread_count, sizeof(ThreadData));

    if (!threads || !data) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        free(threads);
        free(data);
        return -1.0;
    }

    uint64_t base = experiments / thread_count;
    uint64_t remainder = experiments % thread_count;

    for (int i = 0; i < thread_count; i++) {
        data[i].experiments = base + (i < remainder ? 1 : 0);

        data[i].rounds = rounds;
        data[i].score1 = score1;
        data[i].score2 = score2;
        
        if((1000003ULL > INT64_MAX/(i+1)) && (123456789ULL > (INT64_MAX - (i + 1) * 1000003ULL))) {
            fprintf(stderr, "Ошибка! Слишком много потоков\n");
            free(threads);
            free(data);
            return -1.0;
        }

        data[i].seed = 123456789ULL + (uint64_t)(i + 1) * 1000003ULL;
    }

    double start = get_time_ms();

    int created = 0;

    for (int i = 0; i < thread_count; i++) {
        int rc = pthread_create(&threads[i], NULL, worker, &data[i]);

        if (rc != 0) {
            fprintf(stderr, "Ошибка создания потока %d: %d\n", i, rc);
            break;
        }

        created++;
    }

    for (int i = 0; i < created; i++) {
        if(pthread_join(threads[i], NULL)!=0) {
            free(threads);
            free(data);
            return -1.0;
        }
    }

    double end = get_time_ms();

    if (created != thread_count) {
        free(threads);
        free(data);
        return -1.0;
    }

    result->wins1 = 0;
    result->wins2 = 0;
    result->draws = 0;

    for (int i = 0; i < thread_count; i++) {
        result->wins1 += data[i].wins1;
        result->wins2 += data[i].wins2;
        result->draws += data[i].draws;
    }

    free(threads);
    free(data);

    return end - start;
}

static void print_results(const ThreadData *result, uint64_t experiments) {
    printf("Победы игрока 1: %" PRIu64 "\n", result->wins1);

    printf("Победы игрока 2: %" PRIu64 "\n", result->wins2);

    printf("Ничьи: %" PRIu64 "\n", result->draws);

    printf("Вероятность победы игрока 1: %.4f%%\n", 100.0 * result->wins1 / experiments);

    printf("Вероятность победы игрока 2: %.4f%%\n", 100.0 * result->wins2 / experiments);

    printf("Вероятность ничьей: %.4f%%\n", 100.0 * result->draws / experiments);
}

static void add_thread_count(int *counts, int *size, int value, int max_threads) {
    if (value < 1 || value > max_threads)
        return;

    for (int i = 0; i < *size; i++) {
        if (counts[i] == value)
            return;
    }

    counts[(*size)++] = value;
}

int main(int argc, char **argv) {
    if (argc < 6 || argc > 7) {
        printf("Использование:\n");
        printf("%s K(number of rounds) current_round score1 score2 " "experiments [max_threads]\n", argv[0]);
        return 1;
    }

    uint64_t K = strtoull(argv[1], NULL, 10);
    uint64_t current_round = strtoull(argv[2], NULL, 10);
    uint64_t score1 = strtoull(argv[3], NULL, 10);
    uint64_t score2 = strtoull(argv[4], NULL, 10);
    uint64_t experiments = strtoull(argv[5], NULL, 10);

    int max_threads = argc == 7 ? atoi(argv[6]) : 1024;

    if (K == 0 || current_round > K || experiments == 0 || max_threads < 1 ||
        score1 > UINT64_MAX - K * 12 || score2 > UINT64_MAX - K * 12) {
        fprintf(stderr, "Некорректные входные данные\n");
        return 1;
    }

    uint64_t remaining_rounds = K - current_round;

    long logical_cores = sysconf(_SC_NPROCESSORS_ONLN);

    if (logical_cores < 1)
        logical_cores = 1;

    printf("Количество логических ядер: %ld\n", logical_cores);

    printf("Всего туров: %" PRIu64 "\n", K);
    printf("Текущий тур: %" PRIu64 "\n", current_round);
    printf("Оставшихся туров: %" PRIu64 "\n", remaining_rounds);

    printf("Начальные очки: %" PRIu64 " : %" PRIu64 "\n", score1, score2);

    printf("Количество экспериментов: %" PRIu64 "\n", experiments);

    printf("Максимальное число потоков: %d\n\n", max_threads);

    ThreadData seq_result = {0};

    printf("=== ПОСЛЕДОВАТЕЛЬНЫЙ АЛГОРИТМ ===\n");

    double sequential_time = run_sequential(experiments, remaining_rounds, score1, score2, &seq_result);

    print_results(&seq_result, experiments);

    printf("Время выполнения: %.3f мс\n\n", sequential_time);

    printf("=== ПАРАЛЛЕЛЬНЫЙ АЛГОРИТМ ===\n");

    int counts[16];
    int count_size = 0;

    int cores = (int)logical_cores;

    if (cores >= 5) {
        add_thread_count(counts, &count_size, 1, max_threads);

        add_thread_count(counts, &count_size, cores / 4, max_threads);/*2*/

        add_thread_count(counts, &count_size, cores / 2, max_threads);/*4*/

        add_thread_count(counts, &count_size, cores - 1, max_threads);/*7*/

        for (int i = 1; count_size < 4 && i < cores; i++) 
            add_thread_count(counts, &count_size, i, max_threads);
    }

    add_thread_count(counts, &count_size, cores, max_threads);


    int large_values[] = {16, 128, 1024};

    for (int i = 0; i < 3; i++) {
        int value = large_values[i];

        if (value <= cores)
            value = cores + 1;

        add_thread_count(counts, &count_size, value, max_threads);
    }

    printf("%-15s %-20s %-15s %-15s\n", "Потоки", "Время (мс)", "Ускорение", "Эффективность");

    printf("%-15s %-20.3f %-15.3f %-15.3f\n", "Sequential", sequential_time, 1.0, 1.0);

    for (int i = 0; i < count_size; i++) {
        int p = counts[i];

        ThreadData parallel_result = {0};

        double parallel_time = run_parallel(experiments, remaining_rounds, score1, score2, p, &parallel_result);

        if (parallel_time < 0)
            continue;

        double speedup = sequential_time / parallel_time;

        double efficiency = speedup / p;

        printf("%-15d %-20.3f %-15.3f %-15.3f\n", p, parallel_time, speedup, efficiency);
    }


    return 0;
}
