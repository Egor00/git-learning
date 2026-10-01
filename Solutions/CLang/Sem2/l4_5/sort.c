#include <stdio.h>
#include <stdbool.h>
#include "queue.h"

bool queue_swap_first_inversion(udt *q, int from, int to_excl) {
    for (int i = from; i < to_excl - 1; i++) {
        data_type a = udt_at(q, i);
        data_type b = udt_at(q, i + 1);

        if (a.key > b.key) {
            udt_set_at(q, i,     b);
            udt_set_at(q, i + 1, a);
            return true;
        }
    }
    return false;
}

void queue_sort_bubble(udt *q) {
    int n = (int)udt_size(q);
    if (n < 2) return;

    for (int pass = 0; pass < n - 1; pass++) {
        bool any_swap = false;
        for (int i = 0; i < n - 1 - pass; i++) {
            data_type a = udt_at(q, i);
            data_type b = udt_at(q, i + 1);

            if (a.key > b.key) {
                udt_set_at(q, i,     b);
                udt_set_at(q, i + 1, a);
                any_swap = true;
            }
        }
        if (!any_swap) break;
    }
}
