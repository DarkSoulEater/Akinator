#include <stdlib.h>
#include <assert.h>
#include "BinaryTree.hpp"

#define ERROR(ERROR_TEXT) perror(ERROR_TEXT); return;

void BinTreeInsert(BinTree::Node **ptr, void *data) {
    assert(ptr);

    if (*ptr) {
        ERROR("binary tree node must be empty");
    }

    *ptr = (BinTree::Node*) calloc(1, sizeof(BinTree::Node));
    if (!*ptr) {
        ERROR("failed to allocate memory for binary tree node");
    }

    (*ptr)->data_ = data;
}