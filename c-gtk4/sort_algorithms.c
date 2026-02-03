#include "sort_algorithms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- Utilitaires de comparaison ---
int compare_int(const void *a, const void *b) {
  return (*(int *)a - *(int *)b);
}
int compare_float(const void *a, const void *b) {
  return (*(float *)a < *(float *)b) ? -1 : (*(float *)a > *(float *)b);
}
int compare_char(const void *a, const void *b) {
  return (*(char *)a - *(char *)b);
}
int compare_string(const void *a, const void *b) {
  return strcmp(*(char **)a, *(char **)b);
}

// --- Implémentations spécifiques (Exemple simplifié pour INT) ---
// Note: Pour un vrai système complet, on utiliserait des templates ou macros,
// mais ici on va dupliquer un peu ou utiliser void* avec memcpy pour qsort.

// -- BULLE --
void bubble_sort_int(int *arr, size_t size) {
  for (size_t i = 0; i < size - 1; i++) {
    for (size_t j = 0; j < size - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        int temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}

// -- INSERTION --
void insertion_sort_int(int *arr, size_t size) {
  for (size_t i = 1; i < size; i++) {
    int key = arr[i];
    int j = i - 1;
    while (j >= 0 && arr[j] > key) {
      arr[j + 1] = arr[j];
      j = j - 1;
    }
    arr[j + 1] = key;
  }
}

// -- SHELL --
void shell_sort_int(int *arr, size_t size) {
  for (size_t gap = size / 2; gap > 0; gap /= 2) {
    for (size_t i = gap; i < size; i += 1) {
      int temp = arr[i];
      size_t j;
      for (j = i; j >= gap && arr[j - gap] > temp; j -= gap)
        arr[j] = arr[j - gap];
      arr[j] = temp;
    }
  }
}

// -- QUICK (Wrapper qsort standard pour simplicité et performance) --
void quick_sort_int(int *arr, size_t size) {
  qsort(arr, size, sizeof(int), compare_int);
}

// --- Fonctions publiques ---

void generate_random_data(ArrayData *data, size_t size, DataType type) {
  if (data->array)
    free(data->array);
  data->size = size;
  data->type = type;

  // Seed du random
  srand((unsigned int)time(NULL));

  switch (type) {
  case TYPE_INT:
    data->array = malloc(size * sizeof(int));
    for (size_t i = 0; i < size; i++)
      ((int *)data->array)[i] = rand() % 1000;
    break;
  case TYPE_FLOAT:
    data->array = malloc(size * sizeof(float));
    for (size_t i = 0; i < size; i++)
      ((float *)data->array)[i] = (float)rand() / (float)(RAND_MAX / 1000.0);
    break;
  case TYPE_CHAR:
    data->array = malloc(size * sizeof(char));
    for (size_t i = 0; i < size; i++)
      ((char *)data->array)[i] = 'A' + (rand() % 26);
    break;
  case TYPE_STRING:
    // Pas implémenté pour l'instant pour garder simple (gestion mémoire
    // complexe) Fallback sur INT
    data->type = TYPE_INT;
    data->array = malloc(size * sizeof(int));
    for (size_t i = 0; i < size; i++)
      ((int *)data->array)[i] = rand() % 1000;
    break;
  }
}

void free_array_data(ArrayData *data) {
  if (data->array) {
    free(data->array);
    data->array = NULL;
  }
  data->size = 0;
}

double sort_array(ArrayData *data, SortAlgo algo) {
  clock_t start = clock();

  // NOTE: Ici on ne traite que les INT pour l'exemple du prototype.
  // Il faudrait ajouter des switch cases pour FLOAT, CHAR, etc.
  if (data->type == TYPE_INT) {
    int *arr = (int *)data->array;
    switch (algo) {
    case ALGO_BUBBLE:
      bubble_sort_int(arr, data->size);
      break;
    case ALGO_INSERTION:
      insertion_sort_int(arr, data->size);
      break;
    case ALGO_SHELL:
      shell_sort_int(arr, data->size);
      break;
    case ALGO_QUICK:
      quick_sort_int(arr, data->size);
      break;
    }
  }

  clock_t end = clock();
  return ((double)(end - start)) / CLOCKS_PER_SEC;
}

char *array_to_string(const ArrayData *data) {
  // Allocation d'un grand buffer
  // On estime ~12 chars par nombre + virgule (max int 10 chars + signes)
  size_t buffer_size = data->size * 15 + 100;
  char *buffer = malloc(buffer_size);
  if (!buffer)
    return NULL;

  char *ptr = buffer; // Pointeur courant pour écriture efficace
  *ptr = '\0';

  // Affichage complet (aucune troncation demandée)

  if (data->type == TYPE_INT) {
    int *arr = (int *)data->array;
    for (size_t i = 0; i < data->size; i++) {
      // Imprime directement à la position ptr
      int written = sprintf(ptr, "%d", arr[i]);
      if (written > 0)
        ptr += written;

      if (i < data->size - 1) {
        if ((i + 1) % 100 == 0) { // Saut de ligne toutes les 100 valeurs
          strcpy(ptr, "\n");
          ptr += 1;
        } else {
          strcpy(ptr, ", ");
          ptr += 2;
        }
      }
    }
  } else if (data->type == TYPE_FLOAT) {
    float *arr = (float *)data->array;
    for (size_t i = 0; i < data->size; i++) {
      int written = sprintf(ptr, "%.2f", arr[i]);
      if (written > 0)
        ptr += written;

      if (i < data->size - 1) {
        if ((i + 1) % 80 == 0) {
          strcpy(ptr, "\n");
          ptr += 1;
        } else {
          strcpy(ptr, ", ");
          ptr += 2;
        }
      }
    }
  } else if (data->type == TYPE_CHAR) {
    char *arr = (char *)data->array;
    for (size_t i = 0; i < data->size; i++) {
      int written = sprintf(ptr, "%c", arr[i]);
      if (written > 0)
        ptr += written;

      if (i < data->size - 1) {
        if ((i + 1) % 150 == 0) {
          strcpy(ptr, "\n");
          ptr += 1;
        } else {
          strcpy(ptr, ", ");
          ptr += 2;
        }
      }
    }
  }

  return buffer;
}
