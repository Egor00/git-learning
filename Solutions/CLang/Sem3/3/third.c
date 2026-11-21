#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "myerrors.h"

#define BYTE_COUNT 256


typedef struct HuffmanNode
{
    unsigned char byte;
    unsigned long long frequency;

    struct HuffmanNode *left;
    struct HuffmanNode *right;
} HuffmanNode;


typedef struct
{
    HuffmanNode **data;
    size_t size;
    size_t capacity;
} MinHeap;


/*
 * Проверка, является ли узел листом.
 */
static int is_leaf(const HuffmanNode *node)
{
    return node != NULL &&
           node->left == NULL &&
           node->right == NULL;
}


/*
 * Создание узла дерева.
 */
static ErrorsDef create_node(unsigned char byte,
                             unsigned long long frequency,
                             HuffmanNode *left,
                             HuffmanNode *right,
                             HuffmanNode **result)
{
    if (result == NULL)
        return INVALID_ARGUMENT;

    HuffmanNode *node = malloc(sizeof(HuffmanNode));

    if (node == NULL)
        return MEMORY_ERROR;

    node->byte = byte;
    node->frequency = frequency;
    node->left = left;
    node->right = right;

    *result = node;

    return STAT_OK;
}


/*
 * Освобождение дерева Хаффмана.
 */
static void free_tree(HuffmanNode *root)
{
    if (root == NULL)
        return;

    free_tree(root->left);
    free_tree(root->right);

    free(root);
}


/*
 * Инициализация минимальной кучи.
 */
static ErrorsDef heap_init(MinHeap *heap, size_t capacity)
{
    if (heap == NULL)
        return INVALID_ARGUMENT;

    if (capacity == 0)
        return INVALID_ARGUMENT;

    if (capacity > SIZE_MAX / sizeof(HuffmanNode *))
        return MEMORY_ERROR;

    heap->data = malloc(capacity * sizeof(HuffmanNode *));

    if (heap->data == NULL)
        return MEMORY_ERROR;

    heap->size = 0;
    heap->capacity = capacity;

    return STAT_OK;
}


/*
 * Освобождение памяти кучи.
 *
 * Узлы дерева здесь НЕ освобождаются.
 */
static void heap_destroy(MinHeap *heap)
{
    if (heap == NULL)
        return;

    free(heap->data);

    heap->data = NULL;
    heap->size = 0;
    heap->capacity = 0;
}


/*
 * Сравнение узлов для минимальной кучи.
 *
 * При одинаковых частотах порядок произвольный.
 */
static int node_less(const HuffmanNode *a,
                     const HuffmanNode *b)
{
    if (a->frequency < b->frequency)
        return 1;

    if (a->frequency > b->frequency)
        return 0;

    return a->byte < b->byte;
}


/*
 * Обмен двух элементов кучи.
 */
static void heap_swap(HuffmanNode **a,
                      HuffmanNode **b)
{
    HuffmanNode *tmp = *a;
    *a = *b;
    *b = tmp;
}


/*
 * Восстановление свойства кучи вверх.
 */
static void heap_sift_up(MinHeap *heap, size_t index)
{
    while (index > 0)
    {
        size_t parent = (index - 1) / 2;

        if (!node_less(heap->data[index],
                       heap->data[parent]))
        {
            break;
        }

        heap_swap(&heap->data[index],
                  &heap->data[parent]);

        index = parent;
    }
}


/*
 * Восстановление свойства кучи вниз.
 */
static void heap_sift_down(MinHeap *heap, size_t index)
{
    while (1)
    {
        size_t left = index * 2 + 1;
        size_t right = index * 2 + 2;
        size_t smallest = index;

        if (left < heap->size &&
            node_less(heap->data[left],
                      heap->data[smallest]))
        {
            smallest = left;
        }

        if (right < heap->size &&
            node_less(heap->data[right],
                      heap->data[smallest]))
        {
            smallest = right;
        }

        if (smallest == index)
            break;

        heap_swap(&heap->data[index],
                  &heap->data[smallest]);

        index = smallest;
    }
}


/*
 * Добавление элемента в кучу.
 */
static ErrorsDef heap_push(MinHeap *heap,
                           HuffmanNode *node)
{
    if (heap == NULL || node == NULL)
        return INVALID_ARGUMENT;

    if (heap->size >= heap->capacity)
        return ARRAY_BOUNDS_ERROR;

    heap->data[heap->size] = node;

    heap_sift_up(heap, heap->size);

    ++heap->size;

    return STAT_OK;
}


/*
 * Извлечение минимального элемента.
 */
static ErrorsDef heap_pop(MinHeap *heap,
                          HuffmanNode **result)
{
    if (heap == NULL || result == NULL)
        return INVALID_ARGUMENT;

    if (heap->size == 0)
        return EMPTY_ARRAY;

    *result = heap->data[0];

    --heap->size;

    if (heap->size > 0)
    {
        heap->data[0] = heap->data[heap->size];

        heap_sift_down(heap, 0);
    }

    return STAT_OK;
}


/*
 * Подсчёт частот байтов.
 */
static ErrorsDef count_frequencies(FILE *file,
                                   unsigned long long frequencies[BYTE_COUNT],
                                   unsigned long long *file_size)
{
    if (file == NULL ||
        frequencies == NULL ||
        file_size == NULL)
    {
        return INVALID_ARGUMENT;
    }

    *file_size = 0;

    int c;

    while ((c = fgetc(file)) != EOF)
    {
        unsigned char byte = (unsigned char)c;

        /*
         * Проверяем переполнение счётчика размера.
         */
        if (*file_size == ULLONG_MAX)
            return FUNC_OVERFLOW;

        ++frequencies[byte];
        ++(*file_size);
    }

    if (ferror(file))
        return READFILE_ERROR;

    return STAT_OK;
}


/*
 * Построение дерева Хаффмана.
 */
static ErrorsDef build_tree(
    const unsigned long long frequencies[BYTE_COUNT],
    HuffmanNode **root)
{
    if (frequencies == NULL || root == NULL)
        return INVALID_ARGUMENT;

    *root = NULL;

    size_t different = 0;

    for (int i = 0; i < BYTE_COUNT; ++i)
    {
        if (frequencies[i] != 0)
            ++different;
    }

    /*
     * Пустой файл.
     */
    if (different == 0)
        return STAT_OK;

    MinHeap heap;

    ErrorsDef status = heap_init(&heap, BYTE_COUNT);

    if (status != STAT_OK)
        return status;

    /*
     * Создаём листья для всех встречающихся байтов.
     */
    for (int i = 0; i < BYTE_COUNT; ++i)
    {
        if (frequencies[i] != 0)
        {
            HuffmanNode *node = NULL;

            status = create_node(
                (unsigned char)i,
                frequencies[i],
                NULL,
                NULL,
                &node
            );

            if (status != STAT_OK)
            {
                heap_destroy(&heap);
                free_tree(*root);
                *root = NULL;

                /*
                 * Освобождаем уже созданные узлы.
                 */
                for (size_t j = 0; j < heap.size; ++j)
                {
                    free(heap.data[j]);
                }

                heap_destroy(&heap);

                return status;
            }

            status = heap_push(&heap, node);

            if (status != STAT_OK)
            {
                free(node);

                for (size_t j = 0; j < heap.size; ++j)
                    free(heap.data[j]);

                heap_destroy(&heap);

                return status;
            }
        }
    }

    /*
     * Особый случай:
     * файл содержит только один различный байт.
     */
    if (heap.size == 1)
    {
        status = heap_pop(&heap, root);

        heap_destroy(&heap);

        return status;
    }

    /*
     * Объединяем два минимальных узла,
     * пока не останется корень.
     */
    while (heap.size > 1)
    {
        HuffmanNode *left = NULL;
        HuffmanNode *right = NULL;

        status = heap_pop(&heap, &left);

        if (status != STAT_OK)
        {
            heap_destroy(&heap);
            return status;
        }

        status = heap_pop(&heap, &right);

        if (status != STAT_OK)
        {
            free(left);

            for (size_t j = 0; j < heap.size; ++j)
                free_tree(heap.data[j]);

            heap_destroy(&heap);

            return status;
        }

        if (ULLONG_MAX - left->frequency < right->frequency)
        {
            free_tree(left);
            free_tree(right);

            for (size_t j = 0; j < heap.size; ++j)
                free_tree(heap.data[j]);

            heap_destroy(&heap);

            return FUNC_OVERFLOW;
        }

        HuffmanNode *parent = NULL;

        status = create_node(
            0,
            left->frequency + right->frequency,
            left,
            right,
            &parent
        );

        if (status != STAT_OK)
        {
            free_tree(left);
            free_tree(right);

            for (size_t j = 0; j < heap.size; ++j)
                free_tree(heap.data[j]);

            heap_destroy(&heap);

            return status;
        }

        status = heap_push(&heap, parent);

        if (status != STAT_OK)
        {
            free_tree(parent);

            for (size_t j = 0; j < heap.size; ++j)
                free_tree(heap.data[j]);

            heap_destroy(&heap);

            return status;
        }
    }

    status = heap_pop(&heap, root);

    heap_destroy(&heap);

    return status;
}


/*
 * Заполнение кодов Хаффмана.
 *
 * codes[byte] будет содержать строку из 0 и 1.
 */
static ErrorsDef generate_codes(
    const HuffmanNode *root,
    char *codes[BYTE_COUNT])
{
    if (root == NULL || codes == NULL)
        return INVALID_ARGUMENT;

    /*
     * Максимальная длина кода для дерева из 256 листьев
     * не превышает 255.
     */
    char buffer[BYTE_COUNT + 1];

    /*
     * Стек для обхода дерева без рекурсивной функции
     * генерации кодов.
     */
    typedef struct
    {
        const HuffmanNode *node;
        int position;
    } StackItem;

    StackItem stack[BYTE_COUNT * 2];

    size_t stack_size = 0;

    stack[stack_size].node = root;
    stack[stack_size].position = 0;
    ++stack_size;

    while (stack_size > 0)
    {
        --stack_size;

        const HuffmanNode *node = stack[stack_size].node;
        int position = stack[stack_size].position;

        if (is_leaf(node))
        {
            /*
             * Единственный байт в файле получает код "0".
             */
            if (position == 0)
            {
                buffer[0] = '0';
                buffer[1] = '\0';
            }
            else
            {
                buffer[position] = '\0';
            }

            codes[node->byte] = malloc(
                (strlen(buffer) + 1) * sizeof(char)
            );

            if (codes[node->byte] == NULL)
                return MEMORY_ERROR;

            strcpy(codes[node->byte], buffer);

            continue;
        }

        /*
         * Сначала кладём правого ребёнка,
         * затем левого, чтобы левый обрабатывался первым.
         */
        if (node->right != NULL)
        {
            buffer[position] = '1';

            stack[stack_size].node = node->right;
            stack[stack_size].position = position + 1;
            ++stack_size;
        }

        if (node->left != NULL)
        {
            buffer[position] = '0';

            stack[stack_size].node = node->left;
            stack[stack_size].position = position + 1;
            ++stack_size;
        }
    }

    return STAT_OK;
}


/*
 * Освобождение массива кодов.
 */
static void free_codes(char *codes[BYTE_COUNT])
{
    if (codes == NULL)
        return;

    for (int i = 0; i < BYTE_COUNT; ++i)
    {
        free(codes[i]);
        codes[i] = NULL;
    }
}


/*
 * Вычисление количества бит закодированных данных.
 */
static ErrorsDef calculate_bits(
    const unsigned long long frequencies[BYTE_COUNT],
    char *codes[BYTE_COUNT],
    unsigned long long *bits)
{
    if (frequencies == NULL ||
        codes == NULL ||
        bits == NULL)
    {
        return INVALID_ARGUMENT;
    }

    *bits = 0;

    for (int i = 0; i < BYTE_COUNT; ++i)
    {
        if (frequencies[i] == 0)
            continue;

        if (codes[i] == NULL)
            return CONVERSION_ERROR;

        unsigned long long code_length =
            (unsigned long long)strlen(codes[i]);

        if (code_length != 0 &&
            frequencies[i] > ULLONG_MAX / code_length)
        {
            return FUNC_OVERFLOW;
        }

        unsigned long long contribution =
            frequencies[i] * code_length;

        if (ULLONG_MAX - *bits < contribution)
            return FUNC_OVERFLOW;

        *bits += contribution;
    }

    return STAT_OK;
}


/*
 * Округление количества бит вверх до байтов.
 */
static ErrorsDef bits_to_bytes(
    unsigned long long bits,
    unsigned long long *bytes)
{
    if (bytes == NULL)
        return INVALID_ARGUMENT;

    if (bits == 0)
    {
        *bytes = 0;
        return STAT_OK;
    }

    *bytes = (bits - 1) / 8 + 1;

    return STAT_OK;
}


/*
 * Вывод таблицы кодов.
 */
static void print_codes(
    const unsigned long long frequencies[BYTE_COUNT],
    char *codes[BYTE_COUNT])
{
    printf("Byte\tFrequency\tCode\n");

    for (int i = 0; i < BYTE_COUNT; ++i)
    {
        if (frequencies[i] != 0)
        {
            printf("%d\t%llu\t\t%s\n",
                   i,
                   frequencies[i],
                   codes[i]);
        }
    }
}


int main(int argc, char *argv[])
{
    /*
     * По условию программе передаётся путь к файлу
     * через аргумент командной строки.
     */
    if (argc != 2)
    {
        errors(MATCH_ARGS);
        return 1;
    }

    if (argv[1] == NULL || argv[1][0] == '\0')
    {
        errors(FILENAME_ERROR);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");

    if (file == NULL)
    {
        errors(OPENFILE_ERROR);
        return 1;
    }

    unsigned long long frequencies[BYTE_COUNT] = {0};
    unsigned long long file_size = 0;

    ErrorsDef status =
        count_frequencies(file,
                          frequencies,
                          &file_size);

    if (status != STAT_OK)
    {
        errors(status);

        if (fclose(file) != 0)
            errors(CLOSEFILE_ERROR);

        return 1;
    }

    /*
     * После чтения закрываем исходный файл.
     */
    if (fclose(file) != 0)
    {
        errors(CLOSEFILE_ERROR);
        return 1;
    }

    /*
     * Для пустого файла всё должно быть равно нулю.
     */
    if (file_size == 0)
    {
        printf("Huffman codes:\n");
        printf("\n");

        printf("Original file size: 0 bytes\n");
        printf("Encoded data size: 0 bits\n");
        printf("Encoded data size: 0 bytes\n");
        printf("Compression saving: 0.00%%\n");

        return 0;
    }

    HuffmanNode *root = NULL;

    status = build_tree(frequencies, &root);

    if (status != STAT_OK)
    {
        errors(status);
        free_tree(root);
        return 1;
    }

    char *codes[BYTE_COUNT] = {0};

    status = generate_codes(root, codes);

    if (status != STAT_OK)
    {
        errors(status);

        free_codes(codes);
        free_tree(root);

        return 1;
    }

    unsigned long long encoded_bits = 0;

    status = calculate_bits(
        frequencies,
        codes,
        &encoded_bits
    );

    if (status != STAT_OK)
    {
        errors(status);

        free_codes(codes);
        free_tree(root);

        return 1;
    }

    unsigned long long encoded_bytes = 0;

    status = bits_to_bytes(
        encoded_bits,
        &encoded_bytes
    );

    if (status != STAT_OK)
    {
        errors(status);

        free_codes(codes);
        free_tree(root);

        return 1;
    }

    /*
     * Формула:
     *
     * 100 * (1 - M / N)
     *
     * где N — исходный размер в байтах,
     * M — размер закодированных данных в байтах.
     */
    double saving =
        100.0 *
        (1.0 -
         (double)encoded_bytes /
         (double)file_size);

    print_codes(frequencies, codes);

    printf("\n");
    printf("Original file size: %llu bytes\n",
           file_size);

    printf("Encoded data size: %llu bits\n",
           encoded_bits);

    printf("Encoded data size: %llu bytes\n",
           encoded_bytes);

    printf("Compression saving: %.2f%%\n",
           saving);

    free_codes(codes);
    free_tree(root);

    return 0;
}