/*
ds.h
basic data structures
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* #region List and Stack */

typedef struct ListNode {
    void *data;
    struct ListNode *link;
} ListNode;

typedef struct {
    struct ListNode *top;
} List;

void list_init(List *l) { l->top = NULL; }

bool list_empty(List *l) { return l->top == NULL; }

void *list_peek(List *l) { return l->top->data; }

// void list_dump(List *l) {
//     ListNode *n = l->top;
//     printf("dmp: ");
//     while (n) {
//         printf("%s ", n->data);
//         n = n->link;
//         if (!n)
//             break;
//     }
//     printf("\n");
// }

void stack_push(List *l, void *d) {
    // alloc a new node to hold 'c'
    ListNode *node;
    node = malloc(sizeof(node));
    node->data = d;
    // there is no top node, make a new one
    if (!l->top) {
        node->link = NULL;
        l->top = node;
    } else {
        // get the top node
        ListNode *p = l->top;
        l->top = node;
        node->link = p;
    }
}

void *stack_pop(List *l) {
    // get the tio node
    ListNode *p = l->top;
    // set top to element below
    l->top = p->link;
    void *ch = p->data;
    free(p);
    return ch;
}

void list_push(List *l, void *d) {
    // alloc a new node to hold 'c'
    ListNode *node;
    node = malloc(sizeof(node));
    node->data = d;
    // check top node
    if (!l->top) {
        node->link = NULL;
        l->top = node;
    } else {
        // get the last node
        ListNode *n = l->top;
        while (n->link) {
            n = n->link;
        }
        // chain our node on
        n->link = node;
    }
}

void *list_pop(List *l) {
    // pop first node and set top to next
    ListNode *node = l->top;
    l->top = node->link; // move top pointer
    void *d = node->data;
    free(node);
    return d;
}

int list_indexof(List *l, void* item) {
    ListNode *n = l->top;
    int i = 0;
    do {
        if (n->data == item) return i;
        n = n->link;
        i++;
    } while (n);
    return -1;
}

/* #endregion */

/* #region Hashtable */
// TODO: damn it
/* #endregion */
