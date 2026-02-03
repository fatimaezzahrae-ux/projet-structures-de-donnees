#ifndef TREE_ALGORITHMS_H
#define TREE_ALGORITHMS_H

#include <stdlib.h>

// Structures de données

typedef struct {
    int *values;
    int count;
    int capacity;
} TraversalResult;

typedef struct BinaryNode {
    int value;
    struct BinaryNode *left;
    struct BinaryNode *right;
    int x, y; // Pour l'affichage graphique
} BinaryNode;

typedef struct NaryNode {
    int value;
    struct NaryNode **children;
    int num_children;
    int capacity;
    int x, y; // Pour l'affichage graphique
} NaryNode;

// Fonctions utilitaires
void free_traversal_result(TraversalResult *res);

// Fonctions pour Arbres Binaires (BST)
BinaryNode *create_binary_node(int value);
BinaryNode *bst_insert(BinaryNode *root, int value);
BinaryNode *bst_delete(BinaryNode *root, int value);
void bst_modify(BinaryNode **root, int old_val, int new_val);
BinaryNode *bst_search(BinaryNode *root, int value);
void free_binary_tree(BinaryNode *root);
int bst_get_height(BinaryNode *root);
int bst_get_size(BinaryNode *root);

// Parcours BST
TraversalResult bst_traverse_preorder(BinaryNode *root);
TraversalResult bst_traverse_inorder(BinaryNode *root);
TraversalResult bst_traverse_postorder(BinaryNode *root);
TraversalResult bst_traverse_bfs(BinaryNode *root);

// Fonctions pour Arbres N-aires
NaryNode *create_nary_node(int value);
NaryNode *nary_search(NaryNode *root, int value);
NaryNode *nary_find_parent(NaryNode *root, int value);
NaryNode *nary_insert(NaryNode *root, int value, int parent_value);
NaryNode *nary_delete(NaryNode *root, int value);
void nary_modify(NaryNode *root, int old_val, int new_val);
void free_nary_tree(NaryNode *root);
int nary_get_height(NaryNode *root);
int nary_get_size(NaryNode *root);

// Parcours N-aires
TraversalResult nary_traverse_preorder(NaryNode *root);
TraversalResult nary_traverse_postorder(NaryNode *root);
TraversalResult nary_traverse_bfs(NaryNode *root);

// Conversion
BinaryNode *convert_nary_to_binary(NaryNode *nary_root);

#endif // TREE_ALGORITHMS_H
