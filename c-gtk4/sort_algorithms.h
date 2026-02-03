#ifndef SORT_ALGORITHMS_H
#define SORT_ALGORITHMS_H

#include <stddef.h>

// Types de données supportés
typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_CHAR, TYPE_STRING } DataType;

// Algorithmes de tri supportés
typedef enum { ALGO_BUBBLE, ALGO_INSERTION, ALGO_SHELL, ALGO_QUICK } SortAlgo;

// Structure unique pour passer les données
typedef struct {
  void *array;   // Pointeur vers le début du tableau
  size_t size;   // Nombre d'éléments
  DataType type; // Type des éléments
} ArrayData;

// Génère des données aléatoires
void generate_random_data(ArrayData *data, size_t size, DataType type);

// Libère la mémoire du tableau
void free_array_data(ArrayData *data);

// Fonction de tri générique wrapper
// Retourne le temps d'exécution en secondes
double sort_array(ArrayData *data, SortAlgo algo);

// Convertit le tableau en chaîne de caractères pour affichage
char *array_to_string(const ArrayData *data);

#endif
