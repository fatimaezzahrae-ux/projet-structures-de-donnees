#ifndef GRAPH_ALGORITHMS_H
#define GRAPH_ALGORITHMS_H

#include <float.h>
#include <limits.h>
#include <stdbool.h>


#define MAX_VERTICES 50
#define INF DBL_MAX

// Types de données pour les sommets
typedef enum {
  DATA_TYPE_INT,
  DATA_TYPE_DOUBLE,
  DATA_TYPE_CHAR,
  DATA_TYPE_STRING
} DataType;

// Types d'algorithmes
typedef enum {
  ALGO_DIJKSTRA,
  ALGO_BELLMAN_FORD,
  ALGO_FLOYD_WARSHALL
} AlgorithmType;

// Valeur d'un sommet (union pour différents types)
typedef union {
  int int_val;
  double double_val;
  char char_val;
  char string_val[64];
} NodeValue;

// Structure d'un sommet
typedef struct {
  int id;
  NodeValue value;
  double x, y; // Position pour le dessin
} GraphNode;

// Structure d'une arête
typedef struct {
  int from;
  int to;
  double weight;
} GraphEdge;

// Structure principale du graphe
typedef struct {
  int num_vertices;
  int num_edges;
  DataType data_type;
  GraphNode nodes[MAX_VERTICES];
  double adj_matrix[MAX_VERTICES]
                   [MAX_VERTICES]; // Matrice d'adjacence avec poids
  GraphEdge edges[MAX_VERTICES * MAX_VERTICES];
} Graph;

// Résultat d'un algorithme de plus court chemin
typedef struct {
  bool found;
  double distance;
  int path[MAX_VERTICES];
  int path_length;
  double execution_time_ms;
  bool has_negative_cycle; // Pour Bellman-Ford
} PathResult;

// Fonctions de création/destruction
Graph *graph_create(int num_vertices, DataType data_type);
void graph_destroy(Graph *graph);
void graph_reset(Graph *graph);

// Gestion des arêtes
void graph_add_edge(Graph *graph, int from, int to, double weight);
void graph_remove_edge(Graph *graph, int from, int to);
bool graph_has_edge(Graph *graph, int from, int to);
double graph_get_weight(Graph *graph, int from, int to);

// Fonctions pour les valeurs des sommets
void graph_set_node_value_int(Graph *graph, int node_id, int value);
void graph_set_node_value_double(Graph *graph, int node_id, double value);
void graph_set_node_value_char(Graph *graph, int node_id, char value);
void graph_set_node_value_string(Graph *graph, int node_id, const char *value);
const char *graph_get_node_label(Graph *graph, int node_id, char *buffer,
                                 size_t buffer_size);

// Algorithmes de plus court chemin
PathResult dijkstra(Graph *graph, int start, int end);
PathResult bellman_ford(Graph *graph, int start, int end);
PathResult floyd_warshall(Graph *graph, int start, int end);

// Fonction utilitaire pour exécuter un algorithme par type
PathResult execute_algorithm(Graph *graph, int start, int end,
                             AlgorithmType algo);

#endif // GRAPH_ALGORITHMS_H
