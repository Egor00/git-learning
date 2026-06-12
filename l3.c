#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Node {
    char data[32];
    struct Node *left;
    struct Node *right;
} Node;

/* ---------- Работа с деревом ---------- */

Node* createNode(const char *s)
{
    Node *p = (Node*)malloc(sizeof(Node));
    strcpy(p->data, s);
    p->left = NULL;
    p->right = NULL;
    return p;
}

Node* copyTree(Node *root)
{
    if (!root)
        return NULL;

    Node *p = createNode(root->data);

    p->left = copyTree(root->left);
    p->right = copyTree(root->right);

    return p;
}

/* ---------- Построение дерева ---------- */

int priority(char op)
{
    switch(op)
    {
        case '+':
        case '-':
            return 1;

        case '*':
        case '/':
            return 2;

        case '^':
            return 3;
    }

    return 0;
}

void makeOperation(Node *nodes[], int *ntop,
                   char ops[], int *otop)
{
    char op = ops[(*otop)--];

    Node *right = nodes[(*ntop)--];
    Node *left  = nodes[(*ntop)--];

    char s[2];
    s[0] = op;
    s[1] = '\0';

    Node *p = createNode(s);

    p->left = left;
    p->right = right;

    nodes[++(*ntop)] = p;
}

Node* buildTree(char expr[])
{
    Node *nodes[200];
    char ops[200];

    int ntop = -1;
    int otop = -1;

    for (int i = 0; expr[i]; i++)
    {
        if (isspace(expr[i]))
            continue;

        if (isalnum(expr[i]))
        {
            char token[32];
            int k = 0;

            while (isalnum(expr[i]))
            {
                token[k++] = expr[i];
                i++;
            }

            token[k] = '\0';
            i--;

            nodes[++ntop] = createNode(token);
        }
        else if (expr[i] == '(')
        {
            ops[++otop] = '(';
        }
        else if (expr[i] == ')')
        {
            while (otop >= 0 && ops[otop] != '(')
                makeOperation(nodes, &ntop, ops, &otop);

            otop--;
        }
        else
        {
            while (otop >= 0 &&
                   ops[otop] != '(' &&
                   priority(ops[otop]) >= priority(expr[i]))
            {
                makeOperation(nodes, &ntop, ops, &otop);
            }

            ops[++otop] = expr[i];
        }
    }

    while (otop >= 0)
        makeOperation(nodes, &ntop, ops, &otop);

    return nodes[0];
}

/* строим a*a*...*a */
Node* makePowerProduct(Node *base, int n)
{
    if (n == 1)
        return copyTree(base);

    Node *result = copyTree(base);

    for (int i = 1; i < n; i++)
    {
        Node *mul = createNode("*");

        mul->left = result;
        mul->right = copyTree(base);

        result = mul;
    }

    return result;
}

int isNumber(const char *s)
{
    int i = 0;

    if (!s[0])
        return 0;

    while (s[i])
    {
        if (!isdigit(s[i]))
            return 0;

        i++;
    }

    return 1;
}

Node* transform(Node *root)
{
    if (!root)
        return NULL;

    root->left = transform(root->left);
    root->right = transform(root->right);

    if (strcmp(root->data, "^") == 0)
    {
        if (root->right &&
            !root->right->left &&
            !root->right->right &&
            isNumber(root->right->data))
        {
            int power = atoi(root->right->data);

            if (power > 0)
                return makePowerProduct(root->left, power);
        }
    }

    return root;
}

/* ---------- Вывод ---------- */

void printExpression(Node *root)
{
    if (!root)
        return;

    if (root->left || root->right)
        printf("(");

    printExpression(root->left);

    printf("%s", root->data);

    printExpression(root->right);

    if (root->left || root->right)
        printf(")");
}

void printTree(Node *root, int level)
{
    if (!root)
        return;

    printTree(root->right, level + 1);

    for (int i = 0; i < level; i++)
        printf("    ");

    printf("%s\n", root->data);

    printTree(root->left, level + 1);
}

/* ---------- main ---------- */
int main()
{
    char expr[256];

    printf("Введите выражение:\n");
    fgets(expr, sizeof(expr), stdin);

    Node *root = buildTree(expr);

    printf("\nИсходное выражение:\n");
    printExpression(root);

    printf("\n\nИсходное дерево:\n");
    printTree(root, 0);

    root = transform(root);

    printf("\nПреобразованное выражение:\n");
    printExpression(root);

    printf("\n\nПреобразованное дерево:\n");
    printTree(root, 0);

    return 0;
}
