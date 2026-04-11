#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdbool.h>

jmp_buf ers;

typedef enum {
	E_AA = 0,
	E_AB,
	E_BAL,
	E_BB
} senum;

typedef struct Node{
	const int key;
	senum value;
	struct Node** children;
	int n;
	struct Node* parent;
} Node;

typedef struct {
	Node* head;
} Ntree;

int Kidz(Node* prnt) {
	if (prnt == NULL) return 0;
	if (prnt->children == NULL) return 0;
	else return prnt->n;
};

void rm_branch(Node* node) {
	if (node == NULL) return;
	for (int i = 0; i < node->n; i++) rm_branch(node->children[i]);
	free(node);
};

Node* find_node(Node* current, const int keydad) {
	if (current == NULL) return NULL;
	if (current->key == keydad) return current;
	for (int i = 0; i < current->n; i++) if (find_node(current->children[i], keydad) != NULL) return find_node(current->children[i], keydad);
	return NULL;
};

void Insert(Ntree* b, int keydad, const int key, senum value){
	Node* dad = find_node(b->head, keydad);
	if ((dad == NULL) && (b->head == NULL)) {
		Node* new_node = (Node*)malloc(sizeof(Node));
		*(int*)&new_node->key = key;
		new_node->value = value;
		new_node->children = NULL;
		new_node->n = 0;
		new_node->parent = NULL;
		b->head  = new_node;
	}
	else if ((dad == NULL) && (b->head != NULL)) {
		longjmp(ers, 1);
	}
	else {
		Node* new_node = (Node*)malloc(sizeof(Node));
		*(int*)&new_node->key = key;
		new_node->value = value;
		new_node->children = NULL;
		new_node->parent = dad;
		if (dad->n == 0) dad->children = (Node**)malloc(sizeof(Node*));
		else dad->children = (Node**)realloc(dad->children, sizeof(Node*)*(dad->n+1));
		dad->n++;
		dad->children[dad->n-1] = new_node;
	};
};

void Erase(Ntree* b, int key){
	if (b->head == NULL) {
		longjmp(ers, 1);
	}
	Node* current = find_node(b->head, key);
	if (current == NULL){
		longjmp(ers, 1);
	};
	if (current == b->head) {
		if (current->children != NULL) {
			senum val = E_AA;
			Node* res = NULL;
			for (int i = 0; i < current->n; i++) {
				if (current->children[i]->value > val) {
					val = current->children[i]->value;
					res = current->children[i];
				};
			};
			for (int i = 0; i < current->n; i++) if (current->children[i] != res) rm_branch(current->children[i]);
            b->head = res;
            free(current);
            b->head->parent = NULL;
		}
		else {
			free(b->head);
			b->head = NULL;
		};
	}
	else {
		if (current->children != NULL) {
			senum val = E_AA;
			Node* res = NULL;
			for (int i = 0; i < current->n; i++) {
				if (current->children[i]->value > val) {
					val = current->children[i]->value;
					res = current->children[i];
				};
			};
			for (int i = 0; i < current->n; i++) if (current->children[i] != res) rm_branch(current->children[i]);
			for (int i = 0; i < current->parent->n; i++) if (current->parent->children[i] == current) current->parent->children[i] = res;
			//res->parent = current->parent;
			free(current);
		}
                else {
			int j = 0;
			for (int i = 0; i < current->parent->n; i++) {
				if (current->parent->children[i] == current) j++;
				current->parent->children[i] = current->parent->children[j];
				j++;
			};
			current->parent->n--;
                        current->parent->children = realloc(current->parent->children, current->parent->n);
			free(current);
		};
	};
};

bool Find(Ntree* b, const int key) {
	if (b->head == NULL) return false;
	else {
		Node* current = find_node(b->head, key);
		if (current == NULL) return false;
		if (current->key == key) return true;
	};
};

int CountNodes(Node* node) {
	if (node == NULL) return 0;
	if (node->value == Kidz(node)) {
		int sum = 0;
		for (int i = 0; i < node->n; i++) sum += CountNodes(node->children[i]);
		return 1+sum;
	}
	else {
		int sum = 0;
		for (int i = 0; i < node->n; i++) sum += CountNodes(node->children[i]);
		return sum;
	};
};

void free_tree(Node* node) {
	if (node == NULL) return;
	for (int i = 0; i < node->n; i++) free_tree(node->children[i]);
	free(node);
};

int main() {
	if (setjmp(ers) == 0) {
		Ntree tr = {.head = NULL};
		senum f = E_AA;
		printf("Inserting nodes...\n");
		Insert(&tr, 0, 1, f);
		Insert(&tr, 1, 2, f);
		Insert(&tr, 2, 0, f);
		printf("Find 0: %s\n", Find(&tr, 0) ? "true" : "false");
		printf("Find 1: %s\n", Find(&tr, 1) ? "true" : "false");
		printf("Find 2: %s\n", Find(&tr, 2) ? "true" : "false");
		Erase(&tr, 2);
		printf("Find 0: %s\n", Find(&tr, 0) ? "true" : "false");
                printf("Find 1: %s\n", Find(&tr, 1) ? "true" : "false");
                printf("Find 2: %s\n", Find(&tr, 2) ? "true" : "false");
		printf("Number of nodes: %d\n", CountNodes(tr.head));
		printf("Find 1: %s\n", Find(&tr, 1) ? "true" : "false");
		printf("Find 3: %s\n", Find(&tr, 3) ? "true" : "false");
		free_tree(tr.head);
	}
	else printf("Error occurred!\n");
	return 0;
};
