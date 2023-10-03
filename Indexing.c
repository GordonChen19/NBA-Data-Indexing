#include <stdio.h>
#include <stdlib.h>

#define bucketSize 3

typedef struct node {
    int isLeaf;
    struct node** ptr;
    int* key;
    int size;
} node;

node* createNode() {
    node* newNode = (node*)malloc(sizeof(node));
    newNode->isLeaf = 0;
    newNode->ptr = (node**)malloc((bucketSize + 1) * sizeof(node*));
    newNode->key = (int*)malloc(bucketSize * sizeof(int));
    newNode->size = 0;
    return newNode;
}

typedef struct Btree {
    node* root;
} Btree;

Btree* createBtree() {
    Btree* newBtree = (Btree*)malloc(sizeof(Btree));
    newBtree->root = NULL;
    return newBtree;
}

node* findParent(node* root, node* child) {
    if (root == NULL || root->isLeaf || root->ptr[0] == child) {
        return NULL;
    }
    for (int i = 0; i < root->size; i++) {
        if (root->ptr[i + 1] == child) {
            return root;
        }
    }
    return findParent(root->ptr[0], child);
}

void shiftLevel(int key, node* leftChild, node* rightChild) {
    node* parent = findParent(root, leftChild);
    if (parent == NULL) {
        node* newRoot = createNode();
        newRoot->key[0] = key;
        newRoot->ptr[0] = leftChild;
        newRoot->ptr[1] = rightChild;
        newRoot->size = 1;
        root = newRoot;
        return;
    }
    int i;
    for (i = 0; i < parent->size; i++) {
        if (parent->key[i] > key) {
            break;
        }
    }
    for (int j = parent->size; j > i; j--) {
        parent->key[j] = parent->key[j - 1];
    }
    for (int j = parent->size + 1; j > i + 1; j--) {
        parent->ptr[j] = parent->ptr[j - 1];
    }
    parent->key[i] = key;
    parent->ptr[i + 1] = rightChild;
    parent->size++;
}

void insert(Btree* btree, int key) {
    if (btree->root == NULL) {
        node* newNode = createNode();
        newNode->key[0] = key;
        newNode->isLeaf = 1;
        newNode->size = 1;
        btree->root = newNode;
        return;
    }
    node* current = btree->root;
    node* parent = NULL;
    while (!current->isLeaf) {
        parent = current;
        for (int i = 0; i < current->size; i++) {
            if (key < current->key[i]) {
                current = current->ptr[i];
                break;
            }
            if (i == current->size - 1) {
                current = current->ptr[i + 1];
                break;
            }
        }
    }
    if (current->size < bucketSize) {
        int i = 0;
        while (key > current->key[i] && i < current->size) {
            i++;
        }
        for (int j = current->size; j > i; j--) {
            current->key[j] = current->key[j - 1];
        }
        current->key[i] = key;
        current->size++;
    } else {
        node* newNode = createNode();
        int virtualNode[bucketSize + 1];
        for (int i = 0; i < bucketSize; i++) {
            virtualNode[i] = current->key[i];
        }
        int i = 0, j;
        while (key > virtualNode[i] && i < bucketSize) {
            i++;
        }
        for (int j = bucketSize + 1; j > i; j--) {
            virtualNode[j] = virtualNode[j - 1];
        }
        virtualNode[i] = key;
        newNode->isLeaf = 1;
        current->size = (bucketSize + 1) / 2;
        newNode->size = bucketSize + 1 - (bucketSize + 1) / 2;
        current->ptr[current->size] = newNode;
        newNode->ptr[newNode->size] = current->ptr[bucketSize];
        current->ptr[bucketSize] = NULL;
        for (i = 0; i < current->size; i++) {
            current->key[i] = virtualNode[i];
        }
        for (i = 0, j = current->size; i < newNode->size; i++, j++) {
            newNode->key[i] = virtualNode[j];
        }
        if (current == btree->root) {
            node* newRoot = createNode();
            newRoot->key[0] = newNode->key[0];
            newRoot->ptr[0] = current;
            newRoot->ptr[1] = newNode;
            newRoot->size = 1;
            btree->root = newRoot;
        } else {
            shiftLevel(newNode->key[0], current, newNode);
        }
    }
}

int search(node* root, int key) {
    if (root == NULL) {
        return 0;
    }
    int i = 0;
    while (i < root->size && key > root->key[i]) {
        i++;
    }
    if (i < root->size && key == root->key[i]) {
        return 1;
    }
    if (root->isLeaf) {
        return 0;
    }
    return search(root->ptr[i], key);
}

void display(node* root) {
    if (root != NULL) {
        for (int i = 0; i < root->size; i++) {
            display(root->ptr[i]);
            printf("%d ", root->key[i]);
        }
        display(root->ptr[root->size]);
    }
}

void deleteNode(Btree* btree, int key) {
    if (btree->root == NULL) {
        return;
    }
    node* current = btree->root;
    node* parent = NULL;
    int isLeftChild = 1;
    while (!current->isLeaf) {
        parent = current;
        isLeftChild