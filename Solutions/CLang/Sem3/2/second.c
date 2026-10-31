
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>

#include "myerrors.h"

typedef struct {
    unsigned int id;
    char *name;
    char *surname;
    double salary;
} Employee;


static ErrorsDef read_token(FILE *file, char **result)
{
    int ch;

    do {
        ch = fgetc(file);
    } while (ch != EOF && isspace((unsigned char)ch));

    if (ch == EOF) {
        if (ferror(file))
            return FILE_ERROR;
        return TOKEN_EOF;
    }

    size_t capacity = 16;
    size_t length = 0;
    char *token = (char*)malloc(sizeof(char)*capacity);

    if (token == NULL)
        return FILE_ERROR;

    while (ch != EOF && !isspace((unsigned char)ch)) {
        if (length >= capacity - 1) {
            if (capacity > SIZE_MAX - 1) {
                free(token);
                return FILE_ERROR;
            }

            size_t new_capacity = capacity + 1;
            char *temp = (char*)realloc(token, new_capacity);

            if (temp == NULL) {
                free(token);
                return FILE_ERROR;
            }

            token = temp;
            capacity = new_capacity;
        }

        token[length++] = (char)ch;
        ch = fgetc(file);
    }

    if (ch == EOF && ferror(file)) {
        free(token);
        return FILE_ERROR;
    }

    token[length] = '\0';
    *result = token;

    return STAT_OK;
}


static ErrorsDef parse_uint(const char *str, unsigned int *result)
{
    unsigned int value = 0;

    if (str == NULL || result == NULL || str[0] == '\0')
        return INVALID_NUMBER_FORMAT;

    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (str[i] < '0' || str[i] > '9')
            return INVALID_NUMBER_FORMAT;

        unsigned int digit = (unsigned int)(str[i] - '0');

        if (value > (UINT_MAX - digit) / 10U)
            return INPUT_OVERFLOW;

        value = value * 10U + digit;
    }

    *result = value;
    return STAT_OK;
}


static int valid_name(const char *str)
{
    if (str == NULL || str[0] == '\0')
        return 0;

    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (!((str[i] >= 'A' && str[i] <= 'Z') ||
              (str[i] >= 'a' && str[i] <= 'z')))
            return 0;
    }

    return 1;
}


static void free_employee(Employee *employee)
{
    if (employee != NULL) {
        free(employee->name);
        free(employee->surname);

        employee->name = NULL;
        employee->surname = NULL;
    }
}


static void free_employees(Employee *employees, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        free_employee(&employees[i]);

    free(employees);
}

static int compare_strings(const char *a, const char *b)
{
    size_t i = 0;

    while (a[i] != '\0' && b[i] != '\0') {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];

        if (ca < cb)
            return -1;

        if (ca > cb)
            return 1;

        ++i;
    }

    if (a[i] == '\0' && b[i] == '\0')
        return 0;

    return a[i] == '\0' ? -1 : 1;
}


static int compare_salary(double a, double b)
{
    if (a < b)
        return -1;

    if (a > b)
        return 1;

    return 0;
}

/* Полное сравнение по возрастанию */
static int compare_employee_ascending(const void *pa, const void *pb)
{
    const Employee *a = pa;
    const Employee *b = pb;

    int cmp = compare_salary(a->salary, b->salary);

    if (cmp != 0)
        return cmp;

    cmp = compare_strings(a->surname, b->surname);

    if (cmp != 0)
        return cmp;

    cmp = compare_strings(a->name, b->name);

    if (cmp != 0)
        return cmp;

    if (a->id < b->id)
        return -1;

    if (a->id > b->id)
        return 1;

    return 0;
}

/* Полное сравнение по убыванию */
static int compare_employee_descending(const void *pa, const void *pb)
{
    int cmp = compare_employee_ascending(pa, pb);

    return -cmp;
}


static ErrorsDef load_employees(FILE *file, Employee **result, size_t *result_count)
{
    Employee *employees = NULL;
    size_t count = 0;
    size_t capacity = 0;

    for (;;) {
        char *id_token = NULL;
        char *name = NULL;
        char *surname = NULL;
        char *salary_token = NULL;

        ErrorsDef ts = read_token(file, &id_token);

        if (ts == TOKEN_EOF)
            break;

        if (ts == FILE_ERROR) {
            free_employees(employees, count);
            return READFILE_ERROR;
        }

        ts = read_token(file, &name);
        if (ts != STAT_OK) {
            free(id_token);
            free(name);
            free_employees(employees, count);
            return ts == FILE_ERROR ? READFILE_ERROR : INVALID_INPUT;
        }

        ts = read_token(file, &surname);
        if (ts != STAT_OK) {
            free(id_token);
            free(name);
            free(surname);
            free_employees(employees, count);
            return ts == FILE_ERROR ? READFILE_ERROR : INVALID_INPUT;
        }

        ts = read_token(file, &salary_token);
        if (ts != STAT_OK) {
            free(id_token);
            free(name);
            free(surname);
            free(salary_token);
            free_employees(employees, count);
            return ts == FILE_ERROR ? READFILE_ERROR : INVALID_INPUT;
        }

        Employee employee = {0};

        ErrorsDef status = parse_uint(id_token, &employee.id);

        if (status != STAT_OK ||
            !valid_name(name) ||
            !valid_name(surname)) {
            free(id_token);
            free(name);
            free(surname);
            free(salary_token);
            free_employees(employees, count);

            return status;
        }

        char *end = NULL;
        double salary = strtod(salary_token, &end);

        if (end == salary_token || *end != '\0' || !isfinite(salary) || salary < 0.0) {
            free(id_token);
            free(name);
            free(surname);
            free(salary_token);
            free_employees(employees, count);
            return INVALID_INPUT;
        }

        employee.salary = salary;
        employee.name = name;
        employee.surname = surname;

        free(id_token);
        free(salary_token);

        if (count == capacity) {
            size_t new_capacity;

            if (capacity == 0) {
                new_capacity = 8;
            } else {
                if (capacity > SIZE_MAX - 1) {
                    free_employee(&employee);
                    free_employees(employees, count);
                    return MEMORY_ERROR;
                }

                new_capacity = capacity + 1;
            }

            if (new_capacity > SIZE_MAX / sizeof(*employees)) {
                free_employee(&employee);
                free_employees(employees, count);
                return MEMORY_ERROR;
            }

            Employee *temp = (Employee*)realloc(employees, new_capacity * sizeof(*employees));

            if (temp == NULL) {
                free_employee(&employee);
                free_employees(employees, count);
                return REALLOC_ERROR;
            }

            employees = temp;
            capacity = new_capacity;
        }

        employees[count++] = employee;
    }

    *result = employees;
    *result_count = count;

    return STAT_OK;
}


static ErrorsDef save_employees(FILE *file, const Employee *employees, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        if (fprintf(file, "%u %s %s %.17g\n", employees[i].id, employees[i].name, employees[i].surname, employees[i].salary) < 0)
            return WRITEFILE_ERROR;
    }

    if (ferror(file))
        return WRITEFILE_ERROR;

    return STAT_OK;
}

static int report_error(ErrorsDef error) {
    if(error!=STAT_OK) {
        errors(error);
    }

    return (int)error;
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        return report_error(MATCH_ARGS);

    }

    const char *input_path = argv[1];
    const char *flag = argv[2];
    const char *output_path = argv[3];

    int ascending;

    if (flag[0] == '-' || flag[0] == '/') {
        if (flag[1] == 'a' && flag[2] == '\0') {
            ascending = 1;
        } else if (flag[1] == 'd' && flag[2] == '\0') {
            ascending = 0;
        } else {
            return report_error(INVALID_FLAG);

        }
    } else {
        return report_error(INVALID_FLAG);
    }

    FILE *input = fopen(input_path, "r");

    if (input == NULL) {
        return report_error(OPENFILE_ERROR);

    }

    Employee *employees = NULL;
    size_t count = 0;

    ErrorsDef status = load_employees(input, &employees, &count);

    if (fclose(input) != 0 && status == STAT_OK)
        status = CLOSEFILE_ERROR;

    if (status != STAT_OK) {
        return report_error(status);

    }

    if (count > 1) {
        qsort(employees, count, sizeof(*employees), ascending ? compare_employee_ascending : compare_employee_descending);
    }

    FILE *output = fopen(output_path, "w");

    if (output == NULL) {
        
        free_employees(employees, count);
        return report_error(OPENFILE_ERROR);
    }

    status = save_employees(output, employees, count);

    if (fclose(output) != 0 && status == STAT_OK)
        status = CLOSEFILE_ERROR;

    free_employees(employees, count);

    if (status != STAT_OK) {
        return report_error(status);

    }

    return STAT_OK;
}
