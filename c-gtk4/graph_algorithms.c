#include "graph_algorithms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


// ============================================================================
// Fonctions de création/destruction
// ============================================================================

Graph *graph_create(int num_vertices, DataType data_type) {
  if (num_vertices <= 0 || num_vertices > MAX_VERTICES) {
    return NULL;
  }

  Graph *graph = (Graph *)malloc(sizeof(Graph));
  if (!graph)
    return NULL;

  graph->num_vertices = num_vertices;
  graph->num_edges = 0;
  graph->data_type = data_type;

  // Initialiser les sommets
  for (int i = 0; i < num_vertices; i++) {
    graph->nodes[i].id = i;
    memset(&graph->nodes[i].value, 0, sizeof(NodeValue));
    graph->nodes[i].x = 0;
    graph->nodes[i].y = 0;

    // Valeurs par défaut selon le type
    switch (data_type) {
    case DATA_TYPE_INT:
      graph->nodes[i].value.int_val = i;
      break;
    case DATA_TYPE_DOUBLE:
      graph->nodes[i].value.double_val = (double)i;
      break;
    case DATA_TYPE_CHAR:
      graph->nodes[i].value.char_val = 'A' + i;
      break;
    case DATA_TYPE_STRING:
      snprintf(graph->nodes[i].value.string_val, 64, "S%d", i);
      break;
    }
  }

  // Initialiser la matrice d'adjacence (pas d'arêtes)
  for (int i = 0; i < MAX_VERTICES; i++) {
    for (int j = 0; j < MAX_VERTICES; j++) {
      graph->adj_matrix[i][j] = (i == j) ? 0 : INF;
    }
  }

  return graph;
}

void graph_destroy(Graph *graph) {
  if (graph) {
    free(graph);
  }
}

void graph_reset(Graph *graph) {
  if (!graph)
    return;

  graph->num_edges = 0;
  for (int i = 0; i < MAX_VERTICES; i++) {
    for (int j = 0; j < MAX_VERTICES; j++) {
      graph->adj_matrix[i][j] = (i == j) ? 0 : INF;
    }
  }
}

// ============================================================================
// Gestion des arêtes
// ============================================================================

void graph_add_edge(Graph *graph, int from, int to, double weight) {
  if (!graph || from < 0 || from >= graph->num_vertices || to < 0 ||
      to >= graph->num_vertices) {
    return;
  }

  // Vérifier si l'arête existe déjà
  if (graph->adj_matrix[from][to] == INF) {
    graph->edges[graph->num_edges].from = from;
    graph->edges[graph->num_edges].to = to;
    graph->edges[graph->num_edges].weight = weight;
    graph->num_edges++;
  }

  graph->adj_matrix[from][to] = weight;
  graph->adj_matrix[to][from] = weight; // Graphe non-orienté
}

void graph_remove_edge(Graph *graph, int from, int to) {
  if (!graph || from < 0 || from >= graph->num_vertices || to < 0 ||
      to >= graph->num_vertices) {
    return;
  }

  graph->adj_matrix[from][to] = INF;
  graph->adj_matrix[to][from] = INF;
}

bool graph_has_edge(Graph *graph, int from, int to) {
  if (!graph || from < 0 || from >= graph->num_vertices || to < 0 ||
      to >= graph->num_vertices) {
    return false;
  }
  return graph->adj_matrix[from][to] != INF && from != to;
}

double graph_get_weight(Graph *graph, int from, int to) {
  if (!graph || from < 0 || from >= graph->num_vertices || to < 0 ||
      to >= graph->num_vertices) {
    return INF;
  }
  return graph->adj_matrix[from][to];
}

// ============================================================================
// Fonctions pour les valeurs des sommets
// ============================================================================

void graph_set_node_value_int(Graph *graph, int node_id, int value) {
  if (graph && node_id >= 0 && node_id < graph->num_vertices) {
    graph->nodes[node_id].value.int_val = value;
  }
}

void graph_set_node_value_double(Graph *graph, int node_id, double value) {
  if (graph && node_id >= 0 && node_id < graph->num_vertices) {
    graph->nodes[node_id].value.double_val = value;
  }
}

void graph_set_node_value_char(Graph *graph, int node_id, char value) {
  if (graph && node_id >= 0 && node_id < graph->num_vertices) {
    graph->nodes[node_id].value.char_val = value;
  }
}

void graph_set_node_value_string(Graph *graph, int node_id, const char *value) {
  if (graph && node_id >= 0 && node_id < graph->num_vertices && value) {
    strncpy(graph->nodes[node_id].value.string_val, value, 63);
    graph->nodes[node_id].value.string_val[63] = '\0';
  }
}

const char *graph_get_node_label(Graph *graph, int node_id, char *buffer,
                                 size_t buffer_size) {
  if (!graph || node_id < 0 || node_id >= graph->num_vertices || !buffer) {
    return "";
  }

  switch (graph->data_type) {
  case DATA_TYPE_INT:
    snprintf(buffer, buffer_size, "%d", graph->nodes[node_id].value.int_val);
    break;
  case DATA_TYPE_DOUBLE:
    snprintf(buffer, buffer_size, "%.1f",
             graph->nodes[node_id].value.double_val);
    break;
  case DATA_TYPE_CHAR:
    snprintf(buffer, buffer_size, "%c", graph->nodes[node_id].value.char_val);
    break;
  case DATA_TYPE_STRING:
    snprintf(buffer, buffer_size, "%s", graph->nodes[node_id].value.string_val);
    break;
  }

  return buffer;
}

// ============================================================================
// Algorithme de Dijkstra
// ============================================================================

PathResult dijkstra(Graph *graph, int start, int end) {
  PathResult result = {0};
  result.found = false;
  result.distance = INF;
  result.path_length = 0;
  result.has_negative_cycle = false;

  if (!graph || start < 0 || start >= graph->num_vertices || end < 0 ||
      end >= graph->num_vertices) {
    return result;
  }

  clock_t start_time = clock();

  int n = graph->num_vertices;
  double dist[MAX_VERTICES];
  int prev[MAX_VERTICES];
  bool visited[MAX_VERTICES];

  // Initialisation
  for (int i = 0; i < n; i++) {
    dist[i] = INF;
    prev[i] = -1;
    visited[i] = false;
  }
  dist[start] = 0;

  // Algorithme principal
  for (int count = 0; count < n; count++) {
    // Trouver le sommet non visité avec la plus petite distance
    int u = -1;
    double min_dist = INF;
    for (int i = 0; i < n; i++) {
      if (!visited[i] && dist[i] < min_dist) {
        min_dist = dist[i];
        u = i;
      }
    }

    if (u == -1 || u == end)
      break;

    visited[u] = true;

    // Relaxation des arêtes
    for (int v = 0; v < n; v++) {
      if (!visited[v] && graph->adj_matrix[u][v] != INF) {
        double new_dist = dist[u] + graph->adj_matrix[u][v];
        if (new_dist < dist[v]) {
          dist[v] = new_dist;
          prev[v] = u;
        }
      }
    }
  }

  // Reconstruction du chemin
  if (dist[end] != INF) {
    result.found = true;
    result.distance = dist[end];

    // Reconstruire le chemin en remontant
    int path_temp[MAX_VERTICES];
    int len = 0;
    int current = end;

    while (current != -1) {
      path_temp[len++] = current;
      current = prev[current];
    }

    // Inverser le chemin
    result.path_length = len;
    for (int i = 0; i < len; i++) {
      result.path[i] = path_temp[len - 1 - i];
    }
  }

  clock_t end_time = clock();
  result.execution_time_ms =
      ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

  return result;
}

// ============================================================================
// Algorithme de Bellman-Ford
// ============================================================================

PathResult bellman_ford(Graph *graph, int start, int end) {
  PathResult result = {0};
  result.found = false;
  result.distance = INF;
  result.path_length = 0;
  result.has_negative_cycle = false;

  if (!graph || start < 0 || start >= graph->num_vertices || end < 0 ||
      end >= graph->num_vertices) {
    return result;
  }

  clock_t start_time = clock();

  int n = graph->num_vertices;
  double dist[MAX_VERTICES];
  int prev[MAX_VERTICES];

  // Initialisation
  for (int i = 0; i < n; i++) {
    dist[i] = INF;
    prev[i] = -1;
  }
  dist[start] = 0;

  // Relaxation des arêtes n-1 fois
  for (int i = 0; i < n - 1; i++) {
    bool updated = false;
    for (int u = 0; u < n; u++) {
      if (dist[u] == INF)
        continue;
      for (int v = 0; v < n; v++) {
        if (graph->adj_matrix[u][v] != INF && u != v) {
          double new_dist = dist[u] + graph->adj_matrix[u][v];
          if (new_dist < dist[v]) {
            dist[v] = new_dist;
            prev[v] = u;
            updated = true;
          }
        }
      }
    }
    if (!updated)
      break;
  }

  // Détection des cycles négatifs
  for (int u = 0; u < n; u++) {
    if (dist[u] == INF)
      continue;
    for (int v = 0; v < n; v++) {
      if (graph->adj_matrix[u][v] != INF && u != v) {
        if (dist[u] + graph->adj_matrix[u][v] < dist[v]) {
          result.has_negative_cycle = true;
          clock_t end_time = clock();
          result.execution_time_ms =
              ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;
          return result;
        }
      }
    }
  }

  // Reconstruction du chemin
  if (dist[end] != INF) {
    result.found = true;
    result.distance = dist[end];

    int path_temp[MAX_VERTICES];
    int len = 0;
    int current = end;

    while (current != -1 && len < MAX_VERTICES) {
      path_temp[len++] = current;
      current = prev[current];
    }

    result.path_length = len;
    for (int i = 0; i < len; i++) {
      result.path[i] = path_temp[len - 1 - i];
    }
  }

  clock_t end_time = clock();
  result.execution_time_ms =
      ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

  return result;
}

// ============================================================================
// Algorithme de Floyd-Warshall
// ============================================================================

PathResult floyd_warshall(Graph *graph, int start, int end) {
  PathResult result = {0};
  result.found = false;
  result.distance = INF;
  result.path_length = 0;
  result.has_negative_cycle = false;

  if (!graph || start < 0 || start >= graph->num_vertices || end < 0 ||
      end >= graph->num_vertices) {
    return result;
  }

  clock_t start_time = clock();

  int n = graph->num_vertices;
  double dist[MAX_VERTICES][MAX_VERTICES];
  int next[MAX_VERTICES][MAX_VERTICES];

  // Initialisation
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      dist[i][j] = graph->adj_matrix[i][j];
      if (graph->adj_matrix[i][j] != INF && i != j) {
        next[i][j] = j;
      } else {
        next[i][j] = -1;
      }
    }
  }

  // Algorithme principal
  for (int k = 0; k < n; k++) {
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        if (dist[i][k] != INF && dist[k][j] != INF) {
          double new_dist = dist[i][k] + dist[k][j];
          if (new_dist < dist[i][j]) {
            dist[i][j] = new_dist;
            next[i][j] = next[i][k];
          }
        }
      }
    }
  }

  // Vérification des cycles négatifs (diagonale négative)
  for (int i = 0; i < n; i++) {
    if (dist[i][i] < 0) {
      result.has_negative_cycle = true;
      clock_t end_time = clock();
      result.execution_time_ms =
          ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;
      return result;
    }
  }

  // Reconstruction du chemin
  if (dist[start][end] != INF) {
    result.found = true;
    result.distance = dist[start][end];

    int current = start;
    result.path[0] = start;
    result.path_length = 1;

    while (current != end && result.path_length < MAX_VERTICES) {
      current = next[current][end];
      if (current == -1) {
        result.found = false;
        result.path_length = 0;
        break;
      }
      result.path[result.path_length++] = current;
    }
  }

  clock_t end_time = clock();
  result.execution_time_ms =
      ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

  return result;
}

// ============================================================================
// Fonction utilitaire pour exécuter un algorithme par type
// ============================================================================

PathResult execute_algorithm(Graph *graph, int start, int end,
                             AlgorithmType algo) {
  switch (algo) {
  case ALGO_DIJKSTRA:
    return dijkstra(graph, start, end);
  case ALGO_BELLMAN_FORD:
    return bellman_ford(graph, start, end);
  case ALGO_FLOYD_WARSHALL:
    return floyd_warshall(graph, start, end);
  default:
    return dijkstra(graph, start, end);
  }
}
