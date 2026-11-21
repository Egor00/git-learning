#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "myerrors.h"

/* ======================= УЗЕЛ ДЕРЕВА ======================= */

typedef struct Node
{
    char *name;
    struct Node **children;
    size_t nchildren;
    size_t cap;
} Node;

static Node *node_create(const char *name, size_t len)
{
    if (!name) return NULL;
    Node *n = (Node *)malloc(sizeof(Node));
    if (!n) return NULL;
    n->name = (char *)malloc(len + 1);
    if (!n->name) { free(n); return NULL; }
    memcpy(n->name, name, len);
    n->name[len] = '\0';
    n->children = NULL;
    n->nchildren = 0;
    n->cap = 0;
    return n;
}

static void node_free(Node *n)
{
    if (!n) return;
    for (size_t i = 0; i < n->nchildren; ++i)
        node_free(n->children[i]);
    free(n->children);
    free(n->name);
    free(n);
}

static ErrorsDef node_add_child(Node *parent, Node *child)
{
    if (!parent || !child) return INVALID_ARGUMENT;
    if (parent->nchildren >= parent->cap)
    {
        size_t ncap = parent->cap ? parent->cap * 2 : 4;
        Node **nc = (Node **)realloc(parent->children, ncap * sizeof(Node *));
        if (!nc) return REALLOC_ERROR;
        parent->children = nc;
        parent->cap = ncap;
    }
    parent->children[parent->nchildren++] = child;
    return STAT_OK;
}

/* ======================= ПАРСЕР ======================= */

/*
  Грамматика:
    expr    := node
    node    := NAME [ '(' list ')' ]
    list    := node (',' node)*
    NAME    := последовательность символов, кроме ' ', '\t', '(', ')', ','
*/

typedef struct
{
    const char *p;
} Parser;

static void skip_ws(Parser *ps)
{
    while (*ps->p == ' ' || *ps->p == '\t' || *ps->p == '\r' || *ps->p == '\n')
        ++ps->p;
}

static int is_name_char(char c)
{
    return c != '\0' && c != ' ' && c != '\t' && c != '\r' && c != '\n'
        && c != '(' && c != ')' && c != ',';
}

static ErrorsDef parse_node(Parser *ps, Node **out);

static ErrorsDef parse_list(Parser *ps, Node *parent)
{
    /* вызывается сразу после '('; ожидается node (',' node)* ')' */
    for (;;)
    {
        skip_ws(ps);
        Node *child = NULL;
        ErrorsDef e = parse_node(ps, &child);
        if (e != STAT_OK) return e;

        e = node_add_child(parent, child);
        if (e != STAT_OK) { node_free(child); return e; }

        skip_ws(ps);
        if (*ps->p == ',') { ++ps->p; continue; }
        if (*ps->p == ')') { ++ps->p; return STAT_OK; }
        return INVALID_NUMBER_FORMAT; /* нет ни ',', ни ')' */
    }
}

static ErrorsDef parse_node(Parser *ps, Node **out)
{
    if (!ps || !out) return INVALID_ARGUMENT;
    *out = NULL;

    skip_ws(ps);
    const char *start = ps->p;
    while (is_name_char(*ps->p)) ++ps->p;
    if (ps->p == start) return EMPTY_STRING;

    Node *n = node_create(start, (size_t)(ps->p - start));
    if (!n) return MEMORY_ERROR;

    skip_ws(ps);
    if (*ps->p == '(')
    {
        ++ps->p;
        ErrorsDef e = parse_list(ps, n);
        if (e != STAT_OK) { node_free(n); return e; }
    }
    *out = n;
    return STAT_OK;
}

/* Парсит всю строку; после выражения допускается только конец строки. */
static ErrorsDef parse_expression(const char *line, Node **out)
{
    if (!line || !out) return INVALID_ARGUMENT;
    *out = NULL;

    Parser ps = { line };
    ErrorsDef e = parse_node(&ps, out);
    if (e != STAT_OK) return e;

    skip_ws(&ps);
    if (*ps.p != '\0')
    {
        node_free(*out);
        *out = NULL;
        return INVALID_NUMBER_FORMAT;
    }
    return STAT_OK;
}

/* ======================= ВЫВОД ДЕРЕВА ======================= */

/*
  Отступный стиль:
    A
    ├── B
    │   ├── E
    │   └── F
    └── C
*/

static ErrorsDef print_tree_rec(const Node *n,
                                const char *prefix,
                                int is_last,
                                int is_root,
                                FILE *f)
{
    if (!n || !f) return INVALID_ARGUMENT;

    if (is_root)
    {
        if (fprintf(f, "%s\n", n->name) < 0) return WRITEFILE_ERROR;
    }
    else
    {
        if (fprintf(f, "%s%s%s\n",
                    prefix,
                    is_last ? "└── " : "├── ",
                    n->name) < 0)
            return WRITEFILE_ERROR;
    }

    /* новый префикс для детей */
    char *new_prefix = NULL;
    size_t plen = strlen(prefix);
    size_t extra = is_root ? 0 : 4;   /* "│   " или "    " */
    new_prefix = (char *)malloc(plen + extra + 1);
    if (!new_prefix) return MEMORY_ERROR;

    memcpy(new_prefix, prefix, plen);
    if (!is_root)
    {
        if (is_last) memcpy(new_prefix + plen, "    ", 4);
        else         memcpy(new_prefix + plen, "│   ", 4);
        new_prefix[plen + 4] = '\0';
    }
    else
    {
        new_prefix[plen] = '\0';
    }

    for (size_t i = 0; i < n->nchildren; ++i)
    {
        int last = (i + 1 == n->nchildren);
        ErrorsDef e = print_tree_rec(n->children[i], new_prefix, last, 0, f);
        if (e != STAT_OK) { free(new_prefix); return e; }
    }

    free(new_prefix);
    return STAT_OK;
}

static ErrorsDef print_tree(const Node *root, FILE *f)
{
    if (!root || !f) return INVALID_ARGUMENT;
    return print_tree_rec(root, "", 0, 1, f);
}

/* ======================= MAIN ======================= */

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        errors(MATCH_ARGS);
        return 1;
    }

    FILE *fin = fopen(argv[1], "r");
    if (!fin) { errors(OPENFILE_ERROR); return 1; }

    FILE *fout = fopen(argv[2], "w");
    if (!fout) { errors(OPENFILE_ERROR); fclose(fin); return 1; }

    char line[8192];
    size_t lineno = 0;
    ErrorsDef err = STAT_OK;

    while (fgets(line, (int)sizeof(line), fin))
    {
        ++lineno;
        size_t L = strlen(line);
        while (L > 0 && (line[L-1] == '\n' || line[L-1] == '\r'))
            line[--L] = '\0';

        /* пропускаем пустые строки */
        const char *q = line;
        while (*q == ' ' || *q == '\t') ++q;
        if (*q == '\0') continue;

        Node *root = NULL;
        ErrorsDef e = parse_expression(line, &root);
        if (e != STAT_OK)
        {
            fprintf(fout, "Строка %zu: ошибка разбора (%d)\n\n", lineno, (int)e);
            continue; /* продолжаем обработку остальных строк */
        }

        if (fprintf(fout, "Выражение %zu:\n", lineno) < 0) { err = WRITEFILE_ERROR; node_free(root); break; }
        e = print_tree(root, fout);
        node_free(root);
        if (e != STAT_OK) { err = e; break; }
        if (fprintf(fout, "\n") < 0) { err = WRITEFILE_ERROR; break; }
    }

    if (ferror(fin) && err == STAT_OK) err = READFILE_ERROR;

    if (fclose(fin) != 0 && err == STAT_OK) err = CLOSEFILE_ERROR;
    if (fclose(fout) != 0 && err == STAT_OK) err = CLOSEFILE_ERROR;

    if (err != STAT_OK) { errors(err); return 1; }
    return 0;
}