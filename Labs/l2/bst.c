#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdbool.h>

jmp_buf exc;

typedef enum {
	E_AA = 0,
	E_AB,
	E_BAL,
	E_BB
} senum;

typedef struct Node{
	const int key;
	senum value;
	struct Node* left;
	struct Node* right;
	struct Node* parent;
} Node;

typedef struct {
	Node* head;
} Bst;

int Kidz(Node* prnt) {
	if (prnt == NULL) return 0;
	if ((prnt->left == NULL) && (prnt->right == NULL)) return 0;
	else if (((prnt->left != NULL) && (prnt->right == NULL)) || ((prnt->left == NULL) && (prnt->right != NULL))) return 1;
	else return 2;
};

void rm_branch(Node* node) {
	if (node == NULL) return;
	rm_branch(node->left);
	rm_branch(node->right);
	free(node);
};

void Insert(Bst* b, const int key, senum value){
	if(b->head == NULL){
		Node* new_node = (Node*)malloc(sizeof(Node));
		*(int*)&new_node->key = key;
		new_node->value = value;
		new_node->left = NULL;
		new_node->right = NULL;
		new_node->parent = NULL;
		b->head  = new_node;
	}
	else{
		Node* current = b->head;
		Node* cur_mir = b->head;
		while (1){
			if (current == NULL) {
				if (key > cur_mir->key)	{
					Node* new_node = (Node*)malloc(sizeof(Node));
                                        *(int*)&new_node->key = key;
                                        new_node->value = value;
                                        new_node->left = NULL;
                                        new_node->right = NULL;
                                        new_node->parent = cur_mir;
					cur_mir->right = new_node;
                                        break;
				}
				else {
					Node* new_node = (Node*)malloc(sizeof(Node));
                                        *(int*)&new_node->key = key;
                                        new_node->value = value;
                                        new_node->left = NULL;
                                        new_node->right = NULL;
                                        new_node->parent = cur_mir;
					cur_mir->left = new_node;
                                        break;
				};
			};
			if (current->key == key){
				current->value = value;
				break;
			}
			else if (current->key < key){
				cur_mir = current;
				current = current->right;
			}
			else {
				cur_mir = current;
				current = current->left;
			};
		};
	};
};

void Erase(Bst* b, const int key){
	if (b->head == NULL) longjmp(exc, 1);
	Node* current = b->head;
	while (1) {
		if (current == NULL){
			longjmp(exc, 1);
			break;
		};
		if (current->key == key){
			if (current == b->head) {
				if ((current->right != NULL) && (current->left != NULL)) {
					if (current->right->value >= current->left->value) {
						rm_branch(current->right);
						b->head = current->left;
						free(b->head->parent);
						b->head->parent = NULL;
					}
					else {
						rm_branch(current->left);
                                                b->head = current->right;
                                                free(b->head->parent);
                                                b->head->parent = NULL;
					}
				}
				else if ((current->right != NULL) && (current->left == NULL)) {
                                        b->head = current->right;
                                        free(b->head->parent);
					b->head->parent = NULL;
				}
				else if ((current->right == NULL) && (current->left != NULL)) {
                                        b->head = current->left;
                                        free(b->head->parent);
					b->head->parent = NULL;
				}
				else {
					free(b->head);
					b->head = NULL;
				};
			}
			else {
				if ((current->right != NULL) && (current->left != NULL)) {
                                        if (current->right->value >= current->left->value) {
						current->parent->left = current->left;
						current->left->parent = current->parent;
						rm_branch(current->right);
						free(current);
					}
                                        else {
						current->parent->right = current->right;
						current->right->parent = current->parent;
						rm_branch(current->left);
						free(current);
					};
				}
                                else if ((current->right != NULL) && (current->left == NULL)) {
					current->parent->right = current->right;
					current->right->parent = current->parent;
					free(current);
				}
                                else if ((current->right == NULL) && (current->left != NULL)) {
					current->parent->left = current->left;
					current->left->parent = current->parent;
					free(current);
				}
                                else {
					if (current->key >= current->parent->key) {
						current->parent->right = NULL;
						free(current);
					}
					else {
						current->parent->left = NULL;
						free(current);
					};
				};
			};
			break;
		}
		else {
			if (current->key <= key) current = current->right;
			else current = current->left;
		};
	};
};

bool Find(Bst* b, const int key) {
	if (b->head == NULL) return false;
	else {
		Node* current = b->head;
		while (1) {
			if (current == NULL) {
				return false;
				break;
			};
			if (current->key == key) {
				return true;
				break;
			}
			else {
				if (current->key <= key) current = current->right;
				else current = current->left;
			};
		};
	};
};

int CountNodes(Bst* b, Node* node) {
	if (node == NULL) return 0;
	if (node == b->head) return 1+CountNodes(b, node->left)+CountNodes(b, node->right);
	if (node->value == Kidz(node)) return 1+CountNodes(b, node->left)+CountNodes(b, node->right);
	else return CountNodes(b, node->left)+CountNodes(b, node->right);
};

void free_tree(Node* node) {
	if (node == NULL) return;
	free_tree(node->left);
	free_tree(node->right);
	free(node);
};

int main() {
	if (setjmp(exc) == 0) {
		Bst bst = {.head = NULL};
		senum f = E_AA;
		printf("Inserting nodes...\n");
		Insert(&bst, 1, f);
		Insert(&bst, 2, f);
		Insert(&bst, 0, f);
		printf("Count: %d\n", CountNodes(&bst, bst.head));
		printf("Find 0: %s\n", Find(&bst, 0) ? "true" : "false");
		printf("Find 1: %s\n", Find(&bst, 1) ? "true" : "false");
		printf("Find 2: %s\n", Find(&bst, 2) ? "true" : "false");
		Erase(&bst, 0);
		printf("Number of nodes: %d\n", CountNodes(&bst, bst.head));
		printf("Find 1: %s\n", Find(&bst, 1) ? "true" : "false");
		printf("Find 2: %s\n", Find(&bst, 2) ? "true" : "false");
		printf("Find 0: %s\n", Find(&bst, 0) ? "true" : "false");
		printf("Headkey: %d\n", bst.head->key);
		free_tree(bst.head);
	} else {
		printf("Error occurred!\n");
	};
	return 0;
};
