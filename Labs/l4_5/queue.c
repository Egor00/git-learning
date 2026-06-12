#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

static int phys(const udt *q, int i) {
    return (q->head + i) % QUEUE_CAPACITY;
}

void udt_create(udt *q) {
    q->head = 0;
    q->size = 0;
}


bool udt_is_empty(const udt *q) {
    return q->size == 0;
}

void udt_push_back(udt *q, data_type d) {
    if (q->size >= QUEUE_CAPACITY) {
        fputs("udt_push_back: переполнение очереди\n", stderr);
        return;
    }
    q->buf[phys(q, q->size)] = d;
    q->size++;
}

void udt_push_front(udt *q, data_type d) {
    if (q->size >= QUEUE_CAPACITY) {
        fputs("udt_push_front: переполнение очереди\n", stderr);
        return;
    }
    q->head = (q->head - 1 + QUEUE_CAPACITY) % QUEUE_CAPACITY;
    q->buf[q->head] = d;
    q->size++;
}

void udt_insert(udt *q, const data_type d) {
    udt_push_back(q, d);
}

void udt_pop_front(udt *q) {
    if (udt_is_empty(q)) {
        fputs("udt_pop_front: очередь пуста\n", stderr);
        return;
    }
    q->head = (q->head + 1) % QUEUE_CAPACITY;
    q->size--;
}

void udt_pop_back(udt *q) {
    if (udt_is_empty(q)) {
        fputs("udt_pop_back: очередь пуста\n", stderr);
        return;
    }
    q->size--;
}

void udt_erase(udt *q, const key_type k) {
    for (int i = 0; i < q->size; i++) {
        if (q->buf[phys(q, i)].key == k) {
            for (int j = i; j < q->size - 1; j++)
                q->buf[phys(q, j)] = q->buf[phys(q, j + 1)];
            q->size--;
            return;
        }
    }
}

size_t udt_size(const udt *q) {
    return (size_t)q->size;
}

void udt_print(const udt *q) {
    printf("Очередь [%zu эл.]: ", udt_size(q));
    for (int i = 0; i < q->size; i++)
        printf("(%d|%d) ", q->buf[phys(q, i)].key,
                           q->buf[phys(q, i)].value);
    putchar('\n');
}

data_type udt_at(const udt *q, int i) {
    return q->buf[phys(q, i)];
}

void udt_set_at(udt *q, int i, data_type d) {
    q->buf[phys(q, i)] = d;
}
