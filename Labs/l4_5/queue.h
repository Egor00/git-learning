#ifndef _UDT_H_
#define _UDT_H_

#include <stdbool.h>
#include <stddef.h>

typedef int key_type;
typedef int value_type;

typedef struct {
    key_type   key;
    value_type value;
} data_type;

#define QUEUE_CAPACITY 256

typedef struct {
    data_type buf[QUEUE_CAPACITY];
    int       head;
    int       size;
} udt;

void    udt_create    (udt *q);
bool    udt_is_empty  (const udt *q);
void    udt_push_front(udt *q, data_type d);
void    udt_push_back (udt *q, data_type d);
void    udt_pop_front (udt *q);
void    udt_pop_back  (udt *q);
void    udt_print     (const udt *q);
size_t  udt_size      (const udt *q);
void    udt_insert    (udt *q, const data_type d);
void    udt_erase     (udt *q, const key_type k);


data_type udt_at    (const udt *q, int i);
void      udt_set_at(udt *q, int i, data_type d);
#endif
