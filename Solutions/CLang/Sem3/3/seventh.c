/*
 * Task 7 — Residents database (C99)
 *
 * Input file format: one resident per line
 * surname name patronymic day month year gender income
 *
 * All name fields are single tokens. Use "-" for an empty patronymic.
 * Example:
 * Smith John Paul 12 3 1980 M 1250.50
 * Brown Anna - 5 11 1995 W 980.00
 *
 * Run: ./livers residents.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>


#define NAME_CAP 128
#define PATH_CAP 512
#define LINE_CAP 512

typedef struct {
    int day;
    int month;
    int year;
} Date;

typedef struct {
    char *surname;
    char *name;
    char *patronymic;
    Date birth;
    char gender;
    double income;
} Liver;

typedef struct Node {
    Liver value;
    struct Node *next;
} Node;

typedef enum {
    HISTORY_ADD,       /* Undo by removing the added node. */
    HISTORY_DELETE,    /* Undo by reinserting the detached node. */
    HISTORY_EDIT       /* Undo by restoring the previous value. */
} HistoryType;

typedef struct {
    HistoryType type;
    Node *node;
    Liver old_value;   /* Used only by HISTORY_EDIT. */
} HistoryEntry;

typedef struct {
    HistoryEntry *items;
    size_t size;
    size_t capacity;
    size_t modifications;
} History;

static char *duplicate_string(const char *source)
{
    size_t length;
    char *copy;

    if (source == NULL)
        return NULL;

    length = strlen(source);
    copy = (char *)malloc(length + 1);
    if (copy != NULL)
        memcpy(copy, source, length + 1);

    return copy;
}

static void liver_init(Liver *person)
{
    person->surname = NULL;
    person->name = NULL;
    person->patronymic = NULL;
    person->birth.day = 1;
    person->birth.month = 1;
    person->birth.year = 1900;
    person->gender = 'M';
    person->income = 0.0;
}

static void liver_free(Liver *person)
{
    if (person == NULL)
        return;

    free(person->surname);
    free(person->name);
    free(person->patronymic);
    liver_init(person);
}

static int liver_copy(Liver *destination, const Liver *source)
{
    Liver copy;

    liver_init(&copy);
    copy.surname = duplicate_string(source->surname);
    copy.name = duplicate_string(source->name);
    copy.patronymic = duplicate_string(source->patronymic);

    if (copy.surname == NULL || copy.name == NULL ||
        copy.patronymic == NULL) {
        liver_free(&copy);
        return 0;
    }

    copy.birth = source->birth;
    copy.gender = source->gender;
    copy.income = source->income;

    liver_free(destination);
    *destination = copy;
    return 1;
}

static int is_leap_year(int year)
{
    return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}

static int valid_date(Date date)
{
    static const int month_days[12] =
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maximum;

    if (date.year < 1 || date.month < 1 || date.month > 12 ||
        date.day < 1)
        return 0;

    maximum = month_days[date.month - 1];
    if (date.month == 2 && is_leap_year(date.year))
        maximum = 29;

    return date.day <= maximum;
}

/* 1 means a is earlier (older birth date), -1 means later. */
static int compare_dates(Date a, Date b)
{
    if (a.year != b.year)
        return a.year < b.year ? -1 : 1;
    if (a.month != b.month)
        return a.month < b.month ? -1 : 1;
    if (a.day != b.day)
        return a.day < b.day ? -1 : 1;
    return 0;
}

/*
 * Increasing age means youngest first, oldest last.
 * Therefore, birth dates are stored in descending order.
 */
static int compare_livers(const Liver *a, const Liver *b)
{
    int result = compare_dates(a->birth, b->birth);

    if (result != 0)
        return -result;

    result = strcmp(a->surname, b->surname);
    if (result != 0)
        return result;

    result = strcmp(a->name, b->name);
    if (result != 0)
        return result;

    return strcmp(a->patronymic, b->patronymic);
}

static int latin_nonempty(const char *string)
{
    size_t i;

    if (string == NULL || string[0] == '\0')
        return 0;

    for (i = 0; string[i] != '\0'; ++i) {
        if (!((string[i] >= 'A' && string[i] <= 'Z') ||
              (string[i] >= 'a' && string[i] <= 'z')))
            return 0;
    }

    return 1;
}

static int latin_or_empty(const char *string)
{
    size_t i;

    if (string == NULL)
        return 0;

    for (i = 0; string[i] != '\0'; ++i) {
        if (!((string[i] >= 'A' && string[i] <= 'Z') ||
              (string[i] >= 'a' && string[i] <= 'z')))
            return 0;
    }

    return 1;
}

static int valid_liver(const Liver *person)
{
    return person != NULL &&
           latin_nonempty(person->surname) &&
           latin_nonempty(person->name) &&
           latin_or_empty(person->patronymic) &&
           valid_date(person->birth) &&
           (person->gender == 'M' || person->gender == 'W') &&
           isfinite(person->income) && person->income >= 0.0;
}

static Node *create_node(const Liver *person)
{
    Node *node = (Node *)malloc(sizeof(Node));

    if (node == NULL)
        return NULL;

    liver_init(&node->value);
    if (!liver_copy(&node->value, person)) {
        free(node);
        return NULL;
    }

    node->next = NULL;
    return node;
}

static void destroy_node(Node *node)
{
    if (node == NULL)
        return;

    liver_free(&node->value);
    free(node);
}

static void insert_sorted(Node **head, Node *node)
{
    Node **position = head;

    while (*position != NULL &&
           compare_livers(&(*position)->value, &node->value) <= 0)
        position = &(*position)->next;

    node->next = *position;
    *position = node;
}

static void free_list(Node *head)
{
    while (head != NULL) {
        Node *next = head->next;
        destroy_node(head);
        head = next;
    }
}

static void history_init(History *history)
{
    history->items = NULL;
    history->size = 0;
    history->capacity = 0;
    history->modifications = 0;
}

static int history_push(History *history, HistoryEntry entry)
{
    HistoryEntry *temporary;
    size_t new_capacity;

    if (history->size == history->capacity) {
        new_capacity = history->capacity == 0 ? 8 : history->capacity * 2;

        if (new_capacity < history->capacity ||
            new_capacity > SIZE_MAX / sizeof(HistoryEntry))
            return 0;

        temporary = (HistoryEntry *)realloc(
            history->items, new_capacity * sizeof(HistoryEntry));

        if (temporary == NULL)
            return 0;

        history->items = temporary;
        history->capacity = new_capacity;
    }

    history->items[history->size++] = entry;
    history->modifications++;
    return 1;
}

/*
 * Frees resources retained by history entries that are no longer undoable.
 * A deleted node is detached from the list and owned by its DELETE entry.
 */
static void history_clear(History *history)
{
    size_t i;

    for (i = 0; i < history->size; ++i) {
        HistoryEntry *entry = &history->items[i];

        if (entry->type == HISTORY_DELETE)
            destroy_node(entry->node);
        else if (entry->type == HISTORY_EDIT)
            liver_free(&entry->old_value);
    }

    free(history->items);
    history_init(history);
}

static int remove_node_pointer(Node **head, Node *target)
{
    Node **position = head;

    while (*position != NULL) {
        if (*position == target) {
            *position = target->next;
            target->next = NULL;
            return 1;
        }
        position = &(*position)->next;
    }

    return 0;
}

static void undo_entry(Node **head, HistoryEntry *entry)
{
    if (entry->type == HISTORY_ADD) {
        if (remove_node_pointer(head, entry->node))
            destroy_node(entry->node);
    } else if (entry->type == HISTORY_DELETE) {
        insert_sorted(head, entry->node);
        entry->node = NULL;
    } else if (entry->type == HISTORY_EDIT) {
        liver_free(&entry->node->value);
        entry->node->value = entry->old_value;
        liver_init(&entry->old_value);
        /* Editing can change the sorting key: reposition the node. */
        remove_node_pointer(head, entry->node);
        insert_sorted(head, entry->node);
    }
}

static void undo_half(Node **head, History *history)
{
    size_t undo_count = history->modifications / 2;
    size_t i;

    if (undo_count == 0) {
        puts("There are not enough modifications to undo (floor(N/2) = 0).");
        return;
    }

    printf("Undoing %zu of %zu modification(s)...\n",
           undo_count, history->modifications);

    for (i = 0; i < undo_count; ++i) {
        HistoryEntry *entry = &history->items[history->size - 1];
        undo_entry(head, entry);
        history->size--;
    }

    /*
     * The specification starts a new modification counter after Undo.
     * Remaining earlier changes are committed; release their undo data.
     */
    history_clear(history);
    puts("Undo completed.");
}

static int read_person(Liver *person)
{
    char surname[NAME_CAP], name[NAME_CAP], patronymic[NAME_CAP];
    char gender;
    int day, month, year;
    double income;

    printf("Surname (Latin letters): ");
    if (scanf("%127s", surname) != 1)
        return 0;

    printf("Name (Latin letters): ");
    if (scanf("%127s", name) != 1)
        return 0;

    printf("Patronymic (use - if empty): ");
    if (scanf("%127s", patronymic) != 1)
        return 0;

    printf("Birth date (day month year): ");
    if (scanf("%d %d %d", &day, &month, &year) != 3)
        return 0;

    printf("Gender (M/W): ");
    if (scanf(" %c", &gender) != 1)
        return 0;

    printf("Monthly average income: ");
    if (scanf("%lf", &income) != 1)
        return 0;

    if (strcmp(patronymic, "-") == 0)
        patronymic[0] = '\0';

    liver_init(person);
    person->surname = duplicate_string(surname);
    person->name = duplicate_string(name);
    person->patronymic = duplicate_string(patronymic);

    if (person->surname == NULL || person->name == NULL ||
        person->patronymic == NULL) {
        liver_free(person);
        return 0;
    }

    person->birth.day = day;
    person->birth.month = month;
    person->birth.year = year;
    person->gender = (char)toupper((unsigned char)gender);
    person->income = income;

    if (!valid_liver(person)) {
        liver_free(person);
        return 0;
    }

    return 1;
}

static void print_person(const Liver *person)
{
    printf("%s %s %s | birth: %02d.%02d.%04d | gender: %c | income: %.2f\n",
           person->surname,
           person->name,
           person->patronymic[0] ? person->patronymic : "(none)",
           person->birth.day, person->birth.month, person->birth.year,
           person->gender, person->income);
}

static int same_search_key(const Liver *person,
                           const char *surname,
                           const char *name,
                           Date birth)
{
    return strcmp(person->surname, surname) == 0 &&
           strcmp(person->name, name) == 0 &&
           compare_dates(person->birth, birth) == 0;
}

static Node *find_person(Node *head, const char *surname,
                         const char *name, Date birth)
{
    Node *node;

    for (node = head; node != NULL; node = node->next) {
        if (same_search_key(&node->value, surname, name, birth))
            return node;
    }

    return NULL;
}

static int read_search_key(char *surname, char *name, Date *birth)
{
    printf("Surname: ");
    if (scanf("%127s", surname) != 1)
        return 0;

    printf("Name: ");
    if (scanf("%127s", name) != 1)
        return 0;

    printf("Birth date (day month year): ");
    if (scanf("%d %d %d", &birth->day, &birth->month,
              &birth->year) != 3)
        return 0;

    return latin_nonempty(surname) && latin_nonempty(name) &&
           valid_date(*birth);
}

static int add_resident(Node **head, History *history,
                        const Liver *person)
{
    Node *node = create_node(person);
    HistoryEntry entry;

    if (node == NULL)
        return 0;

    entry.type = HISTORY_ADD;
    entry.node = node;
    liver_init(&entry.old_value);

    if (!history_push(history, entry)) {
        destroy_node(node);
        return 0;
    }

    insert_sorted(head, node);
    return 1;
}

static int delete_resident(Node **head, History *history, Node *node)
{
    HistoryEntry entry;

    entry.type = HISTORY_DELETE;
    entry.node = node;
    liver_init(&entry.old_value);

    if (!history_push(history, entry))
        return 0;

    remove_node_pointer(head, node);
    return 1;
}

static int edit_resident(Node **head, History *history,
                         Node *node, const Liver *replacement)
{
    HistoryEntry entry;

    entry.type = HISTORY_EDIT;
    entry.node = node;
    liver_init(&entry.old_value);

    if (!liver_copy(&entry.old_value, &node->value))
        return 0;

    if (!history_push(history, entry)) {
        liver_free(&entry.old_value);
        return 0;
    }

    liver_free(&node->value);
    liver_init(&node->value);

    if (!liver_copy(&node->value, replacement)) {
        /* This should only occur on allocation failure. Restore old data. */
        node->value = history->items[history->size - 1].old_value;
        liver_init(&history->items[history->size - 1].old_value);
        history->size--;
        history->modifications--;
        return 0;
    }

    remove_node_pointer(head, node);
    insert_sorted(head, node);
    return 1;
}

static int load_file(const char *path, Node **head)
{
    FILE *file = fopen(path, "r");
    char line[LINE_CAP];
    int line_number = 0;

    if (file == NULL) {
        fprintf(stderr, "Cannot open input file: %s\n", path);
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char surname[NAME_CAP], name[NAME_CAP], patronymic[NAME_CAP];
        char gender, extra;
        int day, month, year, fields;
        double income;
        Liver person;
        Node *node;

        ++line_number;

        if (line[0] == '\n' || line[0] == '\r' || line[0] == '#')
            continue;

        fields = sscanf(line, "%127s %127s %127s %d %d %d %c %lf %c",
                        surname, name, patronymic, &day, &month, &year,
                        &gender, &income, &extra);

        if (fields != 8) {
            fprintf(stderr, "Invalid record at %s:%d\n",
                    path, line_number);
            fclose(file);
            return 0;
        }

        if (strcmp(patronymic, "-") == 0)
            patronymic[0] = '\0';

        liver_init(&person);
        person.surname = duplicate_string(surname);
        person.name = duplicate_string(name);
        person.patronymic = duplicate_string(patronymic);

        if (person.surname == NULL || person.name == NULL ||
            person.patronymic == NULL) {
            liver_free(&person);
            fclose(file);
            return 0;
        }

        person.birth.day = day;
        person.birth.month = month;
        person.birth.year = year;
        person.gender = (char)toupper((unsigned char)gender);
        person.income = income;

        if (!valid_liver(&person)) {
            fprintf(stderr, "Invalid resident data at %s:%d\n",
                    path, line_number);
            liver_free(&person);
            fclose(file);
            return 0;
        }

        node = create_node(&person);
        liver_free(&person);

        if (node == NULL) {
            fclose(file);
            return 0;
        }

        insert_sorted(head, node);
    }

    if (ferror(file)) {
        fprintf(stderr, "Error while reading: %s\n", path);
        fclose(file);
        return 0;
    }

    if (fclose(file) != 0) {
        fprintf(stderr, "Error while closing: %s\n", path);
        return 0;
    }

    return 1;
}

static int save_file(const char *path, const Node *head)
{
    FILE *file = fopen(path, "w");
    const Node *node;

    if (file == NULL) {
        fprintf(stderr, "Cannot open output file: %s\n", path);
        return 0;
    }

    for (node = head; node != NULL; node = node->next) {
        const Liver *p = &node->value;
        if (fprintf(file, "%s %s %s %d %d %d %c %.17g\n",
                    p->surname, p->name,
                    p->patronymic[0] ? p->patronymic : "-",
                    p->birth.day, p->birth.month, p->birth.year,
                    p->gender, p->income) < 0) {
            fprintf(stderr, "Write error: %s\n", path);
            fclose(file);
            return 0;
        }
    }

    if (fclose(file) != 0) {
        fprintf(stderr, "Close error: %s\n", path);
        return 0;
    }

    return 1;
}

static void print_all(const Node *head)
{
    const Node *node;

    if (head == NULL) {
        puts("The list is empty.");
        return;
    }

    for (node = head; node != NULL; node = node->next)
        print_person(&node->value);
}

static void print_menu(void)
{
    puts("\n--- RESIDENTS DATABASE ---");
    puts("1. Print all residents");
    puts("2. Search resident");
    puts("3. Add resident");
    puts("4. Edit resident");
    puts("5. Delete resident");
    puts("6. Undo floor(N/2) latest modifications");
    puts("7. Save list to file");
    puts("0. Exit");
    printf("Choice: ");
}

static void run_dialog(Node **head, History *history)
{
    for (;;) {
        int choice;

        print_menu();

        if (scanf("%d", &choice) != 1) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            puts("Invalid menu input.");
            continue;
        }

        if (choice == 0)
            break;

        if (choice == 1) {
            print_all(*head);
        } else if (choice == 2 || choice == 4 || choice == 5) {
            char surname[NAME_CAP], name[NAME_CAP];
            Date birth;
            Node *found;

            if (!read_search_key(surname, name, &birth)) {
                puts("Invalid search parameters.");
                continue;
            }

            found = find_person(*head, surname, name, birth);
            if (found == NULL) {
                puts("Resident not found.");
                continue;
            }

            if (choice == 2) {
                print_person(&found->value);
            } else if (choice == 5) {
                if (delete_resident(head, history, found))
                    puts("Resident deleted.");
                else
                    puts("Memory error: deletion was not performed.");
            } else {
                Liver replacement;

                puts("Enter new resident data:");
                liver_init(&replacement);

                if (!read_person(&replacement)) {
                    liver_free(&replacement);
                    puts("Invalid resident data.");
                    continue;
                }

                if (edit_resident(head, history, found, &replacement))
                    puts("Resident updated.");
                else
                    puts("Memory error: modification was not performed.");

                liver_free(&replacement);
            }
        } else if (choice == 3) {
            Liver person;

            liver_init(&person);
            if (!read_person(&person)) {
                liver_free(&person);
                puts("Invalid resident data.");
                continue;
            }

            if (add_resident(head, history, &person))
                puts("Resident added.");
            else
                puts("Memory error: resident was not added.");

            liver_free(&person);
        } else if (choice == 6) {
            undo_half(head, history);
        } else if (choice == 7) {
            char path[PATH_CAP];

            printf("Output file path: ");
            if (scanf("%511s", path) != 1) {
                puts("Invalid path.");
                continue;
            }

            if (save_file(path, *head))
                puts("Data saved.");
        } else {
            puts("Unknown menu option.");
        }
    }
}

int main(int argc, char **argv)
{
    Node *head = NULL;
    History history;
    int result = 0;

    history_init(&history);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s input_file\n", argv[0]);
        return 1;
    }

    if (!load_file(argv[1], &head)) {
        free_list(head);
        history_clear(&history);
        return 1;
    }

    run_dialog(&head, &history);

    history_clear(&history);
    free_list(head);

    result = 0;
    return result;
}
