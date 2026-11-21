#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "myerrors.h"

typedef struct
{
    int id;
    int start;
    int finish;
} Report;


/*
 * Преобразование времени HH:MM в количество минут
 * от начала суток.
 */
static ErrorsDef parse_time(const char *str, int *minutes)
{
    int hours;
    int mins;
    char extra;

    if (str == NULL || minutes == NULL)
        return INVALID_ARGUMENT;

    /*
     * Дополнительный символ нужен для того,
     * чтобы "12:30abc" не считалось корректным временем.
     */
    if (sscanf(str, "%2d:%2d%c", &hours, &mins, &extra) != 2)
        return INVALID_NUMBER_FORMAT;

    if (hours < 0 || hours > 23 ||
        mins < 0 || mins > 59)
    {
        return INVALID_NUMBER;
    }

    *minutes = hours * 60 + mins;

    return STAT_OK;
}


/*
 * Чтение одной заявки.
 */
static ErrorsDef read_report(Report *report)
{
    char line[256];
    char start_str[16];
    char finish_str[16];
    char extra;

    if (report == NULL)
        return INVALID_ARGUMENT;

    if (fgets(line, sizeof(line), stdin) == NULL)
        return READFILE_ERROR;

    /*
     * Формат:
     * id start_time finish_time
     *
     * Четвёртый %c позволяет обнаружить лишние данные.
     */
    if (sscanf(line, "%d %15s %15s %c",
               &report->id,
               start_str,
               finish_str,
               &extra) != 3)
    {
        return INVALID_INPUT;
    }

    if (report->id <= 0)
        return INVALID_NUMBER;

    ErrorsDef status = parse_time(start_str, &report->start);

    if (status != STAT_OK)
        return status;

    status = parse_time(finish_str, &report->finish);

    if (status != STAT_OK)
        return status;

    /*
     * Начало должно быть строго раньше окончания.
     */
    if (report->start >= report->finish)
        return INVALID_INTERVAL;

    return STAT_OK;
}


/*
 * Проверка уникальности идентификаторов.
 */
static ErrorsDef check_unique_ids(const Report *reports, int n)
{
    if (reports == NULL)
        return INVALID_ARGUMENT;

    for (int i = 0; i < n; ++i)
    {
        for (int j = i + 1; j < n; ++j)
        {
            if (reports[i].id == reports[j].id)
                return INVALID_ARGUMENT;
        }
    }

    return STAT_OK;
}


/*
 * Сортировка заявок по времени окончания.
 *
 * При одинаковом времени окончания порядок произвольный,
 * поэтому дополнительно сортируем по времени начала и id
 * только для детерминированности.
 */
static int compare_reports(const void *a, const void *b)
{
    const Report *r1 = (const Report *)a;
    const Report *r2 = (const Report *)b;

    if (r1->finish < r2->finish)
        return -1;

    if (r1->finish > r2->finish)
        return 1;

    if (r1->start < r2->start)
        return -1;

    if (r1->start > r2->start)
        return 1;

    if (r1->id < r2->id)
        return -1;

    if (r1->id > r2->id)
        return 1;

    return 0;
}


/*
 * Решение задачи.
 *
 * Для каждого доклада, начиная с самого рано заканчивающегося,
 * ищем зал, в котором предыдущий доклад заканчивается
 * максимально поздно, но не позже начала текущего.
 *
 * Если такой зал существует — доклад можно поставить туда.
 * Если нет — этот доклад пропускаем.
 */
static ErrorsDef max_reports(const Report *reports,
                             int n,
                             int halls,
                             int *result)
{
    if (reports == NULL || result == NULL)
        return INVALID_ARGUMENT;

    if (n <= 0 || halls <= 0)
        return INVALID_ARRAY_SIZE;

    /*
     * finish_times[i] — время окончания последнего
     * поставленного доклада в i-м зале.
     *
     * -1 означает, что зал пока свободен.
     */
    if ((size_t)halls > SIZE_MAX / sizeof(int))
        return MEMORY_ERROR;

    int *finish_times = malloc((size_t)halls * sizeof(int));

    if (finish_times == NULL)
        return MEMORY_ERROR;

    for (int i = 0; i < halls; ++i)
        finish_times[i] = -1;

    int count = 0;

    for (int i = 0; i < n; ++i)
    {
        int best_hall = -1;
        int best_finish = -1;

        /*
         * Ищем зал, который подходит для текущего доклада.
         *
         * Берём зал с максимально поздним временем окончания.
         * Это оставляет остальные залы максимально свободными
         * для последующих докладов.
         */
        for (int j = 0; j < halls; ++j)
        {
            if (finish_times[j] <= reports[i].start)
            {
                if (best_hall == -1 ||
                    finish_times[j] > best_finish)
                {
                    best_hall = j;
                    best_finish = finish_times[j];
                }
            }
        }

        if (best_hall != -1)
        {
            finish_times[best_hall] = reports[i].finish;
            ++count;
        }
    }

    free(finish_times);

    *result = count;

    return STAT_OK;
}


int main(void)
{
    int n;
    int k;

    /*
     * Первая строка:
     * n — количество заявок
     * k — количество залов
     */
    if (scanf("%d %d", &n, &k) != 2)
    {
        errors(INVALID_INPUT);
        return 1;
    }

    /*
     * Проверяем положительность n и k.
     */
    if (n <= 0 || k <= 0)
    {
        errors(INVALID_ARGUMENT);
        return 1;
    }

    /*
     * Убираем остаток первой строки.
     */
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        /* nothing */
    }

    /*
     * Проверка возможности выделить массив.
     */
    if ((size_t)n > SIZE_MAX / sizeof(Report))
    {
        errors(MEMORY_ERROR);
        return 1;
    }

    Report *reports = malloc((size_t)n * sizeof(Report));

    if (reports == NULL)
    {
        errors(MEMORY_ERROR);
        return 1;
    }

    /*
     * Считываем заявки.
     */
    for (int i = 0; i < n; ++i)
    {
        ErrorsDef status = read_report(&reports[i]);

        if (status != STAT_OK)
        {
            errors(status);
            free(reports);
            return 1;
        }
    }

    /*
     * Проверяем уникальность id.
     */
    ErrorsDef status = check_unique_ids(reports, n);

    if (status != STAT_OK)
    {
        errors(status);
        free(reports);
        return 1;
    }

    /*
     * Сортируем по времени окончания.
     */
    qsort(reports,
          (size_t)n,
          sizeof(Report),
          compare_reports);

    int answer = 0;

    status = max_reports(reports, n, k, &answer);

    if (status != STAT_OK)
    {
        errors(status);
        free(reports);
        return 1;
    }

    printf("%d\n", answer);

    free(reports);

    return 0;
}