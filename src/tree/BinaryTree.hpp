#ifndef SRC_TREE_BINARYTREE_HPP_
#define SRC_TREE_BINARYTREE_HPP_

struct BinaryTree {
    struct Node {
        void *data;

        Node *L, *R;
    } *head_ = nullptr;
};

typedef BinaryTree BinTree;

void BinTreeInsert(BinTree::Node **ptr, void *data);

#endif // SRC_TREE_BINARYTREE_HPP_