#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "myerrors.h"

/* ======================= УЗЕЛ ДЕРЕВА ======================= */

typedef struct Node
{
    char *word;
    unsigned long long count;
    struct Node *left;
    struct Node *right;
} Node;

static void tree_free(Node *root)
{
    if (!root) return;
    tree_free(root->left);
    tree_free(root->right);
    free(root->word);
    free(root);
}

/* ======================= ВСТАВКА ======================= */

static Node *node_create(const char *word)
{
    if (!word) return NULL;
    size_t len = strlen(word);
    Node *n = (Node *)malloc(sizeof(Node));
    if (!n) return NULL;
    n->word = (char *)malloc(len + 1);
    if (!n->word) { free(n); return NULL; }
    memcpy(n->word, word, len + 1);
    n->count = 1;
    n->left = NULL;
    n->right = NULL;
    return n;
}

static ErrorsDef tree_insert(Node **root, const char *word)
{
    if (!root || !word) return INVALID_ARGUMENT;
    if (*word == '\0') return EMPTY_STRING;

    Node *cur = *root;
    Node *parent = NULL;
    int cmp = 0;
    while (cur)
    {
        parent = cur;
        cmp = strcmp(word, cur->word);
        if (cmp == 0) { cur->count++; return STAT_OK; }
        cur = (cmp < 0) ? cur->left : cur->right;
    }

    Node *n = node_create(word);
    if (!n) return MEMORY_ERROR;

    if (!parent) *root = n;
    else if (cmp < 0) parent->left = n;
    else parent->right = n;
    return STAT_OK;
}

/* ======================= ГЛУБИНА (п. B) ======================= */

static int tree_depth(const Node *root)
{
    if (!root) return 0;
    int l = tree_depth(root->left);
    int r = tree_depth(root->right);
    return 1 + (l > r ? l : r);
}

/* ======================= ЧТЕНИЕ ФАЙЛА ======================= */

static ErrorsDef build_from_file(const char *path,
                                 const unsigned char sep[256],
                                 Node **root)
{
    if (!path || !sep || !root) return INVALID_ARGUMENT;
    *root = NULL;

    FILE *f = fopen(path, "r");
    if (!f) return OPENFILE_ERROR;

    char *buf = NULL;
    size_t cap = 0;
    size_t len = 0;

    int ch;
    ErrorsDef err = STAT_OK;

    while ((ch = fgetc(f)) != EOF)
    {
        unsigned char c = (unsigned char)ch;
        if (sep[c])
        {
            if (len > 0)
            {
                buf[len] = '\0';
                err = tree_insert(root, buf);
                if (err != STAT_OK) break;
                len = 0;
            }
            continue;
        }

        if (len + 1 >= cap)
        {
            size_t ncap = cap ? cap * 2 : 64;
            char *nb = (char *)realloc(buf, ncap);
            if (!nb) { err = REALLOC_ERROR; break; }
            buf = nb;
            cap = ncap;
        }
        buf[len++] = (char)c;
    }

    /* последнее слово без завершающего сепаратора */
    if (err == STAT_OK && len > 0)
    {
        buf[len] = '\0';
        err = tree_insert(root, buf);
    }

    free(buf);

    if (ferror(f) && err == STAT_OK) err = READFILE_ERROR;
    if (fclose(f) != 0 && err == STAT_OK) err = CLOSEFILE_ERROR;

    if (err != STAT_OK) { tree_free(*root); *root = NULL; }
    return err;
}

/* ======================= ПОИСК ЧАСТОТЫ ======================= */

static ErrorsDef tree_lookup(const Node *root, const char *word,
                             unsigned long long *out)
{
    if (!word || !out) return INVALID_ARGUMENT;
    *out = 0;
    while (root)
    {
        int cmp = strcmp(word, root->word);
        if (cmp == 0) { *out = root->count; return STAT_OK; }
        root = (cmp < 0) ? root->left : root->right;
    }
    return STAT_OK; /* не найдено — 0 */
}

/* ======================= СБОР В МАССИВ ======================= */

typedef struct
{
    Node **arr;
    size_t size;
    size_t cap;
} NodeVec;

static ErrorsDef vec_push(NodeVec *v, Node *n)
{
    if (!v || !n) return INVALID_ARGUMENT;
    if (v->size >= v->cap)
    {
        size_t ncap = v->cap ? v->cap * 2 : 64;
        Node **na = (Node **)realloc(v->arr, ncap * sizeof(Node *));
        if (!na) return REALLOC_ERROR;
        v->arr = na;
        v->cap = ncap;
    }
    v->arr[v->size++] = n;
    return STAT_OK;
}

static void tree_collect(Node *root, NodeVec *v, ErrorsDef *err)
{
    if (!root || *err != STAT_OK) return;
    *err = vec_push(v, root);
    if (*err != STAT_OK) return;
    tree_collect(root->left, v, err);
    tree_collect(root->right, v, err);
}

static int cmp_by_count_desc(const void *a, const void *b)
{
    const Node *na = *(const Node *const *)a;
    const Node *nb = *(const Node *const *)b;
    if (na->count < nb->count) return 1;
    if (na->count > nb->count) return -1;
    return strcmp(na->word, nb->word); /* стабильно по слову */
}

/* ======================= САМОЕ ДЛИННОЕ / КОРОТКОЕ ======================= */

static void tree_longest_shortest(const Node *root,
                                  const Node **longest,
                                  const Node **shortest)
{
    if (!root) return;
    size_t l = strlen(root->word);
    if (*longest == NULL || l > strlen((*longest)->word)) *longest = root;
    if (*shortest == NULL || l < strlen((*shortest)->word)) *shortest = root;
    tree_longest_shortest(root->left, longest, shortest);
    tree_longest_shortest(root->right, longest, shortest);
}

/* ======================= СОХРАНЕНИЕ / ВОССТАНОВЛЕНИЕ (п. C) ======================= */

/*
  Формат: preorder, по одному токену на строку:
    #                     — пустой узел
    <count> <word>        — узел (слово без пробелов, т.к. сепараторы
                            обычно включают пробел; но слово может
                            содержать всё, что не сепаратор, включая
                            перевод строки — его мы исключаем).
  Восстановление читает токены в том же порядке и строит дерево.
*/

static ErrorsDef save_tree_rec(const Node *root, FILE *f)
{
    if (!root)
    {
        if (fprintf(f, "#\n") < 0) return WRITEFILE_ERROR;
        return STAT_OK;
    }
    if (fprintf(f, "%llu %s\n", root->count, root->word) < 0)
        return WRITEFILE_ERROR;
    ErrorsDef e = save_tree_rec(root->left, f);
    if (e != STAT_OK) return e;
    return save_tree_rec(root->right, f);
}

static ErrorsDef save_tree(const char *path, const Node *root)
{
    if (!path) return INVALID_ARGUMENT;
    FILE *f = fopen(path, "w");
    if (!f) return OPENFILE_ERROR;
    ErrorsDef e = save_tree_rec(root, f);
    if (fclose(f) != 0 && e == STAT_OK) e = CLOSEFILE_ERROR;
    return e;
}

/* Читает один токен (строку) из файла; возвращает STAT_OK / TOKEN_EOF */
static ErrorsDef read_line_token(FILE *f, char *buf, size_t cap)
{
    if (fgets(buf, (int)cap, f) == NULL)
        return feof(f) ? TOKEN_EOF : READFILE_ERROR;
    size_t L = strlen(buf);
    while (L > 0 && (buf[L-1] == '\n' || buf[L-1] == '\r'))
        buf[--L] = '\0';
    return STAT_OK;
}

static ErrorsDef load_tree_rec(FILE *f, Node **out)
{
    if (!f || !out) return INVALID_ARGUMENT;
    *out = NULL;

    char line[8192];
    ErrorsDef e = read_line_token(f, line, sizeof(line));
    if (e == TOKEN_EOF) return READFILE_ERROR; /* неполное дерево */
    if (e != STAT_OK) return e;

    if (line[0] == '#' && line[1] == '\0') return STAT_OK;

    /* разбор "<count> <word>" */
    char *sp = strchr(line, ' ');
    if (!sp) return INVALID_NUMBER_FORMAT;
    *sp = '\0';
    char *end = NULL;
    unsigned long long cnt = strtoull(line, &end, 10);
    if (!end || *end != '\0' || end == line) return INVALID_NUMBER_FORMAT;

    const char *word = sp + 1;
    if (*word == '\0') return EMPTY_STRING;

    Node *n = node_create(word);
    if (!n) return MEMORY_ERROR;
    n->count = cnt;

    e = load_tree_rec(f, &n->left);
    if (e != STAT_OK) { n->left = NULL; tree_free(n); return e; }
    e = load_tree_rec(f, &n->right);
    if (e != STAT_OK) { tree_free(n); return e; }

    *out = n;
    return STAT_OK;
}

static ErrorsDef load_tree(const char *path, Node **root)
{
    if (!path || !root) return INVALID_ARGUMENT;
    *root = NULL;
    FILE *f = fopen(path, "r");
    if (!f) return OPENFILE_ERROR;
    ErrorsDef e = load_tree_rec(f, root);
    if (e == TOKEN_EOF) e = READFILE_ERROR; /* пустой файл */
    if (fclose(f) != 0 && e == STAT_OK) e = CLOSEFILE_ERROR;
    if (e != STAT_OK) { tree_free(*root); *root = NULL; }
    return e;
}

/* ======================= ПРОВЕРКА СЕПАРАТОРОВ ======================= */

static ErrorsDef parse_separators(int argc, char **argv,
                                  unsigned char sep[256])
{
    if (!argv || !sep) return INVALID_ARGUMENT;
    for (int i = 0; i < 256; ++i) sep[i] = 0;

    if (argc < 3) return MATCH_ARGS; /* путь + хотя бы один сепаратор */

    for (int i = 2; i < argc; ++i)
    {
        if (!argv[i] || argv[i][0] == '\0' || argv[i][1] != '\0')
            return INVALID_ARGUMENT; /* ровно 1 символ */
        sep[(unsigned char)argv[i][0]] = 1;
    }
    return STAT_OK;
}

/* ======================= ИНТЕРАКТИВНЫЙ ДИАЛОГ ======================= */

static int read_int_console(const char *prompt, long long *out)
{
    if (!prompt || !out) return 0;
    char buf[64];
    fputs(prompt, stdout);
    fflush(stdout);
    if (!fgets(buf, sizeof(buf), stdin)) return 0;
    char *end = NULL;
    long long v = strtoll(buf, &end, 10);
    if (end == buf) return 0;
    while (*end == ' ' || *end == '\t') ++end;
    if (*end != '\0' && *end != '\n' && *end != '\r') return 0;
    *out = v;
    return 1;
}

static int read_str_console(const char *prompt, char *buf, size_t cap)
{
    if (!prompt || !buf || cap == 0) return 0;
    fputs(prompt, stdout);
    fflush(stdout);
    if (!fgets(buf, (int)cap, stdin)) return 0;
    size_t L = strlen(buf);
    while (L > 0 && (buf[L-1] == '\n' || buf[L-1] == '\r'))
        buf[--L] = '\0';
    return 1;
}

static void menu(void)
{
    puts("=== Меню ===");
    puts("1. Частота слова");
    puts("2. Top-N по частоте");
    puts("3. Самое длинное / короткое слово");
    puts("4. Глубина дерева");
    puts("5. Сохранить дерево в файл");
    puts("6. Загрузить дерево из файла");
    puts("7. Показать всё дерево (inorder)");
    puts("0. Выход");
}

static void inorder_print(const Node *root)
{
    if (!root) return;
    inorder_print(root->left);
    printf("%s : %llu\n", root->word, root->count);
    inorder_print(root->right);
}

/* ======================= MAIN ======================= */

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        errors(MATCH_ARGS);
        return 1;
    }

    unsigned char sep[256];
    ErrorsDef err = parse_separators(argc, argv, sep);
    if (err != STAT_OK) { errors(err); return 1; }

    Node *root = NULL;
    err = build_from_file(argv[1], sep, &root);
    if (err != STAT_OK) { errors(err); return 1; }

    int running = 1;
    while (running)
    {
        menu();
        long long choice = -1;
        if (!read_int_console("Выбор: ", &choice))
        {
            puts("Некорректный ввод.");
            continue;
        }

        switch (choice)
        {
        case 1: /* частота слова */
        {
            char w[1024];
            if (!read_str_console("Слово: ", w, sizeof(w))) break;
            unsigned long long c = 0;
            err = tree_lookup(root, w, &c);
            if (err != STAT_OK) { errors(err); break; }
            printf("'%s' встречается %llu раз\n", w, c);
            break;
        }
        case 2: /* top-N */
        {
            long long n = 0;
            if (!read_int_console("N: ", &n)) break;
            if (n < 0) { errors(INVALID_ARRAY_SIZE); break; }
            NodeVec v = {0};
            ErrorsDef e = STAT_OK;
            tree_collect(root, &v, &e);
            if (e != STAT_OK) { errors(e); free(v.arr); break; }
            qsort(v.arr, v.size, sizeof(Node *), cmp_by_count_desc);
            size_t lim = (size_t)n < v.size ? (size_t)n : v.size;
            for (size_t i = 0; i < lim; ++i)
                printf("%zu. %s : %llu\n", i + 1, v.arr[i]->word, v.arr[i]->count);
            free(v.arr);
            break;
        }
        case 3: /* самое длинное/короткое */
        {
            const Node *L = NULL, *S = NULL;
            tree_longest_shortest(root, &L, &S);
            if (!L) puts("Дерево пусто.");
            else printf("Самое длинное: %s (%zu)\nСамое короткое: %s (%zu)\n",
                        L->word, strlen(L->word), S->word, strlen(S->word));
            break;
        }
        case 4: /* глубина */
            printf("Глубина дерева: %d\n", tree_depth(root));
            break;
        case 5: /* сохранить */
        {
            char p[1024];
            if (!read_str_console("Путь для сохранения: ", p, sizeof(p))) break;
            err = save_tree(p, root);
            if (err != STAT_OK) { errors(err); break; }
            puts("Сохранено.");
            break;
        }
        case 6: /* загрузить */
        {
            char p[1024];
            if (!read_str_console("Путь для загрузки: ", p, sizeof(p))) break;
            Node *nr = NULL;
            err = load_tree(p, &nr);
            if (err != STAT_OK) { errors(err); break; }
            tree_free(root);
            root = nr;
            puts("Загружено.");
            break;
        }
        case 7:
            inorder_print(root);
            break;
        case 0:
            running = 0;
            break;
        default:
            errors(INVALID_FLAG);
            break;
        }
    }

    tree_free(root);
    return 0;
}