#include "list_algorithms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================================
// Fonctions pour listes simples
// ============================================================================

SimpleList *create_simple_list(DataType type) {
  SimpleList *list = (SimpleList *)malloc(sizeof(SimpleList));
  if (list) {
    list->head = NULL;
    list->type = type;
    list->size = 0;
  }
  return list;
}

void free_simple_list(SimpleList *list) {
  if (!list)
    return;

  SimpleNode *current = list->head;
  while (current) {
    SimpleNode *next = current->next;
    if (list->type == TYPE_STRING && current->data.string_val) {
      free(current->data.string_val);
    }
    free(current);
    current = next;
  }
  free(list);
}

bool insert_simple_at_beginning(SimpleList *list, NodeData data) {
  if (!list)
    return false;

  SimpleNode *new_node = (SimpleNode *)malloc(sizeof(SimpleNode));
  if (!new_node)
    return false;

  new_node->type = list->type;
  new_node->data = data;
  new_node->next = list->head;
  list->head = new_node;
  list->size++;
  return true;
}

bool insert_simple_at_end(SimpleList *list, NodeData data) {
  if (!list)
    return false;

  SimpleNode *new_node = (SimpleNode *)malloc(sizeof(SimpleNode));
  if (!new_node)
    return false;

  new_node->type = list->type;
  new_node->data = data;
  new_node->next = NULL;

  if (!list->head) {
    list->head = new_node;
  } else {
    SimpleNode *current = list->head;
    while (current->next) {
      current = current->next;
    }
    current->next = new_node;
  }
  list->size++;
  return true;
}

bool insert_simple_at_position(SimpleList *list, NodeData data, int position) {
  if (!list || position < 0 || position > list->size)
    return false;

  if (position == 0)
    return insert_simple_at_beginning(list, data);

  SimpleNode *new_node = (SimpleNode *)malloc(sizeof(SimpleNode));
  if (!new_node)
    return false;

  new_node->type = list->type;
  new_node->data = data;

  SimpleNode *current = list->head;
  for (int i = 0; i < position - 1; i++) {
    current = current->next;
  }

  new_node->next = current->next;
  current->next = new_node;
  list->size++;
  return true;
}

bool delete_simple_at_position(SimpleList *list, int position) {
  if (!list || !list->head || position < 0 || position >= list->size)
    return false;

  SimpleNode *to_delete;

  if (position == 0) {
    to_delete = list->head;
    list->head = list->head->next;
  } else {
    SimpleNode *current = list->head;
    for (int i = 0; i < position - 1; i++) {
      current = current->next;
    }
    to_delete = current->next;
    current->next = to_delete->next;
  }

  if (list->type == TYPE_STRING && to_delete->data.string_val) {
    free(to_delete->data.string_val);
  }
  free(to_delete);
  list->size--;
  return true;
}

bool modify_simple_at_position(SimpleList *list, NodeData data, int position) {
  if (!list || position < 0 || position >= list->size)
    return false;

  SimpleNode *node = get_simple_node_at(list, position);
  if (!node)
    return false;

  if (list->type == TYPE_STRING && node->data.string_val) {
    free(node->data.string_val);
  }
  node->data = data;
  return true;
}

SimpleNode *get_simple_node_at(SimpleList *list, int position) {
  if (!list || position < 0 || position >= list->size)
    return NULL;

  SimpleNode *current = list->head;
  for (int i = 0; i < position; i++) {
    current = current->next;
  }
  return current;
}

// ============================================================================
// Fonctions pour listes doubles
// ============================================================================

DoubleList *create_double_list(DataType type) {
  DoubleList *list = (DoubleList *)malloc(sizeof(DoubleList));
  if (list) {
    list->head = NULL;
    list->tail = NULL;
    list->type = type;
    list->size = 0;
  }
  return list;
}

void free_double_list(DoubleList *list) {
  if (!list)
    return;

  DoubleNode *current = list->head;
  while (current) {
    DoubleNode *next = current->next;
    if (list->type == TYPE_STRING && current->data.string_val) {
      free(current->data.string_val);
    }
    free(current);
    current = next;
  }
  free(list);
}

bool insert_double_at_beginning(DoubleList *list, NodeData data) {
  if (!list)
    return false;

  DoubleNode *new_node = (DoubleNode *)malloc(sizeof(DoubleNode));
  if (!new_node)
    return false;

  new_node->type = list->type;
  new_node->data = data;
  new_node->prev = NULL;
  new_node->next = list->head;

  if (list->head) {
    list->head->prev = new_node;
  } else {
    list->tail = new_node;
  }

  list->head = new_node;
  list->size++;
  return true;
}

bool insert_double_at_end(DoubleList *list, NodeData data) {
  if (!list)
    return false;

  DoubleNode *new_node = (DoubleNode *)malloc(sizeof(DoubleNode));
  if (!new_node)
    return false;

  new_node->type = list->type;
  new_node->data = data;
  new_node->next = NULL;
  new_node->prev = list->tail;

  if (list->tail) {
    list->tail->next = new_node;
  } else {
    list->head = new_node;
  }

  list->tail = new_node;
  list->size++;
  return true;
}

bool insert_double_at_position(DoubleList *list, NodeData data, int position) {
  if (!list || position < 0 || position > list->size)
    return false;

  if (position == 0)
    return insert_double_at_beginning(list, data);
  if (position == list->size)
    return insert_double_at_end(list, data);

  DoubleNode *new_node = (DoubleNode *)malloc(sizeof(DoubleNode));
  if (!new_node)
    return false;

  new_node->type = list->type;
  new_node->data = data;

  DoubleNode *current = list->head;
  for (int i = 0; i < position; i++) {
    current = current->next;
  }

  new_node->next = current;
  new_node->prev = current->prev;
  current->prev->next = new_node;
  current->prev = new_node;

  list->size++;
  return true;
}

bool delete_double_at_position(DoubleList *list, int position) {
  if (!list || !list->head || position < 0 || position >= list->size)
    return false;

  DoubleNode *to_delete = get_double_node_at(list, position);
  if (!to_delete)
    return false;

  if (to_delete->prev) {
    to_delete->prev->next = to_delete->next;
  } else {
    list->head = to_delete->next;
  }

  if (to_delete->next) {
    to_delete->next->prev = to_delete->prev;
  } else {
    list->tail = to_delete->prev;
  }

  if (list->type == TYPE_STRING && to_delete->data.string_val) {
    free(to_delete->data.string_val);
  }
  free(to_delete);
  list->size--;
  return true;
}

bool modify_double_at_position(DoubleList *list, NodeData data, int position) {
  if (!list || position < 0 || position >= list->size)
    return false;

  DoubleNode *node = get_double_node_at(list, position);
  if (!node)
    return false;

  if (list->type == TYPE_STRING && node->data.string_val) {
    free(node->data.string_val);
  }
  node->data = data;
  return true;
}

DoubleNode *get_double_node_at(DoubleList *list, int position) {
  if (!list || position < 0 || position >= list->size)
    return NULL;

  DoubleNode *current = list->head;
  for (int i = 0; i < position; i++) {
    current = current->next;
  }
  return current;
}

// ============================================================================
// Fonctions de tri pour listes simples
// ============================================================================

void bubble_sort_simple(SimpleList *list) {
  if (!list || !list->head || list->size < 2)
    return;

  bool swapped;
  do {
    swapped = false;
    SimpleNode *current = list->head;
    while (current->next) {
      if (compare_node_data(current->data, current->next->data, list->type) >
          0) {
        swap_simple_node_data(current, current->next);
        swapped = true;
      }
      current = current->next;
    }
  } while (swapped);
}

void insertion_sort_simple(SimpleList *list) {
  if (!list || !list->head || list->size < 2)
    return;

  SimpleNode *sorted = NULL;
  SimpleNode *current = list->head;

  while (current) {
    SimpleNode *next = current->next;

    if (!sorted ||
        compare_node_data(current->data, sorted->data, list->type) <= 0) {
      current->next = sorted;
      sorted = current;
    } else {
      SimpleNode *search = sorted;
      while (search->next &&
             compare_node_data(current->data, search->next->data, list->type) >
                 0) {
        search = search->next;
      }
      current->next = search->next;
      search->next = current;
    }
    current = next;
  }
  list->head = sorted;
}

void selection_sort_simple(SimpleList *list) {
  if (!list || !list->head || list->size < 2)
    return;

  SimpleNode *current = list->head;
  while (current) {
    SimpleNode *min = current;
    SimpleNode *search = current->next;

    while (search) {
      if (compare_node_data(search->data, min->data, list->type) < 0) {
        min = search;
      }
      search = search->next;
    }

    if (min != current) {
      swap_simple_node_data(current, min);
    }
    current = current->next;
  }
}

// ============================================================================
// Fonctions de tri pour listes doubles
// ============================================================================

void bubble_sort_double(DoubleList *list) {
  if (!list || !list->head || list->size < 2)
    return;

  bool swapped;
  do {
    swapped = false;
    DoubleNode *current = list->head;
    while (current->next) {
      if (compare_node_data(current->data, current->next->data, list->type) >
          0) {
        swap_double_node_data(current, current->next);
        swapped = true;
      }
      current = current->next;
    }
  } while (swapped);
}

void insertion_sort_double(DoubleList *list) {
  if (!list || !list->head || list->size < 2)
    return;

  DoubleNode *current = list->head->next;
  while (current) {
    DoubleNode *next = current->next;
    DoubleNode *search = current->prev;

    while (search &&
           compare_node_data(current->data, search->data, list->type) < 0) {
      search = search->prev;
    }

    if (search != current->prev) {
      // Retirer current de sa position
      current->prev->next = current->next;
      if (current->next) {
        current->next->prev = current->prev;
      } else {
        list->tail = current->prev;
      }

      // Insérer après search
      if (search) {
        current->next = search->next;
        current->prev = search;
        search->next->prev = current;
        search->next = current;
      } else {
        current->next = list->head;
        current->prev = NULL;
        list->head->prev = current;
        list->head = current;
      }
    }
    current = next;
  }
}

void selection_sort_double(DoubleList *list) {
  if (!list || !list->head || list->size < 2)
    return;

  DoubleNode *current = list->head;
  while (current) {
    DoubleNode *min = current;
    DoubleNode *search = current->next;

    while (search) {
      if (compare_node_data(search->data, min->data, list->type) < 0) {
        min = search;
      }
      search = search->next;
    }

    if (min != current) {
      swap_double_node_data(current, min);
    }
    current = current->next;
  }
}

// ============================================================================
// Fonctions utilitaires
// ============================================================================

int compare_node_data(NodeData a, NodeData b, DataType type) {
  switch (type) {
  case TYPE_INT:
    return a.int_val - b.int_val;
  case TYPE_FLOAT:
    if (a.float_val < b.float_val)
      return -1;
    if (a.float_val > b.float_val)
      return 1;
    return 0;
  case TYPE_CHAR:
    return a.char_val - b.char_val;
  case TYPE_STRING:
    return strcmp(a.string_val, b.string_val);
  }
  return 0;
}

void swap_simple_node_data(SimpleNode *a, SimpleNode *b) {
  NodeData temp = a->data;
  a->data = b->data;
  b->data = temp;
}

void swap_double_node_data(DoubleNode *a, DoubleNode *b) {
  NodeData temp = a->data;
  a->data = b->data;
  b->data = temp;
}

char *node_data_to_string(NodeData data, DataType type) {
  char *str = (char *)malloc(64);
  if (!str)
    return NULL;

  switch (type) {
  case TYPE_INT:
    snprintf(str, 64, "%d", data.int_val);
    break;
  case TYPE_FLOAT:
    snprintf(str, 64, "%.2f", data.float_val);
    break;
  case TYPE_CHAR:
    snprintf(str, 64, "%c", data.char_val);
    break;
  case TYPE_STRING:
    snprintf(str, 64, "%s", data.string_val);
    break;
  }
  return str;
}

// ============================================================================
// Génération de données aléatoires
// ============================================================================

NodeData generate_random_node_data(DataType type) {
  NodeData data;
  static bool seeded = false;

  if (!seeded) {
    srand((unsigned int)time(NULL));
    seeded = true;
  }

  switch (type) {
  case TYPE_INT:
    data.int_val = rand() % 1000;
    break;
  case TYPE_FLOAT:
    data.float_val = (float)(rand() % 10000) / 100.0f;
    break;
  case TYPE_CHAR:
    data.char_val = 'A' + (rand() % 26);
    break;
  case TYPE_STRING: {
    int len = 3 + (rand() % 5);
    data.string_val = (char *)malloc(len + 1);
    for (int i = 0; i < len; i++) {
      data.string_val[i] = 'a' + (rand() % 26);
    }
    data.string_val[len] = '\0';
    break;
  }
  }
  return data;
}

void fill_simple_list_random(SimpleList *list, int count) {
  if (!list)
    return;

  for (int i = 0; i < count; i++) {
    NodeData data = generate_random_node_data(list->type);
    insert_simple_at_end(list, data);
  }
}

void fill_double_list_random(DoubleList *list, int count) {
  if (!list)
    return;

  for (int i = 0; i < count; i++) {
    NodeData data = generate_random_node_data(list->type);
    insert_double_at_end(list, data);
  }
}
