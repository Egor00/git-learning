
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <limits.h>

#include "myerrors.h"

#define TEXT_SIZE 256
#define EXAMS_COUNT 5

typedef struct {
    unsigned int id;
    char *name;
    char *surname;
    char *group;
    unsigned char *grades;
} Student;

typedef struct {
    Student *data;
    size_t size;
    size_t capacity;
} StudentArray;

/* ---------- Memory and strings ---------- */

static char *copy_string(const char *src) {
    size_t len = strlen(src);
    char *result = malloc(len + 1);

    if (result == NULL)
        return NULL;

    memcpy(result, src, len + 1);
    return result;
}

static void student_free(Student *s) {
    if (s == NULL)
        return;

    free(s->name);
    free(s->surname);
    free(s->group);
    free(s->grades);

    memset(s, 0, sizeof(*s));
}

static void array_free(StudentArray *a) {
    for (size_t i = 0; i < a->size; ++i)
        student_free(&a->data[i]);

    free(a->data);

    a->data = NULL;
    a->size = 0;
    a->capacity = 0;
}

static int array_push(StudentArray *a, Student *s) {
    if (a->size == a->capacity) {
        size_t new_capacity =
            a->capacity == 0 ? 8 : a->capacity * 2;

        if (new_capacity < a->capacity ||
            new_capacity > SIZE_MAX / sizeof(Student))
            return 0;

        Student *tmp = realloc(
            a->data,
            new_capacity * sizeof(Student)
        );

        if (tmp == NULL)
            return 0;

        a->data = tmp;
        a->capacity = new_capacity;
    }

    a->data[a->size++] = *s;
    memset(s, 0, sizeof(*s));

    return 1;
}

/* ---------- Validation ---------- */

static int is_latin_word(const char *s) {
    if (s == NULL || *s == '\0')
        return 0;

    for (size_t i = 0; s[i] != '\0'; ++i) {
        if (!((s[i] >= 'a' && s[i] <= 'z') ||
              (s[i] >= 'A' && s[i] <= 'Z')))
            return 0;
    }

    return 1;
}

static int is_valid_group(const char *s) {
    return s != NULL && s[0] != '\0';
}

static int id_exists(const StudentArray *a, unsigned int id) {
    for (size_t i = 0; i < a->size; ++i) {
        if (a->data[i].id == id)
            return 1;
    }

    return 0;
}

/* ---------- File loading ---------- */

/*
 * Формат одной строки:
 * id name surname group grade1 grade2 grade3 grade4 grade5
 *
 * Пустые строки и строки, начинающиеся с #, пропускаются.
 */

static int load_students(const char *path, StudentArray *a) {
    FILE *file = fopen(path, "r");

    if (file == NULL) {
        perror("Не удалось открыть файл студентов");
        return 0;
    }

    char line[TEXT_SIZE * 2];
    size_t line_number = 0;
    int success = 1;

    while (fgets(line, sizeof(line), file) != NULL) {
        ++line_number;

        if (line[0] == '\n' || line[0] == '\r' ||
            line[0] == '#')
            continue;

        unsigned long id;
        unsigned int grades[EXAMS_COUNT];
        char name[TEXT_SIZE];
        char surname[TEXT_SIZE];
        char group[TEXT_SIZE];
        char extra;

        int count = sscanf(
            line,
            "%lu %255s %255s %255s %u %u %u %u %u %c",
            &id,
            name,
            surname,
            group,
            &grades[0],
            &grades[1],
            &grades[2],
            &grades[3],
            &grades[4],
            &extra
        );

        if (count != 9 ||
            id > UINT_MAX ||
            !is_latin_word(name) ||
            !is_latin_word(surname) ||
            !is_valid_group(group)) {
            fprintf(stderr,
                    "Ошибка формата в строке %zu\n",
                    line_number);
            success = 0;
            break;
        }

        for (int i = 0; i < EXAMS_COUNT; ++i) {
            if (grades[i] > UCHAR_MAX) {
                fprintf(stderr,
                        "Некорректная оценка в строке %zu\n",
                        line_number);
                success = 0;
                break;
            }
        }

        if (!success)
            break;

        if (id_exists(a, (unsigned int)id)) {
            fprintf(stderr,
                    "Повторяющийся ID в строке %zu\n",
                    line_number);
            success = 0;
            break;
        }

        Student s = {0};

        s.id = (unsigned int)id;
        s.name = copy_string(name);
        s.surname = copy_string(surname);
        s.group = copy_string(group);
        s.grades = malloc(EXAMS_COUNT * sizeof(unsigned char));

        if (s.name == NULL ||
            s.surname == NULL ||
            s.group == NULL ||
            s.grades == NULL) {
            student_free(&s);
            success = 0;
            break;
        }

        for (int i = 0; i < EXAMS_COUNT; ++i)
            s.grades[i] = (unsigned char)grades[i];

        if (!array_push(a, &s)) {
            student_free(&s);
            success = 0;
            break;
        }
    }

    if (ferror(file)) {
        perror("Ошибка чтения файла");
        success = 0;
    }

    if (fclose(file) != 0) {
        perror("Ошибка закрытия файла");
        success = 0;
    }

    return success;
}

/* ---------- Student information ---------- */

static double student_average(const Student *s) {
    unsigned int sum = 0;

    for (int i = 0; i < EXAMS_COUNT; ++i)
        sum += s->grades[i];

    return (double)sum / EXAMS_COUNT;
}

static void student_print(const Student *s) {
    printf("\nID: %u\n", s->id);
    printf("Имя: %s\n", s->name);
    printf("Фамилия: %s\n", s->surname);
    printf("Группа: %s\n", s->group);

    printf("Оценки: ");

    for (int i = 0; i < EXAMS_COUNT; ++i)
        printf("%u ", (unsigned int)s->grades[i]);

    printf("\nСредний балл: %.2f\n",
           student_average(s));
}

static double all_students_average(const StudentArray *a) {
    if (a->size == 0)
        return 0.0;

    unsigned long long sum = 0;

    for (size_t i = 0; i < a->size; ++i) {
        for (int j = 0; j < EXAMS_COUNT; ++j)
            sum += a->data[i].grades[j];
    }

    return (double)sum / (a->size * EXAMS_COUNT);
}

/* ---------- Comparators for qsort ---------- */

static int compare_id(const void *pa, const void *pb) {
    const Student *a = pa;
    const Student *b = pb;

    if (a->id < b->id) return -1;
    if (a->id > b->id) return 1;
    return 0;
}

static int compare_name(const void *pa, const void *pb) {
    const Student *a = pa;
    const Student *b = pb;

    return strcmp(a->name, b->name);
}

static int compare_surname(const void *pa, const void *pb) {
    const Student *a = pa;
    const Student *b = pb;

    return strcmp(a->surname, b->surname);
}

static int compare_group(const void *pa, const void *pb) {
    const Student *a = pa;
    const Student *b = pb;

    return strcmp(a->group, b->group);
}

/* ---------- Search ---------- */

static void search_id(const StudentArray *a, unsigned int id) {
    int found = 0;

    for (size_t i = 0; i < a->size; ++i) {
        if (a->data[i].id == id) {
            student_print(&a->data[i]);
            found = 1;
        }
    }

    if (!found)
        puts("Студенты с таким ID не найдены.");
}

static void search_string(
    const StudentArray *a,
    const char *value,
    int field
) {
    int found = 0;

    for (size_t i = 0; i < a->size; ++i) {
        const char *current = NULL;

        if (field == 1)
            current = a->data[i].surname;
        else if (field == 2)
            current = a->data[i].name;
        else
            current = a->data[i].group;

        if (strcmp(current, value) == 0) {
            student_print(&a->data[i]);
            found = 1;
        }
    }

    if (!found)
        puts("Совпадений не найдено.");
}

/* ---------- Trace file ---------- */

static int trace_student_by_id(
    const StudentArray *a,
    const char *trace_path,
    unsigned int id
) {
    FILE *file = fopen(trace_path, "a");

    if (file == NULL) {
        perror("Не удалось открыть трассировочный файл");
        return 0;
    }

    int found = 0;

    for (size_t i = 0; i < a->size; ++i) {
        const Student *s = &a->data[i];

        if (s->id == id) {
            fprintf(file, "Найден студент:\n");
            fprintf(file, "ID: %u\n", s->id);
            fprintf(file, "ФИО: %s %s\n",
                    s->surname, s->name);
            fprintf(file, "Группа: %s\n", s->group);
            fprintf(file, "Средний балл: %.2f\n\n",
                    student_average(s));

            found = 1;
            break;
        }
    }

    if (!found)
        fprintf(file, "Студент с ID %u не найден.\n\n", id);

    if (fclose(file) != 0) {
        perror("Ошибка закрытия трассировочного файла");
        return 0;
    }

    return 1;
}

static int trace_above_average(
    const StudentArray *a,
    const char *trace_path
) {
    FILE *file = fopen(trace_path, "a");

    if (file == NULL) {
        perror("Не удалось открыть трассировочный файл");
        return 0;
    }

    double average = all_students_average(a);

    fprintf(file,
            "Студенты со средним баллом выше общего среднего:\n");
    fprintf(file, "Общий средний балл: %.2f\n", average);

    for (size_t i = 0; i < a->size; ++i) {
        const Student *s = &a->data[i];

        if (student_average(s) > average) {
            fprintf(file, "%s %s (%.2f)\n",
                    s->surname,
                    s->name,
                    student_average(s));
        }
    }

    fprintf(file, "\n");

    if (fclose(file) != 0) {
        perror("Ошибка закрытия трассировочного файла");
        return 0;
    }

    return 1;
}

/* ---------- Input ---------- */

static int read_line(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);

    if (fgets(buffer, (int)size, stdin) == NULL)
        return 0;

    size_t len = strlen(buffer);

    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        return 1;
    }

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}

    return 0;
}

static int read_id(unsigned int *id) {
    char buffer[64];
    char *end;

    if (!read_line("Введите ID: ", buffer, sizeof(buffer)))
        return 0;

    errno = 0;
    unsigned long value = strtoul(buffer, &end, 10);

    if (errno != 0 || end == buffer ||
        *end != '\0' || value > UINT_MAX)
        return 0;

    *id = (unsigned int)value;
    return 1;
}

/* ---------- Menu ---------- */

static void print_menu(void) {
    puts("\n========== СТУДЕНТЫ ==========");
    puts("1. Поиск по ID");
    puts("2. Поиск по фамилии");
    puts("3. Поиск по имени");
    puts("4. Поиск по группе");
    puts("5. Сортировка по ID");
    puts("6. Сортировка по фамилии");
    puts("7. Сортировка по имени");
    puts("8. Сортировка по группе");
    puts("9. Записать найденного по ID студента в трассировку");
    puts("10. Записать студентов выше общего среднего балла");
    puts("11. Вывести всех студентов");
    puts("0. Выход");
    puts("==============================");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr,
                "Использование: %s students.txt trace.txt\n",
                argv[0]);
        return 1;
    }

    StudentArray students = {0};

    if (!load_students(argv[1], &students)) {
        array_free(&students);
        return 1;
    }

    printf("Загружено студентов: %zu\n", students.size);

    int running = 1;

    while (running) {
        char choice[32];
        print_menu();

        if (!read_line("Выберите действие: ",
                       choice, sizeof(choice))) {
            puts("Ошибка ввода.");
            continue;
        }

        if (strcmp(choice, "1") == 0) {
            unsigned int id;

            if (read_id(&id))
                search_id(&students, id);
            else
                puts("Некорректный ID.");
        }
        else if (strcmp(choice, "2") == 0 ||
                 strcmp(choice, "3") == 0 ||
                 strcmp(choice, "4") == 0) {
            char value[TEXT_SIZE];

            if (!read_line("Введите значение: ",
                           value, sizeof(value)) ||
                value[0] == '\0') {
                puts("Некорректный ввод.");
                continue;
            }

            int field = choice[0] - '0' - 1;
            search_string(&students, value, field);
        }
        else if (strcmp(choice, "5") == 0) {
            qsort(students.data, students.size,
                  sizeof(Student), compare_id);
            puts("Сортировка по ID выполнена.");
        }
        else if (strcmp(choice, "6") == 0) {
            qsort(students.data, students.size,
                  sizeof(Student), compare_surname);
            puts("Сортировка по фамилии выполнена.");
        }
        else if (strcmp(choice, "7") == 0) {
            qsort(students.data, students.size,
                  sizeof(Student), compare_name);
            puts("Сортировка по имени выполнена.");
        }
        else if (strcmp(choice, "8") == 0) {
            qsort(students.data, students.size,
                  sizeof(Student), compare_group);
            puts("Сортировка по группе выполнена.");
        }
        else if (strcmp(choice, "9") == 0) {
            unsigned int id;

            if (!read_id(&id)) {
                puts("Некорректный ID.");
                continue;
            }

            if (trace_student_by_id(&students, argv[2], id))
                puts("Результат записан в трассировочный файл.");
        }
        else if (strcmp(choice, "10") == 0) {
            if (trace_above_average(&students, argv[2]))
                puts("Результат записан в трассировочный файл.");
        }
        else if (strcmp(choice, "11") == 0) {
            for (size_t i = 0; i < students.size; ++i)
                student_print(&students.data[i]);
        }
        else if (strcmp(choice, "0") == 0) {
            running = 0;
        }
        else {
            puts("Неизвестная команда.");
        }
    }

    array_free(&students);
    return 0;
}
