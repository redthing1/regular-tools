/*
ds.h
basic data structures
*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct ItemNode {
    void* data;
    struct ItemNode* link;
} ItemNode;

typedef struct {
    struct ItemNode* top;
} ItemList;

bool list_empty(ItemList *s) {
    return s->top == NULL;
}

void* list_peek(ItemList *s) {
    return s->top->data;
}

void list_dump(ItemList *s) {
    ItemNode* n = s->top;
    printf("dmp: ");
    while (n) {
        printf("%s ", n->data);
        n = n->link;
        if (!n) break;
    }
    printf("\n");
}

void stack_push(ItemList* s, void* d) {
    // alloc a new node to hold 'c'
    ItemNode* node;
    node = malloc(sizeof(node));
    node->data = d;
    // there is no top node, make a new one
    if (!s->top) {
        node->link = NULL;
        s->top = node;
    } else {
        // get the top node
        ItemNode* p = s->top;
        s->top = node;
        node->link = p;
    }
}

void* stack_pop(ItemList* s) {
    // get the tio node
    ItemNode* p = s->top;
    // set top to element below
    s->top = p->link;
    void* ch = p->data;
    free(p);
    return ch;
}

void queue_push(ItemList *q, void* d) {
    // alloc a new node to hold 'c'
    ItemNode* node;
    node = malloc(sizeof(node));
    node->data = d;
    // check top node
    if (!q->top) {
        node->link = NULL;
        q->top = node;
    } else {
        // get the last node
        ItemNode* n = q->top;
        while (n->link) {
            n = n->link;
        }
        // chain our node on
        n->link = node;
    }
}

void* queue_pop(ItemList *q) {
    // pop first node and set top to next
    ItemNode* node = q->top;
    q->top = node->link; // move top pointer
    void* ch = node->data;
    free(node);
    return ch;
}
