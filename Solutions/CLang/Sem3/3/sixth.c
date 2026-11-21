/*
 * Задание 6. Фундаментальные алгоритмы, C99.
 *
 * Формат каждого входного файла:
 *   latitude longitude
 *   vehicle_id dd.MM.yyyy hh:mm:ss dd.MM.yyyy hh:mm:ss S|I|F
 *
 * S - начальная остановка, I - промежуточная, F - конечная.
 * Координаты в первой строке относятся ко всем записям данного файла.
 *
 * Запуск:
 *   ./transport stops1.txt stops2.txt ...
 *
 * Примечание: маршрут считается завершённым, если содержит S и F.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>

#include "myerrors.h"

#define LINE_SIZE 512
#define ID_SIZE 128
#define PI 3.14159265358979323846
#define EARTH_RADIUS_KM 6371.0

typedef enum {
    STOP_START,
    STOP_INTERMEDIATE,
    STOP_FINISH
} StopType;

typedef struct {
    double latitude;
    double longitude;
} Coordinates;

typedef struct Stop {
    Coordinates point;
    long long arrival;
    long long departure;
    StopType type;
    struct Stop *next;
} Stop;

typedef struct Route {
    char *vehicle_id;
    Stop *stops;
    Stop *last_stop;
    struct Route *next;
} Route;

typedef struct {
    char vehicle_id[ID_SIZE];
    Coordinates point;
    long long arrival;
    long long departure;
    StopType type;
} Record;

typedef struct {
    Route *head;
    Route *tail;
} Storage;

static void init_storage(Storage *storage)
{
    storage->head = NULL;
    storage->tail = NULL;
}

static char *copy_string(const char *src)
{
    size_t len;
    char *dst;

    if (src == NULL)
        return NULL;

    len = strlen(src);
    if (len == SIZE_MAX)
        return NULL;

    dst = (char *)malloc(len + 1);
    if (dst != NULL)
        memcpy(dst, src, len + 1);

    return dst;
}

/* Перевод даты в количество дней относительно 01.01.1970. */
static long long days_from_civil(int year, unsigned month, unsigned day)
{
    int era;
    unsigned yoe, doy, doe;

    year -= month <= 2;
    era = (year >= 0 ? year : year - 399) / 400;
    yoe = (unsigned)(year - era * 400);
    doy = (153 * (month + (month > 2 ? (unsigned)-3 : 9)) + 2) / 5
          + day - 1;
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;

    return (long long)era * 146097 + (long long)doe - 719468;
}

static int parse_datetime(const char *date, const char *clock,
                          long long *result)
{
    int day, month, year, hour, minute, second;
    long long days;

    if (date == NULL || clock == NULL || result == NULL)
        return 0;

    if (sscanf(date, "%2d.%2d.%4d", &day, &month, &year) != 3)
        return 0;

    if (sscanf(clock, "%2d:%2d:%2d", &hour, &minute, &second) != 3)
        return 0;

    if (year < 1 || month < 1 || month > 12 ||
        day < 1 || day > 31 || hour < 0 || hour > 23 ||
        minute < 0 || minute > 59 || second < 0 || second > 59)
        return 0;

    /* Проверка существования календарной даты. */
    {
        static const int mdays[] =
            {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int max_day = mdays[month - 1];
        int leap = (year % 400 == 0) ||
                   (year % 4 == 0 && year % 100 != 0);

        if (month == 2 && leap)
            max_day = 29;
        if (day > max_day)
            return 0;
    }

    days = days_from_civil(year, (unsigned)month, (unsigned)day);
    *result = days * 86400LL + hour * 3600LL +
              minute * 60LL + second;

    return 1;
}

static int parse_type(char c, StopType *type)
{
    if (c == 'S' || c == 's')
        *type = STOP_START;
    else if (c == 'I' || c == 'i')
        *type = STOP_INTERMEDIATE;
    else if (c == 'F' || c == 'f')
        *type = STOP_FINISH;
    else
        return 0;

    return 1;
}

static int valid_coordinates(Coordinates p)
{
    return isfinite(p.latitude) && isfinite(p.longitude) &&
           p.latitude >= -90.0 && p.latitude <= 90.0 &&
           p.longitude >= -180.0 && p.longitude <= 180.0;
}

static int append_record(Record **records, size_t *count,
                         size_t *capacity, const Record *record)
{
    Record *tmp;
    size_t new_capacity;

    if (*count == *capacity) {
        new_capacity = (*capacity == 0) ? 32 : *capacity * 2;

        if (new_capacity < *capacity ||
            new_capacity > SIZE_MAX / sizeof(Record))
            return 0;

        tmp = (Record *)realloc(*records,
                                new_capacity * sizeof(Record));
        if (tmp == NULL)
            return 0;

        *records = tmp;
        *capacity = new_capacity;
    }

    (*records)[(*count)++] = *record;
    return 1;
}

static int read_one_file(const char *filename, Record **records,
                         size_t *count, size_t *capacity)
{
    FILE *file;
    char line[LINE_SIZE];
    Coordinates point;
    int have_coordinates = 0;
    int line_number = 0;

    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        Record rec;
        char id[ID_SIZE], date1[32], time1[32], date2[32], time2[32];
        char type_char, extra;
        int fields;

        ++line_number;

        if (strchr(line, '\n') == NULL && !feof(file)) {
            int ch;
            while ((ch = fgetc(file)) != '\n' && ch != EOF) {}
            fprintf(stderr, "%s:%d: line too long\n",
                    filename, line_number);
            fclose(file);
            return 0;
        }

        if (line[0] == '\n' || line[0] == '\r' ||
            line[0] == '#' || line[0] == '\0')
            continue;

        if (!have_coordinates) {
            fields = sscanf(line, "%lf %lf %c",
                            &point.latitude, &point.longitude, &extra);
            if (fields < 2 || !valid_coordinates(point)) {
                fprintf(stderr, "%s:%d: invalid coordinates\n",
                        filename, line_number);
                fclose(file);
                return 0;
            }
            have_coordinates = 1;
            continue;
        }

        fields = sscanf(line, "%127s %31s %31s %31s %31s %c %c",
                        id, date1, time1, date2, time2,
                        &type_char, &extra);

        if (fields != 6 || !parse_type(type_char, &rec.type) ||
            !parse_datetime(date1, time1, &rec.arrival) ||
            !parse_datetime(date2, time2, &rec.departure) ||
            rec.departure < rec.arrival) {
            fprintf(stderr, "%s:%d: invalid stop record\n",
                    filename, line_number);
            fclose(file);
            return 0;
        }

        if (rec.type == STOP_START &&
            rec.arrival != rec.departure) {
            /* Начальная стоянка допускается, поэтому не ошибка. */
        }

        strcpy(rec.vehicle_id, id);
        rec.point = point;

        if (!append_record(records, count, capacity, &rec)) {
            fclose(file);
            return 0;
        }
    }

    if (ferror(file)) {
        fprintf(stderr, "Read error: %s\n", filename);
        fclose(file);
        return 0;
    }

    if (fclose(file) != 0) {
        fprintf(stderr, "Close error: %s\n", filename);
        return 0;
    }

    if (!have_coordinates) {
        fprintf(stderr, "%s: missing coordinates\n", filename);
        return 0;
    }

    return 1;
}

static int compare_records(const void *a, const void *b)
{
    const Record *x = (const Record *)a;
    const Record *y = (const Record *)b;
    int cmp = strcmp(x->vehicle_id, y->vehicle_id);

    if (cmp != 0)
        return cmp;

    if (x->arrival < y->arrival)
        return -1;
    if (x->arrival > y->arrival)
        return 1;

    if (x->departure < y->departure)
        return -1;
    if (x->departure > y->departure)
        return 1;

    return (int)x->type - (int)y->type;
}

static void free_stops(Stop *stop)
{
    while (stop != NULL) {
        Stop *next = stop->next;
        free(stop);
        stop = next;
    }
}

static void free_storage(Storage *storage)
{
    Route *route = storage->head;

    while (route != NULL) {
        Route *next = route->next;
        free_stops(route->stops);
        free(route->vehicle_id);
        free(route);
        route = next;
    }

    storage->head = NULL;
    storage->tail = NULL;
}

static Route *create_route(const char *vehicle_id)
{
    Route *route = (Route *)malloc(sizeof(Route));

    if (route == NULL)
        return NULL;

    route->vehicle_id = copy_string(vehicle_id);
    if (route->vehicle_id == NULL) {
        free(route);
        return NULL;
    }

    route->stops = NULL;
    route->last_stop = NULL;
    route->next = NULL;

    return route;
}

static int append_stop(Route *route, const Record *record)
{
    Stop *stop = (Stop *)malloc(sizeof(Stop));

    if (stop == NULL)
        return 0;

    stop->point = record->point;
    stop->arrival = record->arrival;
    stop->departure = record->departure;
    stop->type = record->type;
    stop->next = NULL;

    if (route->stops == NULL)
        route->stops = stop;
    else
        route->last_stop->next = stop;

    route->last_stop = stop;
    return 1;
}

static int append_route(Storage *storage, Route *route)
{
    if (storage == NULL || route == NULL)
        return 0;

    if (storage->head == NULL)
        storage->head = route;
    else
        storage->tail->next = route;

    storage->tail = route;
    return 1;
}

/*
 * Записи сначала сортируются по vehicle_id и времени.
 * Для каждого ТС последовательность S ... F образует один маршрут.
 */
static int build_routes(Record *records, size_t count, Storage *storage)
{
    size_t i = 0;
    Route *current = NULL;
    char current_id[ID_SIZE] = "";

    qsort(records, count, sizeof(Record), compare_records);

    while (i < count) {
        const Record *rec = &records[i];

        if (strcmp(current_id, rec->vehicle_id) != 0) {
            if (current != NULL) {
                free_stops(current->stops);
                free(current->vehicle_id);
                free(current);
                return 0; /* Предыдущий маршрут не завершён. */
            }
            strcpy(current_id, rec->vehicle_id);
        }

        if (rec->type == STOP_START) {
            if (current != NULL)
                return 0; /* Новый маршрут начался до завершения старого. */

            current = create_route(rec->vehicle_id);
            if (current == NULL)
                return 0;
        } else if (current == NULL) {
            return 0; /* Нет начальной остановки. */
        }

        if (!append_stop(current, rec)) {
            if (current != NULL) {
                free_stops(current->stops);
                free(current->vehicle_id);
                free(current);
            }
            return 0;
        }

        if (rec->type == STOP_FINISH) {
            Route *completed = current;
            current = NULL;
            current_id[0] = '\0';

            if (!append_route(storage, completed)) {
                free_stops(completed->stops);
                free(completed->vehicle_id);
                free(completed);
                return 0;
            }
        }

        ++i;
    }

    if (current != NULL) {
        free_stops(current->stops);
        free(current->vehicle_id);
        free(current);
        return 0;
    }

    return 1;
}

static double to_radians(double degrees)
{
    return degrees * PI / 180.0;
}

static double distance_km(Coordinates a, Coordinates b)
{
    double lat1 = to_radians(a.latitude);
    double lat2 = to_radians(b.latitude);
    double dlat = lat2 - lat1;
    double dlon = to_radians(b.longitude - a.longitude);
    double h = sin(dlat / 2.0) * sin(dlat / 2.0) +
               cos(lat1) * cos(lat2) *
               sin(dlon / 2.0) * sin(dlon / 2.0);

    if (h > 1.0)
        h = 1.0;

    return 2.0 * EARTH_RADIUS_KM * asin(sqrt(h));
}

static double route_distance(const Route *route)
{
    const Stop *stop = route->stops;
    double total = 0.0;

    while (stop != NULL && stop->next != NULL) {
        total += distance_km(stop->point, stop->next->point);
        stop = stop->next;
    }

    return total;
}

static long long route_duration(const Route *route)
{
    const Stop *first = route->stops;
    const Stop *last = route->last_stop;

    if (first == NULL || last == NULL)
        return 0;

    return last->arrival - first->departure;
}

static long long longest_single_stop(const Route *route)
{
    const Stop *stop;
    long long best = 0;

    for (stop = route->stops; stop != NULL; stop = stop->next) {
        long long duration = stop->departure - stop->arrival;
        if (duration > best)
            best = duration;
    }

    return best;
}

static long long shortest_single_stop(const Route *route)
{
    const Stop *stop;
    long long best = LLONG_MAX;

    for (stop = route->stops; stop != NULL; stop = stop->next) {
        long long duration = stop->departure - stop->arrival;
        if (duration < best)
            best = duration;
    }

    return best == LLONG_MAX ? 0 : best;
}

static long long total_idle_time(const Route *route)
{
    const Stop *stop;
    long long total = 0;

    for (stop = route->stops; stop != NULL; stop = stop->next)
        total += stop->departure - stop->arrival;

    return total;
}

static void print_duration(long long seconds)
{
    long long days = seconds / 86400;
    long long hours = (seconds % 86400) / 3600;
    long long minutes = (seconds % 3600) / 60;
    long long secs = seconds % 60;

    printf("%lld d %02lld:%02lld:%02lld",
           days, hours, minutes, secs);
}

static size_t count_vehicle_routes(const Storage *storage,
                                   const char *vehicle_id)
{
    const Route *route;
    size_t count = 0;

    for (route = storage->head; route != NULL; route = route->next) {
        if (strcmp(route->vehicle_id, vehicle_id) == 0)
            ++count;
    }

    return count;
}

static int already_seen(const Storage *storage, const Route *current)
{
    const Route *route;

    for (route = storage->head; route != current; route = route->next) {
        if (strcmp(route->vehicle_id, current->vehicle_id) == 0)
            return 1;
    }

    return 0;
}

static void search_route_counts(const Storage *storage, int want_max)
{
    const Route *route;
    size_t best = want_max ? 0 : SIZE_MAX;
    int found = 0;

    for (route = storage->head; route != NULL; route = route->next) {
        size_t count;

        if (already_seen(storage, route))
            continue;

        count = count_vehicle_routes(storage, route->vehicle_id);

        if ((!want_max && count < best) || (!found) ||
            (want_max && count > best)) {
            best = count;
            found = 1;
        }
    }

    if (!found) {
        puts("No routes.");
        return;
    }

    printf("%s number of routes (%zu):\n",
           want_max ? "Maximum" : "Minimum", best);

    for (route = storage->head; route != NULL; route = route->next) {
        if (!already_seen(storage, route) &&
            count_vehicle_routes(storage, route->vehicle_id) == best)
            printf("  %s\n", route->vehicle_id);
    }
}

static void search_route_metric(const Storage *storage, int metric)
{
    const Route *route, *best_route = NULL;
    double best_double = 0.0;
    long long best_time = 0;
    int first = 1;

    for (route = storage->head; route != NULL; route = route->next) {
        double value_double = 0.0;
        long long value_time = 0;

        if (metric == 0 || metric == 1)
            value_double = route_distance(route);
        else if (metric == 2 || metric == 3)
            value_time = route_duration(route);
        else if (metric == 4)
            value_time = longest_single_stop(route);
        else if (metric == 5)
            value_time = shortest_single_stop(route);
        else
            value_time = total_idle_time(route);

        if (first) {
            best_route = route;
            best_double = value_double;
            best_time = value_time;
            first = 0;
        } else if (metric == 0 || metric == 2 || metric == 4 || metric == 6) {
            if ((metric == 0 && value_double > best_double) ||
                (metric != 0 && value_time > best_time)) {
                best_route = route;
                best_double = value_double;
                best_time = value_time;
            }
        } else {
            if ((metric == 1 && value_double < best_double) ||
                (metric != 1 && value_time < best_time)) {
                best_route = route;
                best_double = value_double;
                best_time = value_time;
            }
        }
    }

    if (best_route == NULL) {
        puts("No routes.");
        return;
    }

    switch (metric) {
        case 0:
        case 1:
            printf("Vehicle %s, distance: %.3f km\n",
                   best_route->vehicle_id, best_double);
            break;
        case 2:
        case 3:
            printf("Vehicle %s, route duration: ",
                   best_route->vehicle_id);
            print_duration(best_time);
            putchar('\n');
            break;
        case 4:
        case 5:
            printf("Vehicle %s, stop duration: ",
                   best_route->vehicle_id);
            print_duration(best_time);
            putchar('\n');
            break;
        case 6:
            printf("Vehicle %s, total idle time: ",
                   best_route->vehicle_id);
            print_duration(best_time);
            putchar('\n');
            break;
    }
}

static void print_menu(void)
{
    puts("\n--- TRANSPORT STATISTICS ---");
    puts("1. Vehicle(s) with most routes");
    puts("2. Vehicle(s) with fewest routes");
    puts("3. Vehicle with longest total route distance");
    puts("4. Vehicle with shortest total route distance");
    puts("5. Vehicle with longest route duration");
    puts("6. Vehicle with shortest route duration");
    puts("7. Vehicle with longest single stop");
    puts("8. Vehicle with shortest single stop");
    puts("9. Vehicle with greatest total idle time");
    puts("0. Exit");
    printf("Choose: ");
}

static void run_menu(const Storage *storage)
{
    char line[64];
    int choice;

    for (;;) {
        char *end;
        long value;

        print_menu();

        if (fgets(line, sizeof(line), stdin) == NULL)
            break;

        value = strtol(line, &end, 10);
        while (isspace((unsigned char)*end))
            ++end;

        if (end == line || *end != '\0' || value < 0 || value > 9) {
            puts("Invalid choice.");
            continue;
        }

        choice = (int)value;

        if (choice == 0)
            break;

        switch (choice) {
            case 1: search_route_counts(storage, 1); break;
            case 2: search_route_counts(storage, 0); break;
            case 3: search_route_metric(storage, 0); break;
            case 4: search_route_metric(storage, 1); break;
            case 5: search_route_metric(storage, 2); break;
            case 6: search_route_metric(storage, 3); break;
            case 7: search_route_metric(storage, 4); break;
            case 8: search_route_metric(storage, 5); break;
            case 9: search_route_metric(storage, 6); break;
            default: puts("Invalid choice."); break;
        }
    }
}

int main(int argc, char **argv)
{
    Record *records = NULL;
    size_t count = 0, capacity = 0;
    Storage storage;
    int i;
    int success = 1;

    init_storage(&storage);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s file1 [file2 ...]\n", argv[0]);
        return 1;
    }

    for (i = 1; i < argc; ++i) {
        if (!read_one_file(argv[i], &records, &count, &capacity)) {
            success = 0;
            break;
        }
    }

    if (success && count == 0) {
        puts("Input files contain no stop records.");
        success = 0;
    }

    if (success && !build_routes(records, count, &storage)) {
        fprintf(stderr,
                "Cannot reconstruct routes: invalid stop sequence "
                "or memory allocation failure.\n");
        success = 0;
    }

    free(records);

    if (success)
        run_menu(&storage);

    free_storage(&storage);

    return success ? 0 : 1;
}
