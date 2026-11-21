#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "myerrors.h"

/* ======================= СТРУКТУРЫ ======================= */

typedef struct Term
{
    long long coef;
    int deg;
    struct Term *next;
} Term;

typedef Term *Poly;   /* Poly == NULL — нулевой многочлен */

/* ======================= БАЗОВЫЕ ОПЕРАЦИИ ======================= */

static void poly_free(Poly p)
{
    while (p)
    {
        Term *n = p->next;
        free(p);
        p = n;
    }
}

static Poly poly_copy(const Poly p)
{
    Poly head = NULL, tail = NULL;
    for (const Term *t = p; t; t = t->next)
    {
        Term *n = (Term *)malloc(sizeof(Term));
        if (!n) { poly_free(head); return NULL; }
        n->coef = t->coef;
        n->deg = t->deg;
        n->next = NULL;
        if (!head) head = tail = n;
        else { tail->next = n; tail = n; }
    }
    return head;
}

/* Вставка/обновление терма. Список отсортирован по убыванию deg. */
static ErrorsDef poly_insert(Poly *head, long long coef, int deg)
{
    if (!head) return INVALID_ARGUMENT;
    if (coef == 0) return STAT_OK;

    Poly prev = NULL, cur = *head;
    while (cur && cur->deg > deg) { prev = cur; cur = cur->next; }

    if (cur && cur->deg == deg)
    {
        cur->coef += coef;
        if (cur->coef == 0)
        {
            if (prev) prev->next = cur->next;
            else *head = cur->next;
            free(cur);
        }
        return STAT_OK;
    }

    Term *n = (Term *)malloc(sizeof(Term));
    if (!n) return MEMORY_ERROR;
    n->coef = coef;
    n->deg = deg;
    n->next = cur;
    if (prev) prev->next = n;
    else *head = n;
    return STAT_OK;
}

/* ======================= АРИФМЕТИКА ======================= */

static ErrorsDef poly_add(const Poly a, const Poly b, Poly *out)
{
    if (!out) return INVALID_ARGUMENT;
    *out = NULL;
    for (const Term *t = a; t; t = t->next)
    {
        ErrorsDef e = poly_insert(out, t->coef, t->deg);
        if (e != STAT_OK) { poly_free(*out); *out = NULL; return e; }
    }
    for (const Term *t = b; t; t = t->next)
    {
        ErrorsDef e = poly_insert(out, t->coef, t->deg);
        if (e != STAT_OK) { poly_free(*out); *out = NULL; return e; }
    }
    return STAT_OK;
}

static ErrorsDef poly_sub(const Poly a, const Poly b, Poly *out)
{
    if (!out) return INVALID_ARGUMENT;
    *out = NULL;
    for (const Term *t = a; t; t = t->next)
    {
        ErrorsDef e = poly_insert(out, t->coef, t->deg);
        if (e != STAT_OK) { poly_free(*out); *out = NULL; return e; }
    }
    for (const Term *t = b; t; t = t->next)
    {
        ErrorsDef e = poly_insert(out, -t->coef, t->deg);
        if (e != STAT_OK) { poly_free(*out); *out = NULL; return e; }
    }
    return STAT_OK;
}

static ErrorsDef poly_mul(const Poly a, const Poly b, Poly *out)
{
    if (!out) return INVALID_ARGUMENT;
    *out = NULL;
    for (const Term *ta = a; ta; ta = ta->next)
    {
        for (const Term *tb = b; tb; tb = tb->next)
        {
            long long c = ta->coef * tb->coef;
            /* проверка переполнения коэффициента */
            if (tb->coef != 0 && c / tb->coef != ta->coef)
            {
                poly_free(*out); *out = NULL;
                return FUNC_OVERFLOW;
            }
            long long d = (long long)ta->deg + tb->deg;
            if (d > INT_MAX)
            {
                poly_free(*out); *out = NULL;
                return POWER_OVERFLOW;
            }
            ErrorsDef e = poly_insert(out, c, (int)d);
            if (e != STAT_OK) { poly_free(*out); *out = NULL; return e; }
        }
    }
    return STAT_OK;
}

/* Деление с остатком: a = q*b + r, deg(r) < deg(b) */
static ErrorsDef poly_divmod(const Poly a, const Poly b, Poly *q, Poly *r)
{
    if (!q || !r) return INVALID_ARGUMENT;
    *q = NULL; *r = NULL;

    if (b == NULL) return ZERO_DIV;

    ErrorsDef e = poly_copy(a) ? STAT_OK : MEMORY_ERROR;
    Poly rem = poly_copy(a);
    if (a && !rem) return MEMORY_ERROR;

    Poly quot = NULL;
    int b_deg = b->deg;
    long long b_lead = b->coef;

    while (rem && rem->deg >= b_deg)
    {
        if (b_lead == 0) { poly_free(rem); poly_free(quot); return ZERO_DIV; }

        long long c = rem->coef / b_lead;
        int d = rem->deg - b_deg;
        if (c == 0) break;

        e = poly_insert(&quot, c, d);
        if (e != STAT_OK) { poly_free(rem); poly_free(quot); return e; }

        /* rem -= c * x^d * b */
        Poly tmp = NULL;
        e = poly_mul(b, &(Poly){0}, &tmp); /* заглушка, не используется */
        (void)e;

        /* вычисляем c*x^d*b вручную */
        Poly prod = NULL;
        for (const Term *t = b; t; t = t->next)
        {
            long long cc = t->coef * c;
            if (t->coef != 0 && cc / t->coef != c)
            {
                poly_free(rem); poly_free(quot); poly_free(prod);
                return FUNC_OVERFLOW;
            }
            int dd = t->deg + d;
            ErrorsDef ee = poly_insert(&prod, cc, dd);
            if (ee != STAT_OK)
            {
                poly_free(rem); poly_free(quot); poly_free(prod);
                return ee;
            }
        }

        Poly newrem = NULL;
        e = poly_sub(rem, prod, &newrem);
        poly_free(prod);
        if (e != STAT_OK) { poly_free(rem); poly_free(quot); return e; }
        poly_free(rem);
        rem = newrem;
    }

    *q = quot;
    *r = rem;
    return STAT_OK;
}

/* Вычисление в точке x. Результат — long long, с проверкой переполнения. */
static ErrorsDef poly_eval(const Poly p, long long x, long long *out)
{
    if (!out) return INVALID_ARGUMENT;
    *out = 0;
    for (const Term *t = p; t; t = t->next)
    {
        long long val = t->coef;
        for (int i = 0; i < t->deg; ++i)
        {
            if (x != 0 && val > LLONG_MAX / (x > 0 ? x : -x) && x < 0)
                return FUNC_OVERFLOW;
            if (x != 0 && val > LLONG_MAX / x && x > 0)
                return FUNC_OVERFLOW;
            val *= x;
        }
        if ((val > 0 && *out > LLONG_MAX - val) ||
            (val < 0 && *out < LLONG_MIN - val))
            return FUNC_OVERFLOW;
        *out += val;
    }
    return STAT_OK;
}

static ErrorsDef poly_diff(const Poly p, Poly *out)
{
    if (!out) return INVALID_ARGUMENT;
    *out = NULL;
    for (const Term *t = p; t; t = t->next)
    {
        if (t->deg == 0) continue;
        long long c = t->coef * t->deg;
        if (t->coef != 0 && c / t->coef != t->deg)
        {
            poly_free(*out); *out = NULL;
            return FUNC_OVERFLOW;
        }
        ErrorsDef e = poly_insert(out, c, t->deg - 1);
        if (e != STAT_OK) { poly_free(*out); *out = NULL; return e; }
    }
    return STAT_OK;
}

/* Композиция p(q(x)) */
static ErrorsDef poly_compose(const Poly p, const Poly q, Poly *out)
{
    if (!out) return INVALID_ARGUMENT;
    *out = NULL;

    Poly result = NULL;
    Poly qpow = NULL;
    /* q^0 = 1 */
    ErrorsDef e = poly_insert(&qpow, 1, 0);
    if (e != STAT_OK) return e;

    int max_deg = p ? p->deg : -1;
    for (int k = 0; k <= max_deg; ++k)
    {
        /* найти коэффициент при x^k в p */
        long long ck = 0;
        for (const Term *t = p; t; t = t->next)
            if (t->deg == k) { ck = t->coef; break; }

        if (ck != 0)
        {
            Poly scaled = NULL;
            for (const Term *t = qpow; t; t = t->next)
            {
                long long c = t->coef * ck;
                if (t->coef != 0 && c / t->coef != ck)
                {
                    poly_free(qpow); poly_free(result); poly_free(scaled);
                    return FUNC_OVERFLOW;
                }
                e = poly_insert(&scaled, c, t->deg);
                if (e != STAT_OK)
                {
                    poly_free(qpow); poly_free(result); poly_free(scaled);
                    return e;
                }
            }
            Poly nr = NULL;
            e = poly_add(result, scaled, &nr);
            poly_free(scaled);
            if (e != STAT_OK) { poly_free(qpow); poly_free(result); return e; }
            poly_free(result);
            result = nr;
        }

        if (k < max_deg)
        {
            Poly nq = NULL;
            e = poly_mul(qpow, q, &nq);
            if (e != STAT_OK) { poly_free(qpow); poly_free(result); return e; }
            poly_free(qpow);
            qpow = nq;
        }
    }
    poly_free(qpow);
    *out = result;
    return STAT_OK;
}

/* ======================= ВЫВОД ======================= */

static void poly_print(const Poly p)
{
    if (!p) { putchar('0'); return; }

    int first = 1;
    for (const Term *t = p; t; t = t->next)
    {
        long long c = t->coef;
        int d = t->deg;

        if (!first)
        {
            if (c > 0) putchar('+');
        }
        first = 0;

        if (d == 0)
        {
            printf("%lld", c);
        }
        else
        {
            if (c == 1) { /* ничего */ }
            else if (c == -1) putchar('-');
            else printf("%lld", c);

            putchar('x');
            if (d > 1) printf("^%d", d);
        }
    }
}

/* ======================= ПАРСЕР МНОГОЧЛЕНА ======================= */

/*
  Формат: непустая строка вида
   [±]term(±term)*
   term: coef [x [^ deg]]  |  x [^ deg]
   coef: целое число (может быть опущено, если есть x)
   Примеры: 2x^2-x+2, -x^2+3x-1, 4x-8, 1, x, -x, 0
*/

static ErrorsDef parse_poly(const char *s, Poly *out)
{
    if (!s || !out) return INVALID_ARGUMENT;
    *out = NULL;

    const char *p = s;
    while (*p == ' ' || *p == '\t') ++p;

    Poly head = NULL;
    int sign = 1;

    if (*p == '+') { ++p; }
    else if (*p == '-') { sign = -1; ++p; }

    while (*p)
    {
        while (*p == ' ' || *p == '\t') ++p;
        if (!*p) break;

        long long coef = 0;
        int has_coef = 0;

        /* знак */
        if (*p == '+') { sign = 1; ++p; }
        else if (*p == '-') { sign = -1; ++p; }

        /* цифры коэффициента */
        while (isdigit((unsigned char)*p))
        {
            has_coef = 1;
            int digit = *p - '0';
            if (coef > (LLONG_MAX - digit) / 10)
            {
                poly_free(head);
                return FUNC_OVERFLOW;
            }
            coef = coef * 10 + digit;
            ++p;
        }

        int deg = 0;

        if (*p == 'x' || *p == 'X')
        {
            ++p;
            if (*p == '^')
            {
                ++p;
                int d = 0;
                int got = 0;
                while (isdigit((unsigned char)*p))
                {
                    got = 1;
                    int digit = *p - '0';
                    if (d > (INT_MAX - digit) / 10)
                    {
                        poly_free(head);
                        return POWER_OVERFLOW;
                    }
                    d = d * 10 + digit;
                    ++p;
                }
                if (!got)
                {
                    poly_free(head);
                    return INVALID_NUMBER_FORMAT;
                }
                deg = d;
            }
            else
            {
                deg = 1;
            }
            if (!has_coef) coef = 1;
        }
        else
        {
            if (!has_coef)
            {
                poly_free(head);
                return INVALID_NUMBER_FORMAT;
            }
            deg = 0;
        }

        ErrorsDef e = poly_insert(&head, sign * coef, deg);
        if (e != STAT_OK) { poly_free(head); return e; }
        sign = 1;

        while (*p == ' ' || *p == '\t') ++p;
        if (*p && *p != '+' && *p != '-')
        {
            poly_free(head);
            return INVALID_NUMBER_FORMAT;
        }
    }

    *out = head;
    return STAT_OK;
}

/* ======================= РАЗБОР ИНСТРУКЦИИ ======================= */

/*
  Строка инструкции: Name(arg1,arg2,...);
  Возвращает имя и массив аргументов (строк). Память под аргументы
  выделяется динамически; вызывающий освобождает.
*/

typedef struct
{
    char *name;
    char **args;
    int argc;
} Instr;

static void instr_free(Instr *ins)
{
    if (!ins) return;
    free(ins->name);
    if (ins->args)
    {
        for (int i = 0; i < ins->argc; ++i) free(ins->args[i]);
        free(ins->args);
    }
    ins->name = NULL;
    ins->args = NULL;
    ins->argc = 0;
}

static ErrorsDef parse_instr(const char *line, Instr *ins)
{
    if (!line || !ins) return INVALID_ARGUMENT;
    ins->name = NULL; ins->args = NULL; ins->argc = 0;

    const char *p = line;
    while (*p == ' ' || *p == '\t') ++p;

    /* имя */
    const char *ns = p;
    while (isalpha((unsigned char)*p)) ++p;
    if (p == ns) return INVALID_NUMBER_FORMAT;

    size_t nlen = (size_t)(p - ns);
    ins->name = (char *)malloc(nlen + 1);
    if (!ins->name) return MEMORY_ERROR;
    memcpy(ins->name, ns, nlen);
    ins->name[nlen] = '\0';

    while (*p == ' ' || *p == '\t') ++p;
    if (*p != '(') { instr_free(ins); return INVALID_NUMBER_FORMAT; }
    ++p;

    /* аргументы: разбиваем по запятым верхнего уровня.
       Многочлены сами по себе не содержат запятых, но на всякий случай
       отслеживаем глубину скобок. */
    int depth = 0;
    const char *start = p;
    const char *q = p;
    for (;;)
    {
        if (*q == '(') ++depth;
        else if (*q == ')')
        {
            if (depth == 0)
            {
                /* конец списка аргументов */
                if (q > start)
                {
                    size_t len = (size_t)(q - start);
                    char *arg = (char *)malloc(len + 1);
                    if (!arg) { instr_free(ins); return MEMORY_ERROR; }
                    memcpy(arg, start, len);
                    arg[len] = '\0';
                    char **na = (char **)realloc(ins->args,
                                                 (size_t)(ins->argc + 1) * sizeof(char *));
                    if (!na) { free(arg); instr_free(ins); return REALLOC_ERROR; }
                    ins->args = na;
                    ins->args[ins->argc++] = arg;
                }
                ++q;
                break;
            }
            --depth;
        }
        else if (*q == ',' && depth == 0)
        {
            size_t len = (size_t)(q - start);
            char *arg = (char *)malloc(len + 1);
            if (!arg) { instr_free(ins); return MEMORY_ERROR; }
            memcpy(arg, start, len);
            arg[len] = '\0';
            char **na = (char **)realloc(ins->args,
                                         (size_t)(ins->argc + 1) * sizeof(char *));
            if (!na) { free(arg); instr_free(ins); return REALLOC_ERROR; }
            ins->args = na;
            ins->args[ins->argc++] = arg;
            start = q + 1;
        }
        else if (*q == '\0')
        {
            instr_free(ins);
            return INVALID_NUMBER_FORMAT; /* нет закрывающей скобки */
        }
        ++q;
    }

    while (*q == ' ' || *q == '\t') ++q;
    if (*q != ';')
    {
        instr_free(ins);
        return INVALID_NUMBER_FORMAT;
    }

    return STAT_OK;
}

/* ======================= ОБРАБОТКА КОММЕНТАРИЕВ ======================= */

/*
  Читает файл построчно, склеивая многострочные комментарии [ ... ],
  однострочные комментарии % ... до конца строки.
  Накопленные "чистые" строки передаются в process_line.
*/

static ErrorsDef strip_comments_line(char *line, int *in_block, char *clean, size_t cap)
{
    if (!line || !in_block || !clean) return INVALID_ARGUMENT;
    size_t j = 0;
    for (size_t i = 0; line[i] && j + 1 < cap; ++i)
    {
        if (*in_block)
        {
            if (line[i] == ']') *in_block = 0;
            continue;
        }
        if (line[i] == '%') break; /* до конца строки */
        if (line[i] == '[')
        {
            if (strchr(line + i + 1, ']') == NULL && line[i + 1] != '\0')
                *in_block = 1;
            /* если закрывающая скобка в этой же строке — просто пропускаем до неё */
            char *close = strchr(line + i + 1, ']');
            if (close && *in_block == 0)
            {
                /* уже обработано: перепрыгиваем */
                i = (size_t)(close - line);
                continue;
            }
            if (*in_block) continue;
            continue;
        }
        if (line[i] == ']') continue; /* одиночная ] игнорируется */
        clean[j++] = line[i];
    }
    clean[j] = '\0';
    return STAT_OK;
}

/* ======================= ДИСПЕТЧЕР ИНСТРУКЦИЙ ======================= */

static ErrorsDef execute_instr(const Instr *ins, Poly *accum)
{
    if (!ins || !accum) return INVALID_ARGUMENT;

    const char *name = ins->name;
    Poly a = NULL, b = NULL, res = NULL;
    ErrorsDef e;

    /* --- Add / Sub / Mult / Div / Mod / Cmps: 1 или 2 аргумента --- */
    if (strcmp(name, "Add") == 0 || strcmp(name, "Sub") == 0 ||
        strcmp(name, "Mult") == 0 || strcmp(name, "Div") == 0 ||
        strcmp(name, "Mod") == 0 || strcmp(name, "Cmps") == 0)
    {
        if (ins->argc == 1)
        {
            a = poly_copy(*accum);
            if (*accum && !a) return MEMORY_ERROR;
            e = parse_poly(ins->args[0], &b);
            if (e != STAT_OK) { poly_free(a); return e; }
        }
        else if (ins->argc == 2)
        {
            e = parse_poly(ins->args[0], &a);
            if (e != STAT_OK) return e;
            e = parse_poly(ins->args[1], &b);
            if (e != STAT_OK) { poly_free(a); return e; }
        }
        else
        {
            return MATCH_ARGS;
        }

        if (strcmp(name, "Add") == 0)
            e = poly_add(a, b, &res);
        else if (strcmp(name, "Sub") == 0)
            e = poly_sub(a, b, &res);
        else if (strcmp(name, "Mult") == 0)
            e = poly_mul(a, b, &res);
        else if (strcmp(name, "Div") == 0)
        {
            Poly r = NULL;
            e = poly_divmod(a, b, &res, &r);
            poly_free(r);
        }
        else if (strcmp(name, "Mod") == 0)
        {
            Poly q = NULL;
            e = poly_divmod(a, b, &q, &res);
            poly_free(q);
        }
        else /* Cmps */
            e = poly_compose(a, b, &res);

        poly_free(a);
        poly_free(b);
        if (e != STAT_OK) { poly_free(res); return e; }

        poly_free(*accum);
        *accum = res;

        poly_print(*accum);
        putchar('\n');
        return STAT_OK;
    }

    /* --- Eval: 1 или 2 аргумента --- */
    if (strcmp(name, "Eval") == 0)
    {
        Poly target = NULL;
        const char *point_str = NULL;
        if (ins->argc == 1)
        {
            target = poly_copy(*accum);
            if (*accum && !target) return MEMORY_ERROR;
            point_str = ins->args[0];
        }
        else if (ins->argc == 2)
        {
            e = parse_poly(ins->args[0], &target);
            if (e != STAT_OK) return e;
            point_str = ins->args[1];
        }
        else
        {
            return MATCH_ARGS;
        }

        char *end = NULL;
        long long x = strtoll(point_str, &end, 10);
        if (!end || *end != '\0' || end == point_str)
        {
            poly_free(target);
            return INVALID_NUMBER_FORMAT;
        }

        long long val = 0;
        e = poly_eval(target, x, &val);
        poly_free(target);
        if (e != STAT_OK) return e;

        /* Результат — многочлен-константа; записываем в сумматор */
        Poly c = NULL;
        e = poly_insert(&c, val, 0);
        if (e != STAT_OK) return e;

        poly_free(*accum);
        *accum = c;

        printf("%lld\n", val);
        return STAT_OK;
    }

    /* --- Diff: 1 или 2 аргумента --- */
    if (strcmp(name, "Diff") == 0)
    {
        Poly target = NULL;
        if (ins->argc == 1)
        {
            target = poly_copy(*accum);
            if (*accum && !target) return MEMORY_ERROR;
        }
        else if (ins->argc == 2)
        {
            e = parse_poly(ins->args[0], &target);
            if (e != STAT_OK) return e;
        }
        else
        {
            return MATCH_ARGS;
        }

        e = poly_diff(target, &res);
        poly_free(target);
        if (e != STAT_OK) return e;

        poly_free(*accum);
        *accum = res;

        poly_print(*accum);
        putchar('\n');
        return STAT_OK;
    }

    return INVALID_ARGUMENT;
}

/* ======================= MAIN ======================= */

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        errors(MATCH_ARGS);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f)
    {
        errors(OPENFILE_ERROR);
        return 1;
    }

    Poly accum = NULL;
    errors(STAT_OK);

    char line[4096];
    int in_block = 0;
    char clean[4096];

    while (fgets(line, (int)sizeof(line), f))
    {
        /* удалить \n */
        size_t L = strlen(line);
        while (L > 0 && (line[L - 1] == '\n' || line[L - 1] == '\r'))
            line[--L] = '\0';

        ErrorsDef e = strip_comments_line(line, &in_block, clean, sizeof(clean));
        if (e != STAT_OK) { errors(e); poly_free(accum); fclose(f); return 1; }

        /* пропускаем пустые строки */
        char *cp = clean;
        while (*cp == ' ' || *cp == '\t') ++cp;
        if (*cp == '\0') continue;

        Instr ins;
        e = parse_instr(cp, &ins);
        if (e != STAT_OK)
        {
            errors(e);
            /* продолжаем обработку файла, но сообщаем */
            continue;
        }

        e = execute_instr(&ins, &accum);
        instr_free(&ins);

        if (e != STAT_OK)
        {
            errors(e);
            /* не прерываемся: продолжаем обработку следующих инструкций */
        }
    }

    if (ferror(f))
    {
        errors(READFILE_ERROR);
        poly_free(accum);
        fclose(f);
        return 1;
    }

    poly_free(accum);
    if (fclose(f) != 0)
    {
        errors(CLOSEFILE_ERROR);
        return 1;
    }

    return 0;
}