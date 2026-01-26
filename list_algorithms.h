#ifndef LIST_ALGORITHMS_H
#define LIST_ALGORITHMS_H

#include <stdbool.h>
#include <stddef.h>

// Types de données supportés
typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_CHAR, TYPE_STRING } DataType;

// Union pour stocker différents types de données
typedef union {
  int int_val;
  float float_val;
  char char_val;
  char *string_val;
} NodeData;

// Nœud pour liste simplement chaînée
typedef struct SimpleNode {
  DataType type;
  NodeData data;
  struct SimpleNode *next;
} SimpleNode;

// Nœud pour liste doublement chaînée
typedef struct DoubleNode {
  DataType type;
  NodeData data;
  struct DoubleNode *next;
  struct DoubleNode *prev;
} DoubleNode;

// Structure pour gérer une liste simple
typedef struct {
  SimpleNode *head;
  DataType type;
  int size;
} SimpleList;

// Structure pour gérer une liste double
typedef struct {
  DoubleNode *head;
  DoubleNode *tail;
  DataType type;
  int size;
} DoubleList;

// Fonctions pour listes simples
SimpleList *create_simple_list(DataType type);
void free_simple_list(SimpleList *list);
bool insert_simple_at_beginning(SimpleList *list, NodeData data);
bool insert_simple_at_end(SimpleList *list, NodeData data);
bool insert_simple_at_position(SimpleList *list, NodeData data, int position);
bool delete_simple_at_position(SimpleList *list, int position);
bool modify_simple_at_position(SimpleList *list, NodeData data, int position);
SimpleNode *get_simple_node_at(SimpleList *list, int position);

// Fonctions pour listes doubles
DoubleList *create_double_list(DataType type);
void free_double_list(DoubleList *list);
bool insert_double_at_beginning(DoubleList *list, NodeData data);
bool insert_double_at_end(DoubleList *list, NodeData data);
bool insert_double_at_position(DoubleList *list, NodeData data, int position);
bool delete_double_at_position(DoubleList *list, int position);
bool modify_double_at_position(DoubleList *list, NodeData data, int position);
DoubleNode *get_double_node_at(DoubleList *list, int position);

// Fonctions de tri pour listes simples
void bubble_sort_simple(SimpleList *list);
void insertion_sort_simple(SimpleList *list);
void selection_sort_simple(SimpleList *list);

// Fonctions de tri pour listes doubles
void bubble_sort_double(DoubleList *list);
void insertion_sort_double(DoubleList *list);
void selection_sort_double(DoubleList *list);

// Fonctions utilitaires
int compare_node_data(NodeData a, NodeData b, DataType type);
void swap_simple_node_data(SimpleNode *a, SimpleNode *b);
void swap_double_node_data(DoubleNode *a, DoubleNode *b);
char *node_data_to_string(NodeData data, DataType type);

// Génération de données aléatoires
NodeData generate_random_node_data(DataType type);
void fill_simple_list_random(SimpleList *list, int count);
void fill_double_list_random(DoubleList *list, int count);

#endif // LIST_ALGORITHMS_H
