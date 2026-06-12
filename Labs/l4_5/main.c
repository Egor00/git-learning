#include <stdio.h>
#include "queue.h"

bool queue_swap_first_inversion(udt *q, int from, int to_excl);
void queue_sort_bubble(udt *q);

static void fill_queue(udt *q, const int keys[], int n) {
    udt_create(q);
    for (int i = 0; i < n; i++) {
        data_type d = { keys[i], keys[i] * 100 };
        udt_push_back(q, d);
    }
}

int main(void) {
    udt q;

    printf("Вспомогательная процедура: перестановка\n");
    int data1[] = {3, 1, 4, 1, 5, 9, 2, 6};
    fill_queue(&q, data1, 8);

    printf("Исходная очередь:   "); udt_print(&q);

    while (queue_swap_first_inversion(&q, 0, (int)udt_size(&q)));
    udt_print(&q);

    printf("Пузырьковая сортировка\n");
    int data2[] = {64, 34, 25, 12, 22, 11, 90};
    fill_queue(&q, data2, 7);

    printf("До сортировки:  "); udt_print(&q);
    queue_sort_bubble(&q);
    printf("После сортировки: "); udt_print(&q);
    printf("\n");

    printf("Уже отсортированная очередь\n");
    int data3[] = {1, 2, 3, 4, 5};
    fill_queue(&q, data3, 5);
    printf("До сортировки:  "); udt_print(&q);
    queue_sort_bubble(&q);
    printf("После сортировки: "); udt_print(&q);
    printf("\n");

    printf("Очередь из одного элемента\n");
    int data4[] = {42};
    fill_queue(&q, data4, 1);
    printf("До сортировки:  "); udt_print(&q);
    queue_sort_bubble(&q);
    printf("После сортировки: "); udt_print(&q);

    return 0;
}
