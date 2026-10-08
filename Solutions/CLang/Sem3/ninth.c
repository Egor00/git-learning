#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define FIXED_SIZE 20

/*
    Коды возврата:
    0 - успех
    1 - неверные параметры
    2 - ошибка выделения памяти
*/

/* Генерация случайного целого числа в диапазоне [a, b] */
int random_int(int a, int b, int *result)
{
    long long range;
    double value;

    if (result == NULL || a > b)
        return 1;

    range = (long long)b - (long long)a + 1;

    /*
        Используем double, чтобы функция могла работать
        с достаточно широким диапазоном int.
    */
    value = (double)rand() / ((double)RAND_MAX + 1.0);

    *result = a + (int)(value * range);

    return 0;
}

/*
    Заполнение массива случайными числами из [a, b].
*/
int fill_array(int *array, size_t size, int a, int b)
{
    size_t i;

    if (array == NULL || size == 0 || a > b)
        return 1;

    for (i = 0; i < size; ++i)
    {
        if (random_int(a, b, &array[i]) != 0)
            return 1;
    }

    return 0;
}

/*
    Поиск минимального и максимального элементов
    и обмен их местами за один проход.

    min_value и max_value получают найденные значения.
*/
int find_min_max_and_swap(
    int *array,
    size_t size,
    int *min_value,
    int *max_value
)
{
    size_t i;
    size_t min_index;
    size_t max_index;
    int min;
    int max;
    int temp;

    if (array == NULL || size == 0 ||
        min_value == NULL || max_value == NULL)
    {
        return 1;
    }

    min = array[0];
    max = array[0];

    min_index = 0;
    max_index = 0;

    /*
        Один проход по массиву.
    */
    for (i = 1; i < size; ++i)
    {
        if (array[i] < min)
        {
            min = array[i];
            min_index = i;
        }

        if (array[i] > max)
        {
            max = array[i];
            max_index = i;
        }
    }

    /*
        Если минимум и максимум находятся
        на разных позициях, меняем их местами.
    */
    if (min_index != max_index)
    {
        temp = array[min_index];
        array[min_index] = array[max_index];
        array[max_index] = temp;
    }

    *min_value = min;
    *max_value = max;

    return 0;
}

/*
    Печать массива.
*/
int print_array(const int *array, size_t size)
{
    size_t i;

    if (array == NULL || size == 0)
        return 1;

    for (i = 0; i < size; ++i)
    {
        printf("%d", array[i]);

        if (i + 1 < size)
            printf(" ");
    }

    printf("\n");

    return 0;
}

/*
    Поиск элемента массива B, ближайшего по значению к value.

    Если несколько элементов имеют одинаковое минимальное
    расстояние, выбирается первый найденный.
*/
int find_nearest(
    const int *array,
    size_t size,
    int value,
    int *nearest
)
{
    size_t i;
    long long current_difference;
    long long best_difference;

    if (array == NULL || size == 0 || nearest == NULL)
        return 1;

    *nearest = array[0];

    best_difference =
        llabs((long long)array[0] - (long long)value);

    for (i = 1; i < size; ++i)
    {
        current_difference =
            llabs((long long)array[i] - (long long)value);

        if (current_difference < best_difference)
        {
            best_difference = current_difference;
            *nearest = array[i];
        }
    }

    return 0;
}

/*
    Формирование массива C:

        C[i] = A[i] + ближайший по значению элемент B.
*/
int create_array_c(
    const int *a,
    size_t size_a,
    const int *b,
    size_t size_b,
    int **c
)
{
    size_t i;
    int nearest;
    long long sum;
    int *result;

    if (a == NULL || b == NULL ||
        size_a == 0 || size_b == 0 ||
        c == NULL)
    {
        return 1;
    }

    result = (int *)malloc(size_a * sizeof(int));

    if (result == NULL)
        return 2;

    for (i = 0; i < size_a; ++i)
    {
        if (find_nearest(b, size_b, a[i], &nearest) != 0)
        {
            free(result);
            return 1;
        }

        sum = (long long)a[i] + (long long)nearest;

        /*
            В данном задании значения A и B лежат
            в [-1000; 1000], поэтому переполнения int
            здесь не возникает. Проверка оставлена
            для корректной работы функции вообще.
        */
        if (sum < INT_MIN || sum > INT_MAX)
        {
            free(result);
            return 1;
        }

        result[i] = (int)sum;
    }

    *c = result;

    return 0;
}

/*
    Получение случайного размера массива
    в диапазоне [10; 10000].
*/
int random_size(size_t *size)
{
    int value;

    if (size == NULL)
        return 1;

    if (random_int(10, 10000, &value) != 0)
        return 1;

    *size = (size_t)value;

    return 0;
}

/*
    Первая часть N9.
*/
int task_part_one(int a, int b)
{
    int array[FIXED_SIZE];
    int min_value;
    int max_value;

    printf("=== Часть 1 ===\n");

    if (fill_array(array, FIXED_SIZE, a, b) != 0)
    {
        fprintf(stderr, "Ошибка заполнения массива.\n");
        return 1;
    }

    printf("Исходный массив:\n");
    print_array(array, FIXED_SIZE);

    if (find_min_max_and_swap(
            array,
            FIXED_SIZE,
            &min_value,
            &max_value) != 0)
    {
        fprintf(stderr, "Ошибка обработки массива.\n");
        return 1;
    }

    printf("Минимальный элемент: %d\n", min_value);
    printf("Максимальный элемент: %d\n", max_value);

    printf("Массив после обмена минимума и максимума:\n");
    print_array(array, FIXED_SIZE);

    return 0;
}

/*
    Вторая часть N9.
*/
int task_part_two(void)
{
    int *a = NULL;
    int *b = NULL;
    int *c = NULL;

    size_t size_a;
    size_t size_b;

    int result;

    printf("\n=== Часть 2 ===\n");

    /*
        Случайные размеры A и B в [10; 10000].
    */
    if (random_size(&size_a) != 0 ||
        random_size(&size_b) != 0)
    {
        fprintf(stderr, "Ошибка генерации размеров массивов.\n");
        return 1;
    }

    printf("Размер A: %zu\n", size_a);
    printf("Размер B: %zu\n", size_b);

    a = (int *)malloc(size_a * sizeof(int));

    if (a == NULL)
    {
        fprintf(stderr, "Не удалось выделить память для A.\n");
        return 2;
    }

    b = (int *)malloc(size_b * sizeof(int));

    if (b == NULL)
    {
        fprintf(stderr, "Не удалось выделить память для B.\n");
        free(a);
        return 2;
    }

    /*
        A и B заполняются числами из [-1000; 1000].
    */
    if (fill_array(a, size_a, -1000, 1000) != 0)
    {
        fprintf(stderr, "Ошибка заполнения A.\n");
        free(a);
        free(b);
        return 1;
    }

    if (fill_array(b, size_b, -1000, 1000) != 0)
    {
        fprintf(stderr, "Ошибка заполнения B.\n");
        free(a);
        free(b);
        return 1;
    }

    /*
        Формируем C.
        Размер C равен размеру A.
    */
    result = create_array_c(
        a,
        size_a,
        b,
        size_b,
        &c
    );

    if (result != 0)
    {
        fprintf(stderr, "Ошибка формирования массива C.\n");
        free(a);
        free(b);
        return result;
    }

    /*
        Чтобы не выводить до 10000 элементов,
        показываем первые 20.
    */
    printf("\nПервые элементы A:\n");
    print_array(a, size_a < 20 ? size_a : 20);

    printf("Первые элементы B:\n");
    print_array(b, size_b < 20 ? size_b : 20);

    printf("Первые элементы C:\n");
    print_array(c, size_a < 20 ? size_a : 20);

    /*
        Освобождение всей динамической памяти.
    */
    free(a);
    free(b);
    free(c);

    return 0;
}

int main(int argc, char *argv[])
{
    char *end_ptr;
    long a_long;
    long b_long;
    int a;
    int b;
    int result;

    srand((unsigned int)time(NULL));

    /*
        Для первой части a и b передаются
        через командную строку:

        program a b

        Например:

        program -100 100
    */
    if (argc != 3)
    {
        fprintf(
            stderr,
            "Использование: %s <a> <b>\n",
            argv[0]
        );

        return 1;
    }

    /*
        Чтение a.
    */
    end_ptr = NULL;
    a_long = strtol(argv[1], &end_ptr, 10);

    if (*argv[1] == '\0' ||
        *end_ptr != '\0' ||
        a_long < INT_MIN ||
        a_long > INT_MAX)
    {
        fprintf(stderr, "Некорректное значение a.\n");
        return 1;
    }

    /*
        Чтение b.
    */
    end_ptr = NULL;
    b_long = strtol(argv[2], &end_ptr, 10);

    if (*argv[2] == '\0' ||
        *end_ptr != '\0' ||
        b_long < INT_MIN ||
        b_long > INT_MAX)
    {
        fprintf(stderr, "Некорректное значение b.\n");
        return 1;
    }

    a = (int)a_long;
    b = (int)b_long;

    if (a > b)
    {
        fprintf(stderr, "Должно выполняться условие a <= b.\n");
        return 1;
    }

    /*
        Часть 1.
    */
    result = task_part_one(a, b);

    if (result != 0)
        return result;

    /*
        Часть 2.
    */
    result = task_part_two();

    return result;
}