
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>


#include "myerrors.h"

/* ---------- String ---------- */

typedef struct {
    char *data;
    int length;
} String;

int string_init(String *s, const char *src) {
    if (s == NULL || src == NULL)
        return 0;

    size_t len = strlen(src);
    if (len > 2147483647)
        return 0;

    char *p = malloc(len + 1);
    if (p == NULL)
        return 0;

    memcpy(p, src, len + 1);

    s->data = p;
    s->length = (int)len;
    return 1;
}

void string_free(String *s) {
    if (s == NULL)
        return;

    free(s->data);
    s->data = NULL;
    s->length = 0;
}

int string_copy(String *dst, const String *src) {
    if (dst == NULL || src == NULL || src->data == NULL)
        return 0;

    String tmp = {NULL, 0};

    if (!string_init(&tmp, src->data))
        return 0;

    string_free(dst);
    *dst = tmp;
    return 1;
}

int string_compare(const String *a, const String *b) {
    if (a->length != b->length)
        return a->length < b->length ? -1 : 1;

    return strcmp(a->data, b->data);
}

int string_equal(const String *a, const String *b) {
    return strcmp(a->data, b->data) == 0;
}

/* ---------- Structures ---------- */

typedef struct {
    String city;
    String street;
    unsigned int house;
    String building;
    unsigned int apartment;
    String index;
} Address;

typedef struct {
    Address recipient;
    float weight;
    String id;
    String creation_time;
    String delivery_time;
    int delivered;
} Mail;

typedef struct {
    Address *office;
    Mail *mails;
    size_t count;
    size_t capacity;
} Post;

/* ---------- Input ---------- */

static int read_line(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);

    if (fgets(buffer, (int)size, stdin) == NULL)
        return 0;

    size_t len = strlen(buffer);

    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
        return 0;
    }

    return 1;
}

static int read_nonempty(const char *prompt, char *buffer, size_t size) {
    do {
        if (!read_line(prompt, buffer, size))
            return 0;

        if (buffer[0] == '\0')
            puts("Строка не должна быть пустой.");
    } while (buffer[0] == '\0');

    return 1;
}

static int read_uint(const char *prompt, unsigned int *value) {
    char buffer[128];
    char *end;
    unsigned long n;

    if (!read_nonempty(prompt, buffer, sizeof(buffer)))
        return 0;

    errno = 0;
    n = strtoul(buffer, &end, 10);

    if (errno || *end != '\0' || n == 0 || n > 4294967295UL) {
        puts("Введите натуральное число.");
        return 0;
    }

    *value = (unsigned int)n;
    return 1;
}

static int read_float_value(const char *prompt, float *value) {
    char buffer[128];
    char *end;

    if (!read_nonempty(prompt, buffer, sizeof(buffer)))
        return 0;

    errno = 0;
    float x = strtof(buffer, &end);

    if (errno || *end != '\0' || x < 0.0f) {
        puts("Введите неотрицательное число.");
        return 0;
    }

    *value = x;
    return 1;
}

static int is_digits(const char *s, size_t expected) {
    if (strlen(s) != expected)
        return 0;

    for (size_t i = 0; i < expected; ++i) {
        if (!isdigit((unsigned char)s[i]))
            return 0;
    }

    return 1;
}

/* ---------- Date and time ---------- */

/*
 * Формат: dd:MM:yyyy hh:mm:ss
 * Преобразование выполняется в локальное системное время.
 */

static int parse_datetime(const char *s, time_t *result) {
    int day, month, year, hour, minute, second;
    char extra;

    if (sscanf(s, "%2d:%2d:%4d %2d:%2d:%2d%c",
               &day, &month, &year,
               &hour, &minute, &second, &extra) != 6)
        return 0;

    if (day < 1 || day > 31 ||
        month < 1 || month > 12 ||
        year < 1900 ||
        hour < 0 || hour > 23 ||
        minute < 0 || minute > 59 ||
        second < 0 || second > 59)
        return 0;

    struct tm tm_value = {0};

    tm_value.tm_mday = day;
    tm_value.tm_mon = month - 1;
    tm_value.tm_year = year - 1900;
    tm_value.tm_hour = hour;
    tm_value.tm_min = minute;
    tm_value.tm_sec = second;
    tm_value.tm_isdst = -1;

    time_t t = mktime(&tm_value);

    if (t == (time_t)-1)
        return 0;

    /* Проверяем, что дата действительно существует. */
    struct tm check;
    if (localtime_r(&t, &check) == NULL)
        return 0;

    if (check.tm_mday != day ||
        check.tm_mon != month - 1 ||
        check.tm_year != year - 1900 ||
        check.tm_hour != hour ||
        check.tm_min != minute ||
        check.tm_sec != second)
        return 0;

    *result = t;
    return 1;
}

static int read_datetime(const char *prompt, String *result) {
    char buffer[64];
    time_t ignored;

    if (!read_nonempty(prompt, buffer, sizeof(buffer)))
        return 0;

    if (!parse_datetime(buffer, &ignored)) {
        puts("Некорректная дата. Формат: dd:MM:yyyy hh:mm:ss");
        return 0;
    }

    return string_init(result, buffer);
}

/* ---------- Address ---------- */

static void address_free(Address *a) {
    if (a == NULL)
        return;

    string_free(&a->city);
    string_free(&a->street);
    string_free(&a->building);
    string_free(&a->index);
}

static int address_read(Address *a) {
    char buffer[256];

    if (!read_nonempty("Город: ", buffer, sizeof(buffer)) ||
        !string_init(&a->city, buffer))
        return 0;

    if (!read_nonempty("Улица: ", buffer, sizeof(buffer)) ||
        !string_init(&a->street, buffer))
        return 0;

    if (!read_uint("Номер дома: ", &a->house))
        return 0;

    if (!read_line("Корпус (можно оставить пустым): ",
                   buffer, sizeof(buffer)) ||
        !string_init(&a->building, buffer))
        return 0;

    if (!read_uint("Номер квартиры: ", &a->apartment))
        return 0;

    do {
        if (!read_nonempty("Индекс (6 цифр): ",
                           buffer, sizeof(buffer)))
            return 0;

        if (!is_digits(buffer, 6))
            puts("Индекс должен содержать ровно 6 цифр.");
    } while (!is_digits(buffer, 6));

    if (!string_init(&a->index, buffer))
        return 0;

    return 1;
}

static int address_copy(Address *dst, const Address *src) {
    Address tmp = {0};

    if (!string_init(&tmp.city, src->city.data) ||
        !string_init(&tmp.street, src->street.data) ||
        !string_init(&tmp.building, src->building.data) ||
        !string_init(&tmp.index, src->index.data)) {
        address_free(&tmp);
        return 0;
    }

    tmp.house = src->house;
    tmp.apartment = src->apartment;

    address_free(dst);
    *dst = tmp;
    return 1;
}

static void address_print(const Address *a) {
    printf("Город: %s\n", a->city.data);
    printf("Улица: %s\n", a->street.data);
    printf("Дом: %u\n", a->house);
    printf("Корпус: %s\n", a->building.data);
    printf("Квартира: %u\n", a->apartment);
    printf("Индекс: %s\n", a->index.data);
}

/* ---------- Mail ---------- */

static void mail_free(Mail *m) {
    if (m == NULL)
        return;

    address_free(&m->recipient);
    string_free(&m->id);
    string_free(&m->creation_time);
    string_free(&m->delivery_time);
}

static int mail_read(Mail *m) {
    char buffer[128];

    if (!address_read(&m->recipient))
        return 0;

    if (!read_float_value("Вес посылки: ", &m->weight))
        return 0;

    do {
        if (!read_nonempty("Почтовый ID (14 цифр): ",
                           buffer, sizeof(buffer)))
            return 0;

        if (!is_digits(buffer, 14))
            puts("ID должен содержать ровно 14 цифр.");
    } while (!is_digits(buffer, 14));

    if (!string_init(&m->id, buffer))
        return 0;

    if (!read_datetime("Время создания: ", &m->creation_time))
        return 0;

    if (!read_datetime("Срок доставки: ", &m->delivery_time))
        return 0;

    char answer[16];

    do {
        if (!read_nonempty("Отправление уже доставлено? (1 - да, 0 - нет): ",
                           answer, sizeof(answer)))
            return 0;

        if (strcmp(answer, "0") != 0 && strcmp(answer, "1") != 0)
            puts("Введите 1 или 0.");
    } while (strcmp(answer, "0") != 0 && strcmp(answer, "1") != 0);

    m->delivered = (answer[0] == '1');

    return 1;
}

static void mail_print(const Mail *m) {
    puts("\n--- Информация об отправлении ---");

    printf("ID: %s\n", m->id.data);
    puts("Адрес получателя:");
    address_print(&m->recipient);

    printf("Вес: %.3f\n", m->weight);
    printf("Время создания: %s\n", m->creation_time.data);
    printf("Время вручения / срок доставки: %s\n",
           m->delivery_time.data);
    printf("Статус: %s\n",
           m->delivered ? "Доставлено" : "Не доставлено");

    puts("---------------------------------");
}

/* ---------- Post ---------- */

static int post_init(Post *p, const Address *office) {
    p->office = malloc(sizeof(Address));

    if (p->office == NULL)
        return 0;

    *p->office = (Address){0};

    if (!address_copy(p->office, office)) {
        free(p->office);
        p->office = NULL;
        return 0;
    }

    p->mails = NULL;
    p->count = 0;
    p->capacity = 0;

    return 1;
}

static void post_free(Post *p) {
    for (size_t i = 0; i < p->count; ++i)
        mail_free(&p->mails[i]);

    free(p->mails);
    p->mails = NULL;
    p->count = 0;
    p->capacity = 0;

    if (p->office != NULL) {
        address_free(p->office);
        free(p->office);
        p->office = NULL;
    }
}

static int post_has_id(const Post *p, const char *id) {
    for (size_t i = 0; i < p->count; ++i) {
        if (strcmp(p->mails[i].id.data, id) == 0)
            return 1;
    }

    return 0;
}

static int post_add(Post *p, const Mail *mail) {
    if (p->count == p->capacity) {
        size_t new_capacity = p->capacity == 0
                            ? 4
                            : p->capacity * 2;

        if (new_capacity < p->capacity ||
            new_capacity > (size_t)-1 / sizeof(Mail))
            return 0;

        Mail *tmp = realloc(p->mails,
                            new_capacity * sizeof(Mail));

        if (tmp == NULL)
            return 0;

        p->mails = tmp;
        p->capacity = new_capacity;
    }

    Mail copy = {0};

    if (!address_copy(&copy.recipient, &mail->recipient) ||
        !string_init(&copy.id, mail->id.data) ||
        !string_init(&copy.creation_time, mail->creation_time.data) ||
        !string_init(&copy.delivery_time, mail->delivery_time.data)) {
        mail_free(&copy);
        return 0;
    }

    copy.weight = mail->weight;
    copy.delivered = mail->delivered;

    p->mails[p->count] = copy;
    ++p->count;

    return 1;
}

static int post_delete(Post *p, const char *id) {
    size_t pos = p->count;

    for (size_t i = 0; i < p->count; ++i) {
        if (strcmp(p->mails[i].id.data, id) == 0) {
            pos = i;
            break;
        }
    }

    if (pos == p->count)
        return 0;

    mail_free(&p->mails[pos]);

    for (size_t i = pos; i + 1 < p->count; ++i)
        p->mails[i] = p->mails[i + 1];

    --p->count;
    return 1;
}

static Mail *post_find(Post *p, const char *id) {
    for (size_t i = 0; i < p->count; ++i) {
        if (strcmp(p->mails[i].id.data, id) == 0)
            return &p->mails[i];
    }

    return NULL;
}

/* ---------- Sorting ---------- */

static int compare_mail(const void *pa, const void *pb) {
    const Mail *a = pa;
    const Mail *b = pb;

    int cmp = strcmp(a->recipient.index.data,
                     b->recipient.index.data);

    if (cmp != 0)
        return cmp;

    return strcmp(a->id.data, b->id.data);
}

static int compare_creation_time(const void *pa, const void *pb) {
    const Mail *a = pa;
    const Mail *b = pb;

    time_t ta, tb;

    parse_datetime(a->creation_time.data, &ta);
    parse_datetime(b->creation_time.data, &tb);

    if (ta < tb) return -1;
    if (ta > tb) return 1;
    return 0;
}

static void post_sort(Post *p) {
    qsort(p->mails, p->count, sizeof(Mail), compare_mail);
}

/* ---------- Lists ---------- */

static void post_print_delivered(const Post *p) {
    Mail *list = malloc(p->count * sizeof(Mail));

    if (list == NULL && p->count != 0) {
        puts("Не удалось выделить память.");
        return;
    }

    size_t n = 0;

    for (size_t i = 0; i < p->count; ++i) {
        if (p->mails[i].delivered)
            list[n++] = p->mails[i];
    }

    qsort(list, n, sizeof(Mail), compare_creation_time);

    printf("\nДоставленные отправления: %zu\n", n);

    for (size_t i = 0; i < n; ++i)
        mail_print(&list[i]);

    free(list);
}

static void post_print_overdue(const Post *p) {
    time_t now = time(NULL);
    Mail *list = malloc(p->count * sizeof(Mail));

    if (list == NULL && p->count != 0) {
        puts("Не удалось выделить память.");
        return;
    }

    size_t n = 0;

    for (size_t i = 0; i < p->count; ++i) {
        time_t deadline;

        if (!p->mails[i].delivered &&
            parse_datetime(p->mails[i].delivery_time.data, &deadline) &&
            deadline < now) {
            list[n++] = p->mails[i];
        }
    }

    qsort(list, n, sizeof(Mail), compare_creation_time);

    printf("\nПросроченные отправления: %zu\n", n);

    for (size_t i = 0; i < n; ++i)
        mail_print(&list[i]);

    free(list);
}

/* ---------- Menu ---------- */

static void print_menu(void) {
    puts("\n========== ПОЧТА ==========");
    puts("1. Добавить отправление");
    puts("2. Удалить отправление");
    puts("3. Найти отправление по ID");
    puts("4. Отсортировать отправления");
    puts("5. Вывести доставленные отправления");
    puts("6. Вывести просроченные отправления");
    puts("7. Вывести все отправления");
    puts("0. Выход");
    puts("===========================");
}

int main(void) {
    Address office = {0};
    Post post = {0};

    puts("Введите адрес текущего почтового отделения:");

    if (!address_read(&office)) {
        puts("Ошибка ввода адреса.");
        address_free(&office);
        return 1;
    }

    if (!post_init(&post, &office)) {
        puts("Не удалось создать почтовое отделение.");
        address_free(&office);
        return 1;
    }

    address_free(&office);

    int running = 1;

    while (running) {
        char choice[32];
        print_menu();

        if (!read_nonempty("Выберите действие: ",
                           choice, sizeof(choice)))
            continue;

        if (strcmp(choice, "1") == 0) {
            Mail mail = {0};

            if (!mail_read(&mail)) {
                puts("Не удалось прочитать отправление.");
                mail_free(&mail);
                continue;
            }

            if (post_has_id(&post, mail.id.data)) {
                puts("Отправление с таким ID уже существует.");
                mail_free(&mail);
                continue;
            }

            if (!post_add(&post, &mail))
                puts("Не удалось добавить отправление.");
            else
                puts("Отправление добавлено.");

            mail_free(&mail);
        }
        else if (strcmp(choice, "2") == 0) {
            char id[32];

            if (!read_nonempty("Введите ID: ", id, sizeof(id)))
                continue;

            if (post_delete(&post, id))
                puts("Отправление удалено.");
            else
                puts("Отправление не найдено.");
        }
        else if (strcmp(choice, "3") == 0) {
            char id[32];

            if (!read_nonempty("Введите ID: ", id, sizeof(id)))
                continue;

            Mail *mail = post_find(&post, id);

            if (mail != NULL)
                mail_print(mail);
            else
                puts("Отправление не найдено.");
        }
        else if (strcmp(choice, "4") == 0) {
            post_sort(&post);
            puts("Отправления отсортированы по индексу и ID.");
        }
        else if (strcmp(choice, "5") == 0) {
            post_print_delivered(&post);
        }
        else if (strcmp(choice, "6") == 0) {
            post_print_overdue(&post);
        }
        else if (strcmp(choice, "7") == 0) {
            for (size_t i = 0; i < post.count; ++i)
                mail_print(&post.mails[i]);
        }
        else if (strcmp(choice, "0") == 0) {
            running = 0;
        }
        else {
            puts("Неизвестная команда.");
        }
    }

    post_free(&post);
    return 0;
}
