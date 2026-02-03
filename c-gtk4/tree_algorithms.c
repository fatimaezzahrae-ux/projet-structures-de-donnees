#include "tree_algorithms.h"
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// Helpers
// ============================================================================

static void traversal_add(TraversalResult *res, int value) {
  if (res->count >= res->capacity) {
    res->capacity = (res->capacity == 0) ? 10 : res->capacity * 2;
    res->values = (int *)realloc(res->values, res->capacity * sizeof(int));
  }
  res->values[res->count++] = value;
}

void free_traversal_result(TraversalResult *res) {
  if (res->values)
    free(res->values);
  res->values = NULL;
  res->count = 0;
  res->capacity = 0;
}

// ============================================================================
// BST Implementation
// ============================================================================

BinaryNode *create_binary_node(int value) {
  BinaryNode *node = (BinaryNode *)malloc(sizeof(BinaryNode));
  if (!node)
    return NULL;
  node->value = value;
  node->left = NULL;
  node->right = NULL;
  node->x = 0;
  node->y = 0;
  return node;
}

BinaryNode *bst_insert(BinaryNode *root, int value) {
  if (!root)
    return create_binary_node(value);

  if (value < root->value) {
    root->left = bst_insert(root->left, value);
  } else if (value > root->value) {
    root->right = bst_insert(root->right, value);
  }
  // Si égal, on ne fait rien (pas de doublons dans ce BST simple)
  return root;
}

static BinaryNode *bst_min_value_node(BinaryNode *node) {
  BinaryNode *current = node;
  while (current && current->left != NULL)
    current = current->left;
  return current;
}

BinaryNode *bst_delete(BinaryNode *root, int value) {
  if (!root)
    return root;

  if (value < root->value) {
    root->left = bst_delete(root->left, value);
  } else if (value > root->value) {
    root->right = bst_delete(root->right, value);
  } else {
    // Trouvé
    if (root->left == NULL) {
      BinaryNode *temp = root->right;
      free(root);
      return temp;
    } else if (root->right == NULL) {
      BinaryNode *temp = root->left;
      free(root);
      return temp;
    }

    // 2 enfants: successeur in-order (plus petit du sous-arbre droit)
    BinaryNode *temp = bst_min_value_node(root->right);
    root->value = temp->value; // Copie valeur
    root->right =
        bst_delete(root->right, temp->value); // Supprime le successeur
  }
  return root;
}

void bst_modify(BinaryNode **root, int old_val, int new_val) {
  *root = bst_delete(*root, old_val);
  *root = bst_insert(*root, new_val);
}

BinaryNode *bst_search(BinaryNode *root, int value) {
  if (!root || root->value == value)
    return root;
  if (value < root->value)
    return bst_search(root->left, value);
  return bst_search(root->right, value);
}

void free_binary_tree(BinaryNode *root) {
  if (!root)
    return;
  free_binary_tree(root->left);
  free_binary_tree(root->right);
  free(root);
}

int bst_get_height(BinaryNode *root) {
  if (!root)
    return 0;
  int lh = bst_get_height(root->left);
  int rh = bst_get_height(root->right);
  return 1 + (lh > rh ? lh : rh);
}

int bst_get_size(BinaryNode *root) {
  if (!root)
    return 0;
  return 1 + bst_get_size(root->left) + bst_get_size(root->right);
}

// BST Traversals

void _bst_pre(BinaryNode *n, TraversalResult *r) {
  if (!n)
    return;
  traversal_add(r, n->value);
  _bst_pre(n->left, r);
  _bst_pre(n->right, r);
}
TraversalResult bst_traverse_preorder(BinaryNode *root) {
  TraversalResult r = {0};
  _bst_pre(root, &r);
  return r;
}

void _bst_in(BinaryNode *n, TraversalResult *r) {
  if (!n)
    return;
  _bst_in(n->left, r);
  traversal_add(r, n->value);
  _bst_in(n->right, r);
}
TraversalResult bst_traverse_inorder(BinaryNode *root) {
  TraversalResult r = {0};
  _bst_in(root, &r);
  return r;
}

void _bst_post(BinaryNode *n, TraversalResult *r) {
  if (!n)
    return;
  _bst_post(n->left, r);
  _bst_post(n->right, r);
  traversal_add(r, n->value);
}
TraversalResult bst_traverse_postorder(BinaryNode *root) {
  TraversalResult r = {0};
  _bst_post(root, &r);
  return r;
}

TraversalResult bst_traverse_bfs(BinaryNode *root) {
  TraversalResult r = {0};
  if (!root)
    return r;

  // File simple (tableau dynamique)
  BinaryNode **queue = (BinaryNode **)malloc(
      10000 * sizeof(BinaryNode *)); // Capacité fixe simple
  int front = 0, rear = 0;

  queue[rear++] = root;

  while (front < rear) {
    BinaryNode *n = queue[front++];
    traversal_add(&r, n->value);
    if (n->left)
      queue[rear++] = n->left;
    if (n->right)
      queue[rear++] = n->right;
  }

  free(queue);
  return r;
}

// ============================================================================
// N-ary Implementation
// ============================================================================

NaryNode *create_nary_node(int value) {
  NaryNode *node = (NaryNode *)malloc(sizeof(NaryNode));
  if (!node)
    return NULL;
  node->value = value;
  node->num_children = 0;
  node->capacity = 2;
  node->children = (NaryNode **)malloc(sizeof(NaryNode *) * node->capacity);
  node->x = 0;
  node->y = 0;
  return node;
}

void add_child_internal(NaryNode *parent, NaryNode *child) {
  if (!parent || !child)
    return;
  if (parent->num_children >= parent->capacity) {
    parent->capacity *= 2;
    parent->children = (NaryNode **)realloc(
        parent->children, sizeof(NaryNode *) * parent->capacity);
  }
  parent->children[parent->num_children++] = child;
}

NaryNode *nary_search(NaryNode *root, int value) {
  if (!root)
    return NULL;
  if (root->value == value)
    return root;
  for (int i = 0; i < root->num_children; i++) {
    NaryNode *res = nary_search(root->children[i], value);
    if (res)
      return res;
  }
  return NULL;
}

NaryNode *nary_find_parent(NaryNode *root, int value) {
  if (!root)
    return NULL;
  for (int i = 0; i < root->num_children; i++) {
    if (root->children[i]->value == value)
      return root;
    NaryNode *res = nary_find_parent(root->children[i], value);
    if (res)
      return res;
  }
  return NULL;
}

NaryNode *nary_insert(NaryNode *root, int value, int parent_value) {
  if (!root)
    return create_nary_node(value);

  NaryNode *parent = nary_search(root, parent_value);
  if (parent) {
    add_child_internal(parent, create_nary_node(value));
    return root;
  }
  return root; // Parent non trouvé
}

NaryNode *nary_delete(NaryNode *root, int value) {
  if (!root)
    return NULL;

  // Si on supprime la racine
  if (root->value == value) {
    if (root->num_children > 0) {
      // Promouvoir le premier enfant
      NaryNode *new_root = root->children[0];
      // Ajouter les autres enfants de l'ancienne racine au nouveau
      for (int i = 1; i < root->num_children; i++) {
        add_child_internal(new_root, root->children[i]);
      }
      // Attention: root->children est un tableau de pointeurs, libération
      // propre nécessaire Ici on a déplacé les pointeurs, donc on peut juste
      // free le tableau struct
      free(root->children);
      free(root);
      return new_root;
    } else {
      free(root->children);
      free(root);
      return NULL;
    }
  }

  NaryNode *parent = nary_find_parent(root, value);
  if (parent) {
    for (int i = 0; i < parent->num_children; i++) {
      if (parent->children[i]->value == value) {
        NaryNode *to_del = parent->children[i];
        // Réattacher les petits-enfants au parent
        for (int j = 0; j < to_del->num_children; j++) {
          add_child_internal(parent, to_del->children[j]);
        }

        // Supprimer l'élément du tableau du parent (shift)
        for (int k = i; k < parent->num_children - 1; k++) {
          parent->children[k] = parent->children[k + 1];
        }
        parent->num_children--;

        free(to_del->children);
        free(to_del);
        break;
      }
    }
  }
  return root;
}

void nary_modify(NaryNode *root, int old_val, int new_val) {
  NaryNode *node = nary_search(root, old_val);
  if (node) {
    node->value = new_val;
  }
}

void free_nary_tree(NaryNode *root) {
  if (!root)
    return;
  for (int i = 0; i < root->num_children; i++) {
    free_nary_tree(root->children[i]);
  }
  free(root->children);
  free(root);
}

int nary_get_height(NaryNode *root) {
  if (!root)
    return 0;
  if (root->num_children == 0)
    return 1;
  int max_h = 0;
  for (int i = 0; i < root->num_children; i++) {
    int h = nary_get_height(root->children[i]);
    if (h > max_h)
      max_h = h;
  }
  return 1 + max_h;
}

int nary_get_size(NaryNode *root) {
  if (!root)
    return 0;
  int size = 1;
  for (int i = 0; i < root->num_children; i++) {
    size += nary_get_size(root->children[i]);
  }
  return size;
}

// N-ary Traversals

void _nary_pre(NaryNode *n, TraversalResult *r) {
  if (!n)
    return;
  traversal_add(r, n->value);
  for (int i = 0; i < n->num_children; i++)
    _nary_pre(n->children[i], r);
}
TraversalResult nary_traverse_preorder(NaryNode *root) {
  TraversalResult r = {0};
  _nary_pre(root, &r);
  return r;
}

void _nary_post(NaryNode *n, TraversalResult *r) {
  if (!n)
    return;
  for (int i = 0; i < n->num_children; i++)
    _nary_post(n->children[i], r);
  traversal_add(r, n->value);
}
TraversalResult nary_traverse_postorder(NaryNode *root) {
  TraversalResult r = {0};
  _nary_post(root, &r);
  return r;
}

TraversalResult nary_traverse_bfs(NaryNode *root) {
  TraversalResult r = {0};
  if (!root)
    return r;

  NaryNode **queue = (NaryNode **)malloc(10000 * sizeof(NaryNode *));
  int front = 0, rear = 0;

  queue[rear++] = root;

  while (front < rear) {
    NaryNode *n = queue[front++];
    traversal_add(&r, n->value);
    for (int i = 0; i < n->num_children; i++)
      queue[rear++] = n->children[i];
  }

  free(queue);
  return r;
}

// ============================================================================
// Conversion
// ============================================================================

BinaryNode *convert_nary_to_binary(NaryNode *nary_root) {
  if (!nary_root)
    return NULL;
  BinaryNode *bin_root = create_binary_node(nary_root->value);
  if (nary_root->num_children > 0) {
    bin_root->left = convert_nary_to_binary(nary_root->children[0]);
    BinaryNode *current = bin_root->left;
    for (int i = 1; i < nary_root->num_children; i++) {
      current->right = convert_nary_to_binary(nary_root->children[i]);
      current = current->right;
    }
  }
  return bin_root;
}
