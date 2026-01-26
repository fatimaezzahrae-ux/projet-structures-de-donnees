#include "graphs_window.h"
#include "graph_algorithms.h"
#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Structure pour gérer l'état de la fenêtre
typedef struct {
  GtkWidget *window;
  GtkWidget *drawing_area;
  GtkWidget *data_type_dropdown;
  GtkWidget *path_type_dropdown; // Nouveau dropdown pour le type de chemin
  GtkWidget *num_vertices_spin;
  GtkWidget *start_node_dropdown;
  GtkWidget *end_node_dropdown;
  GtkWidget *algorithm_dropdown;
  GtkWidget *result_label;
  GtkWidget *draw_graph_btn;
  GtkWidget *execute_btn;
  GtkWidget *filling_mode_dropdown; // Nouveau dropdown

  Graph *graph;
  bool graph_drawn;
  int drag_start_node;   // Sommet de départ du drag (-1 si aucun)
  int drag_end_node;     // Sommet de fin du drag (-1 si aucun)
  bool is_dragging;      // En train de dessiner une arête
  double drag_x, drag_y; // Position actuelle de la souris pendant le drag
  PathResult last_result;

  // Pour le dessin
  double center_x;
  double center_y;
  double radius;
} GraphsWindowData;

// Forward declarations of helper functions
static void fill_graph_random(GraphsWindowData *data);
static void fill_graph_manual(GraphsWindowData *data);
static void refresh_node_dropdowns(GraphsWindowData *data);

// ============================================================================
// Fonctions utilitaires
// ============================================================================

static void calculate_node_positions(GraphsWindowData *data, int width,
                                     int height) {
  if (!data->graph)
    return;

  data->center_x = width / 2.0;
  data->center_y = height / 2.0;
  data->radius = (width < height ? width : height) * 0.35;

  int n = data->graph->num_vertices;
  for (int i = 0; i < n; i++) {
    double angle = 2.0 * M_PI * i / n - M_PI / 2;
    data->graph->nodes[i].x = data->center_x + data->radius * cos(angle);
    data->graph->nodes[i].y = data->center_y + data->radius * sin(angle);
  }
}

static int find_node_at_position(GraphsWindowData *data, double x, double y) {
  if (!data->graph)
    return -1;

  const double node_radius = 25.0;

  for (int i = 0; i < data->graph->num_vertices; i++) {
    double dx = x - data->graph->nodes[i].x;
    double dy = y - data->graph->nodes[i].y;
    double dist = sqrt(dx * dx + dy * dy);

    if (dist <= node_radius) {
      return i;
    }
  }

  return -1;
}

static bool is_edge_in_path(GraphsWindowData *data, int from, int to) {
  if (!data->last_result.found)
    return false;

  for (int i = 0; i < data->last_result.path_length - 1; i++) {
    int a = data->last_result.path[i];
    int b = data->last_result.path[i + 1];
    if ((a == from && b == to) || (a == to && b == from)) {
      return true;
    }
  }

  return false;
}

// ============================================================================
// Fonction de dessin avec Cairo
// ============================================================================

static void draw_graph(GtkDrawingArea *area, cairo_t *cr, int width, int height,
                       gpointer user_data) {
  GraphsWindowData *data = (GraphsWindowData *)user_data;

  // Fond blanc
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_paint(cr);

  if (!data->graph || !data->graph_drawn) {
    // Message instructif
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 16);

    const char *msg =
        "Configurez et cliquez sur 'Dessiner Graphe' pour commencer";
    cairo_text_extents_t extents;
    cairo_text_extents(cr, msg, &extents);

    cairo_move_to(cr, (width - extents.width) / 2, height / 2);
    cairo_show_text(cr, msg);
    return;
  }

  calculate_node_positions(data, width, height);

  const double node_radius = 25.0;

  // Dessiner les arêtes
  for (int i = 0; i < data->graph->num_edges; i++) {
    GraphEdge *edge = &data->graph->edges[i];
    GraphNode *from = &data->graph->nodes[edge->from];
    GraphNode *to = &data->graph->nodes[edge->to];

    // Couleur de l'arête (rouge si dans le chemin, gris sinon)
    bool in_path = is_edge_in_path(data, edge->from, edge->to);
    if (in_path) {
      cairo_set_source_rgb(cr, 0.9, 0.1, 0.1); // Rouge
      cairo_set_line_width(cr, 3.0);
    } else {
      cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
      cairo_set_line_width(cr, 2.0);
    }

    // Récupérer le type de chemin (0 = Orienté, 1 = Non orienté)
    int path_type = 0;
    if (data->path_type_dropdown) {
      path_type =
          gtk_drop_down_get_selected(GTK_DROP_DOWN(data->path_type_dropdown));
    }

    double start_x = from->x;
    double start_y = from->y;
    double end_x = to->x;
    double end_y = to->y;

    // Calculer la géométrie pour les flèches si nécessaire
    double dx = to->x - from->x;
    double dy = to->y - from->y;
    double length = sqrt(dx * dx + dy * dy);

    // Si orienté, on arrête le trait au bord du cercle pour dessiner la flèche
    if (path_type == 0 && length > 0) {
      double u_x = dx / length;
      double u_y = dy / length;
      // Le rayon est 25, on s'arrête un peu avant pour que la pointe touche le
      // cercle
      end_x = to->x - (node_radius * u_x);
      end_y = to->y - (node_radius * u_y);
    }

    cairo_move_to(cr, start_x, start_y);
    cairo_line_to(cr, end_x, end_y);
    cairo_stroke(cr);

    // Dessiner la flèche si orienté
    if (path_type == 0 && length > 0) {
      double angle = atan2(dy, dx);
      double arrow_len = 15.0;
      double arrow_angle = M_PI / 6; // 30 degrés

      cairo_move_to(cr, end_x, end_y);
      cairo_line_to(cr, end_x - arrow_len * cos(angle - arrow_angle),
                    end_y - arrow_len * sin(angle - arrow_angle));

      cairo_move_to(cr, end_x, end_y);
      cairo_line_to(cr, end_x - arrow_len * cos(angle + arrow_angle),
                    end_y - arrow_len * sin(angle + arrow_angle));
      cairo_stroke(cr);
    }

    // Afficher le poids AU-DESSUS de l'arête
    double mid_x = (from->x + to->x) / 2;
    double mid_y = (from->y + to->y) / 2;

    // Calculer le vecteur perpendiculaire pour placer le texte au-dessus
    double perp_x = -dy / length; // Vecteur perpendiculaire
    double perp_y = dx / length;

    // Décalage de 15 pixels au-dessus de l'arête
    double text_x = mid_x + perp_x * 15;
    double text_y = mid_y + perp_y * 15;

    char weight_str[32];
    snprintf(weight_str, sizeof(weight_str), "%.1f", edge->weight);

    // Fond blanc pour le texte
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_arc(cr, text_x, text_y, 12, 0, 2 * M_PI);
    cairo_fill(cr);

    // Texte du poids
    if (in_path) {
      cairo_set_source_rgb(cr, 0.9, 0.1, 0.1);
    } else {
      cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    }
    cairo_set_font_size(cr, 12);

    cairo_text_extents_t extents;
    cairo_text_extents(cr, weight_str, &extents);
    cairo_move_to(cr, text_x - extents.width / 2, text_y + extents.height / 2);
    cairo_show_text(cr, weight_str);
  }

  // Dessiner les sommets
  for (int i = 0; i < data->graph->num_vertices; i++) {
    GraphNode *node = &data->graph->nodes[i];

    // Déterminer la couleur du sommet
    bool is_start =
        (data->last_result.found && data->last_result.path_length > 0 &&
         data->last_result.path[0] == i);
    bool is_end =
        (data->last_result.found && data->last_result.path_length > 0 &&
         data->last_result.path[data->last_result.path_length - 1] == i);
    bool in_path = false;

    if (data->last_result.found) {
      for (int j = 0; j < data->last_result.path_length; j++) {
        if (data->last_result.path[j] == i) {
          in_path = true;
          break;
        }
      }
    }

    // Cercle du sommet
    if (is_start) {
      cairo_set_source_rgb(cr, 0.2, 0.7, 0.2); // Vert pour départ
    } else if (is_end) {
      cairo_set_source_rgb(cr, 0.9, 0.3, 0.1); // Orange pour arrivée
    } else if (in_path) {
      cairo_set_source_rgb(cr, 0.9, 0.6, 0.1); // Jaune pour chemin
    } else {
      cairo_set_source_rgb(cr, 0.38, 0.0, 0.92); // Violet (couleur principale)
    }

    cairo_arc(cr, node->x, node->y, node_radius, 0, 2 * M_PI);
    cairo_fill(cr);

    // Bordure
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_set_line_width(cr, 2.0);
    cairo_arc(cr, node->x, node->y, node_radius, 0, 2 * M_PI);
    cairo_stroke(cr);

    // Texte du label
    char label[64];
    graph_get_node_label(data->graph, i, label, sizeof(label));

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);

    cairo_text_extents_t extents;
    cairo_text_extents(cr, label, &extents);

    cairo_move_to(cr, node->x - extents.width / 2,
                  node->y + extents.height / 2);
    cairo_show_text(cr, label);
  }

  // Afficher la ligne de drag en cours
  if (data->is_dragging && data->drag_start_node >= 0) {
    GraphNode *start = &data->graph->nodes[data->drag_start_node];

    // Ligne en pointillé de couleur jaune
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 0.8);
    cairo_set_line_width(cr, 2.5);

    // Style pointillé
    double dashes[] = {5.0, 5.0};
    cairo_set_dash(cr, dashes, 2, 0);

    cairo_move_to(cr, start->x, start->y);
    cairo_line_to(cr, data->drag_x, data->drag_y);
    cairo_stroke(cr);

    // Réinitialiser le dash
    cairo_set_dash(cr, NULL, 0, 0);

    // Highlight du sommet de départ du drag
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 0.6);
    cairo_set_line_width(cr, 4.0);
    cairo_arc(cr, start->x, start->y, node_radius + 5, 0, 2 * M_PI);
    cairo_stroke(cr);

    // Highlight du sommet de fin si on survole un sommet
    if (data->drag_end_node >= 0) {
      GraphNode *end = &data->graph->nodes[data->drag_end_node];
      cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 0.6);
      cairo_set_line_width(cr, 4.0);
      cairo_arc(cr, end->x, end->y, node_radius + 5, 0, 2 * M_PI);
      cairo_stroke(cr);
    }
  }
}

// ============================================================================
// Popover pour saisir le poids d'une arête (plus simple qu'une fenêtre modale)
// ============================================================================

// Structure pour les données du popover
typedef struct {
  GraphsWindowData *graph_data;
  GtkWidget *popover;
  GtkWidget *entry;
  int from;
  int to;
} PopoverData;

// Callback pour OK
static void popover_on_ok_clicked(GtkWidget *widget, gpointer user_data) {
  PopoverData *pd = (PopoverData *)user_data;
  const char *text = gtk_editable_get_text(GTK_EDITABLE(pd->entry));

  double weight = atof(text);
  if (weight == 0.0 && text[0] != '0') {
    weight = 1.0; // Valeur par défaut si la saisie est invalide
  }

  graph_add_edge(pd->graph_data->graph, pd->from, pd->to, weight);
  gtk_widget_queue_draw(pd->graph_data->drawing_area);

  gtk_popover_popdown(GTK_POPOVER(pd->popover));
  g_free(pd);
}

// Callback pour Annuler
static void popover_on_cancel_clicked(GtkWidget *widget, gpointer user_data) {
  PopoverData *pd = (PopoverData *)user_data;
  gtk_popover_popdown(GTK_POPOVER(pd->popover));
  g_free(pd);
}

// Callback quand l'utilisateur appuie sur Entrée
static gboolean on_entry_activate(GtkEntry *entry, gpointer user_data) {
  popover_on_ok_clicked(NULL, user_data);
  return TRUE;
}

static void show_weight_popover(GraphsWindowData *data, int from, int to,
                                double x, double y) {
  // Créer le popover
  GtkWidget *popover = gtk_popover_new();
  gtk_widget_set_parent(popover, data->drawing_area);

  // Positionner le popover à l'endroit du clic
  GdkRectangle rect = {(int)x - 10, (int)y - 10, 20, 20};
  gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rect);

  // Créer le contenu
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_start(box, 15);
  gtk_widget_set_margin_end(box, 15);
  gtk_widget_set_margin_top(box, 15);
  gtk_widget_set_margin_bottom(box, 15);
  gtk_popover_set_child(GTK_POPOVER(popover), box);

  char label_text[128];
  char from_label[64], to_label[64];
  graph_get_node_label(data->graph, from, from_label, sizeof(from_label));
  graph_get_node_label(data->graph, to, to_label, sizeof(to_label));
  snprintf(label_text, sizeof(label_text), "Poids %s → %s:", from_label,
           to_label);

  GtkWidget *label = gtk_label_new(label_text);
  gtk_box_append(GTK_BOX(box), label);

  GtkWidget *entry = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(entry), "1.0");
  gtk_box_append(GTK_BOX(box), entry);

  GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(btn_box, GTK_ALIGN_END);
  gtk_box_append(GTK_BOX(box), btn_box);

  GtkWidget *ok_btn = gtk_button_new_with_label("OK");
  GtkWidget *cancel_btn = gtk_button_new_with_label("Annuler");
  gtk_box_append(GTK_BOX(btn_box), cancel_btn);
  gtk_box_append(GTK_BOX(btn_box), ok_btn);

  // Données pour les callbacks
  PopoverData *popover_data = g_new(PopoverData, 1);
  popover_data->graph_data = data;
  popover_data->popover = popover;
  popover_data->entry = entry;
  popover_data->from = from;
  popover_data->to = to;

  g_signal_connect(ok_btn, "clicked", G_CALLBACK(popover_on_ok_clicked),
                   popover_data);
  g_signal_connect(cancel_btn, "clicked", G_CALLBACK(popover_on_cancel_clicked),
                   popover_data);
  g_signal_connect(entry, "activate", G_CALLBACK(on_entry_activate),
                   popover_data);

  // Afficher le popover
  gtk_popover_popup(GTK_POPOVER(popover));

  // Focus sur l'entrée pour permettre la saisie immédiate
  gtk_widget_grab_focus(entry);
}

// ============================================================================
// Callbacks pour les interactions (DRAG pour créer les arêtes)
// ============================================================================

static void on_drag_begin(GtkGestureDrag *gesture, double x, double y,
                          gpointer user_data) {
  GraphsWindowData *data = (GraphsWindowData *)user_data;

  if (!data->graph_drawn)
    return;

  int node = find_node_at_position(data, x, y);

  if (node >= 0) {
    data->is_dragging = true;
    data->drag_start_node = node;
    data->drag_end_node = -1;
    data->drag_x = x;
    data->drag_y = y;
    gtk_widget_queue_draw(data->drawing_area);
  }
}

static void on_drag_update(GtkGestureDrag *gesture, double offset_x,
                           double offset_y, gpointer user_data) {
  GraphsWindowData *data = (GraphsWindowData *)user_data;

  if (!data->is_dragging)
    return;

  // Obtenir la position de départ du drag
  double start_x, start_y;
  gtk_gesture_drag_get_start_point(gesture, &start_x, &start_y);

  // Position actuelle
  data->drag_x = start_x + offset_x;
  data->drag_y = start_y + offset_y;

  // Vérifier si on survole un sommet
  data->drag_end_node = find_node_at_position(data, data->drag_x, data->drag_y);

  gtk_widget_queue_draw(data->drawing_area);
}

static void on_drag_end(GtkGestureDrag *gesture, double offset_x,
                        double offset_y, gpointer user_data) {
  GraphsWindowData *data = (GraphsWindowData *)user_data;

  if (!data->is_dragging)
    return;

  // Obtenir la position finale
  double start_x, start_y;
  gtk_gesture_drag_get_start_point(gesture, &start_x, &start_y);
  double end_x = start_x + offset_x;
  double end_y = start_y + offset_y;

  int end_node = find_node_at_position(data, end_x, end_y);

  // Si on a relâché sur un sommet différent, créer l'arête
  if (end_node >= 0 && end_node != data->drag_start_node) {
    // Vérifier si l'arête existe déjà
    if (!graph_has_edge(data->graph, data->drag_start_node, end_node)) {
      // Calculer le point milieu entre les deux sommets pour positionner le
      // popover
      GraphNode *from_n = &data->graph->nodes[data->drag_start_node];
      GraphNode *to_n = &data->graph->nodes[end_node];
      double mid_x = (from_n->x + to_n->x) / 2;
      double mid_y = (from_n->y + to_n->y) / 2;
      show_weight_popover(data, data->drag_start_node, end_node, mid_x, mid_y);
    }
  }

  // Réinitialiser l'état du drag
  data->is_dragging = false;
  data->drag_start_node = -1;
  data->drag_end_node = -1;

  gtk_widget_queue_draw(data->drawing_area);
}

// ============================================================================
// Callbacks pour les boutons
// ============================================================================

static void on_setting_changed(GObject *object, GParamSpec *pspec,
                               gpointer user_data) {
  GraphsWindowData *data = (GraphsWindowData *)user_data;
  gtk_widget_queue_draw(data->drawing_area);
}

static void on_clear_graph_clicked(GtkWidget *widget, gpointer user_data) {
  GraphsWindowData *data = (GraphsWindowData *)user_data;

  if (data->graph) {
    graph_destroy(data->graph);
    data->graph = NULL;
  }

  data->graph_drawn = false;
  data->is_dragging = false;
  data->drag_start_node = -1;
  data->drag_end_node = -1;
  data->last_result.found = false;

  // Vider les dropdowns de sommets
  GtkStringList *empty_list = gtk_string_list_new(NULL);
  gtk_drop_down_set_model(GTK_DROP_DOWN(data->start_node_dropdown),
                          G_LIST_MODEL(empty_list));

  GtkStringList *empty_list2 = gtk_string_list_new(NULL);
  gtk_drop_down_set_model(GTK_DROP_DOWN(data->end_node_dropdown),
                          G_LIST_MODEL(empty_list2));

  gtk_label_set_text(GTK_LABEL(data->result_label),
                     "Graphe vidé. Configurez et dessinez à nouveau.");

  gtk_widget_queue_draw(data->drawing_area);
}

static void on_draw_graph_clicked(GtkWidget *widget, gpointer user_data) {
  GraphsWindowData *data = (GraphsWindowData *)user_data;

  // Détruire l'ancien graphe si existant
  if (data->graph) {
    graph_destroy(data->graph);
  }

  // Récupérer les paramètres
  int num_vertices =
      (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->num_vertices_spin));
  DataType data_type = (DataType)gtk_drop_down_get_selected(
      GTK_DROP_DOWN(data->data_type_dropdown));

  // Créer le nouveau graphe
  data->graph = graph_create(num_vertices, data_type);
  data->graph_drawn = true;
  data->is_dragging = false;
  data->drag_start_node = -1;
  data->drag_end_node = -1;
  data->last_result.found = false;

  // Mettre à jour les dropdowns de sommets initialement (pour le cas Vide)
  GtkStringList *node_list = gtk_string_list_new(NULL);

  for (int i = 0; i < num_vertices; i++) {
    char label[64];
    graph_get_node_label(data->graph, i, label, sizeof(label));
    gtk_string_list_append(node_list, label);
  }

  gtk_drop_down_set_model(GTK_DROP_DOWN(data->start_node_dropdown),
                          G_LIST_MODEL(node_list));
  gtk_drop_down_set_model(GTK_DROP_DOWN(data->end_node_dropdown),
                          G_LIST_MODEL(node_list));

  gtk_drop_down_set_selected(GTK_DROP_DOWN(data->start_node_dropdown), 0);
  gtk_drop_down_set_selected(GTK_DROP_DOWN(data->end_node_dropdown),
                             num_vertices > 1 ? num_vertices - 1 : 0);

  // Récupérer le mode de remplissage
  int fill_mode = 0;
  if (data->filling_mode_dropdown) {
    fill_mode =
        gtk_drop_down_get_selected(GTK_DROP_DOWN(data->filling_mode_dropdown));
  }

  // Appliquer le remplissage si nécessaire
  if (fill_mode == 1) { // Aléatoire
    fill_graph_random(data);
    // Mettre à jour les dropdowns après remplissage aléatoire
    refresh_node_dropdowns(data);
    gtk_label_set_text(GTK_LABEL(data->result_label),
                       "Graphe créé et rempli aléatoirement. Cliquez sur deux "
                       "sommets pour créer une arête.");
  } else if (fill_mode == 2) { // Manuel
    gtk_label_set_text(GTK_LABEL(data->result_label),
                       "Graphe créé. Remplissage manuel en cours...");
    // Forcer le dessin maintenant pour voir les cercles vides en fond
    gtk_widget_queue_draw(data->drawing_area);
    while (g_main_context_iteration(NULL, FALSE))
      ;

    fill_graph_manual(data);
    // Note: refresh_node_dropdowns sera appelé dans on_manual_fill_validate
  } else {
    gtk_label_set_text(GTK_LABEL(data->result_label),
                       "Cliquez sur deux sommets pour créer une arête.");
  }

  gtk_widget_queue_draw(data->drawing_area);
}

static void on_execute_algorithm_clicked(GtkWidget *widget,
                                         gpointer user_data) {
  GraphsWindowData *data = (GraphsWindowData *)user_data;

  if (!data->graph || !data->graph_drawn) {
    gtk_label_set_text(GTK_LABEL(data->result_label),
                       "Veuillez d'abord dessiner un graphe.");
    return;
  }

  if (data->graph->num_edges == 0) {
    gtk_label_set_text(GTK_LABEL(data->result_label),
                       "Le graphe doit avoir au moins une arête.");
    return;
  }

  int start =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(data->start_node_dropdown));
  int end = gtk_drop_down_get_selected(GTK_DROP_DOWN(data->end_node_dropdown));
  AlgorithmType algo = (AlgorithmType)gtk_drop_down_get_selected(
      GTK_DROP_DOWN(data->algorithm_dropdown));

  // Exécuter l'algorithme
  data->last_result = execute_algorithm(data->graph, start, end, algo);

  // Afficher le résultat
  char result_text[512];

  if (data->last_result.has_negative_cycle) {
    snprintf(result_text, sizeof(result_text),
             "❌ Cycle négatif détecté !\nTemps: %.3f ms",
             data->last_result.execution_time_ms);
  } else if (!data->last_result.found) {
    snprintf(result_text, sizeof(result_text),
             "❌ Aucun chemin trouvé.\nTemps: %.3f ms",
             data->last_result.execution_time_ms);
  } else {
    // Construire le chemin en texte
    char path_str[256] = "";
    for (int i = 0; i < data->last_result.path_length; i++) {
      char label[64];
      graph_get_node_label(data->graph, data->last_result.path[i], label,
                           sizeof(label));
      strcat(path_str, label);
      if (i < data->last_result.path_length - 1) {
        strcat(path_str, " → ");
      }
    }

    snprintf(result_text, sizeof(result_text),
             "✓ Chemin trouvé !\nChemin: %s\nDistance: %.2f\nTemps: %.3f ms",
             path_str, data->last_result.distance,
             data->last_result.execution_time_ms);
  }

  gtk_label_set_text(GTK_LABEL(data->result_label), result_text);
  gtk_widget_queue_draw(data->drawing_area);
}

static void on_window_destroy(GtkWidget *widget, gpointer user_data) {
  GraphsWindowData *data = (GraphsWindowData *)user_data;

  if (data->graph) {
    graph_destroy(data->graph);
  }

  g_free(data);
}

// ============================================================================
// Helpers pour le Remplissage
// ============================================================================

// Helper pour rafraîchir les listes déroulantes des sommets
static void refresh_node_dropdowns(GraphsWindowData *data) {
  if (!data->graph)
    return;

  GtkStringList *node_list = gtk_string_list_new(NULL);

  for (int i = 0; i < data->graph->num_vertices; i++) {
    char label[64];
    graph_get_node_label(data->graph, i, label, sizeof(label));
    gtk_string_list_append(node_list, label);
  }

  // On sauvegarde les sélections actuelles si possible, sinon 0 et dernier
  int current_start =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(data->start_node_dropdown));
  int current_end =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(data->end_node_dropdown));

  gtk_drop_down_set_model(GTK_DROP_DOWN(data->start_node_dropdown),
                          G_LIST_MODEL(node_list));
  gtk_drop_down_set_model(GTK_DROP_DOWN(data->end_node_dropdown),
                          G_LIST_MODEL(node_list));

  // Restauration ou valeurs par défaut sécurisées
  if (current_start >= data->graph->num_vertices)
    current_start = 0;
  if (current_end >= data->graph->num_vertices)
    current_end =
        (data->graph->num_vertices > 0) ? data->graph->num_vertices - 1 : 0;

  gtk_drop_down_set_selected(GTK_DROP_DOWN(data->start_node_dropdown),
                             current_start);
  gtk_drop_down_set_selected(GTK_DROP_DOWN(data->end_node_dropdown),
                             current_end);
}

static void fill_graph_random(GraphsWindowData *data) {
  if (!data->graph)
    return;

  srand(time(NULL));

  for (int i = 0; i < data->graph->num_vertices; i++) {
    bool unique;
    do {
      unique = true;
      // Générer une valeur candidate
      switch (data->graph->data_type) {
      case DATA_TYPE_INT:
        graph_set_node_value_int(data->graph, i, rand() % 100);
        break;
      case DATA_TYPE_DOUBLE: {
        double val = (double)rand() / RAND_MAX * 100.0;
        graph_set_node_value_double(data->graph, i, val);
        break;
      }
      case DATA_TYPE_CHAR:
        graph_set_node_value_char(data->graph, i, 'A' + (rand() % 26));
        break;
      case DATA_TYPE_STRING: {
        char buf[32];
        snprintf(buf, sizeof(buf), "S%d_%d", i, rand() % 100);
        graph_set_node_value_string(data->graph, i, buf);
        break;
      }
      }

      // Vérifier l'unicité par rapport aux précédents
      for (int j = 0; j < i; j++) {
        switch (data->graph->data_type) {
        case DATA_TYPE_INT:
          if (data->graph->nodes[i].value.int_val ==
              data->graph->nodes[j].value.int_val) {
            unique = false;
          }
          break;
        case DATA_TYPE_DOUBLE:
          if (fabs(data->graph->nodes[i].value.double_val -
                   data->graph->nodes[j].value.double_val) < 0.01) {
            unique = false;
          }
          break;
        case DATA_TYPE_CHAR:
          if (data->graph->nodes[i].value.char_val ==
              data->graph->nodes[j].value.char_val) {
            unique = false;
          }
          break;
        case DATA_TYPE_STRING:
          if (strcmp(data->graph->nodes[i].value.string_val,
                     data->graph->nodes[j].value.string_val) == 0) {
            unique = false;
          }
          break;
        }
        if (!unique)
          break;
      }
    } while (!unique);
  }
}

// ============================================================================
// Callbacks pour le Remplissage (Aléatoire et Manuel)
// ============================================================================

// Structure pour passer les données à la validation manuelle
typedef struct {
  GraphsWindowData *graph_data;
  GtkWidget *dialog;
  GtkWidget **entries; // Tableau des widgets d'entrée
} ManualFillData;

static void on_manual_fill_validate(GtkWidget *widget, gpointer user_data) {
  ManualFillData *mf_data = (ManualFillData *)user_data;
  GraphsWindowData *data = mf_data->graph_data;
  bool all_valid = true;
  char error_msg[512] = "";

  for (int i = 0; i < data->graph->num_vertices; i++) {
    const char *text = gtk_editable_get_text(GTK_EDITABLE(mf_data->entries[i]));

    switch (data->graph->data_type) {
    case DATA_TYPE_INT: {
      char *endptr;
      strtol(text, &endptr, 10);
      if (*endptr != '\0' || strlen(text) == 0) {
        all_valid = false;
        snprintf(error_msg, sizeof(error_msg),
                 "Erreur Sommet %d: '%s' n'est pas un entier valide.", i, text);
      }
      break;
    }
    case DATA_TYPE_DOUBLE: {
      char *endptr;
      strtod(text, &endptr);
      if (*endptr != '\0' || strlen(text) == 0) {
        all_valid = false;
        snprintf(error_msg, sizeof(error_msg),
                 "Erreur Sommet %d: '%s' n'est pas un réel valide.", i, text);
      }
      break;
    }
    case DATA_TYPE_CHAR:
      if (strlen(text) != 1) {
        all_valid = false;
        snprintf(error_msg, sizeof(error_msg),
                 "Erreur Sommet %d: Veuillez entrer exactement 1 caractère.",
                 i);
      }
      break;
    case DATA_TYPE_STRING:
      if (strlen(text) == 0) {
        all_valid = false;
        snprintf(error_msg, sizeof(error_msg),
                 "Erreur Sommet %d: La chaîne ne peut pas être vide.", i);
      }
      break;
    }

    if (!all_valid)
      break;
  }

  if (!all_valid) {
    GtkWidget *error_dialog = gtk_message_dialog_new(
        GTK_WINDOW(mf_data->dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK, "%s", error_msg);
    g_signal_connect(error_dialog, "response", G_CALLBACK(gtk_window_destroy),
                     NULL);
    gtk_widget_show(error_dialog);
    return;
  }

  // Si tout est valide, on met à jour le graphe
  for (int i = 0; i < data->graph->num_vertices; i++) {
    const char *text = gtk_editable_get_text(GTK_EDITABLE(mf_data->entries[i]));
    switch (data->graph->data_type) {
    case DATA_TYPE_INT:
      graph_set_node_value_int(data->graph, i, atoi(text));
      break;
    case DATA_TYPE_DOUBLE:
      graph_set_node_value_double(data->graph, i, atof(text));
      break;
    case DATA_TYPE_CHAR:
      graph_set_node_value_char(data->graph, i, text[0]);
      break;
    case DATA_TYPE_STRING:
      graph_set_node_value_string(data->graph, i, text);
      break;
    }
  }

  refresh_node_dropdowns(data); // Mettre à jour les listes déroulantes

  gtk_widget_queue_draw(data->drawing_area);
  gtk_label_set_text(GTK_LABEL(data->result_label),
                     "Graphe rempli manuellement.");

  gtk_window_destroy(GTK_WINDOW(mf_data->dialog));
}

static void on_manual_fill_destroy(GtkWidget *widget, gpointer user_data) {
  ManualFillData *mf_data = (ManualFillData *)user_data;
  g_free(mf_data->entries);
  g_free(mf_data);
}

static void fill_graph_manual(GraphsWindowData *data) {
  // Fenêtre modale pour la saisie
  GtkWidget *dialog = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(dialog), "Remplissage Manuel");
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(data->window));
  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 400);

  GtkWidget *content_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_start(content_area, 20);
  gtk_widget_set_margin_end(content_area, 20);
  gtk_widget_set_margin_top(content_area, 20);
  gtk_widget_set_margin_bottom(content_area, 20);

  GtkWidget *scrolled_window = gtk_scrolled_window_new();
  gtk_window_set_child(GTK_WINDOW(dialog), scrolled_window);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window),
                                content_area);

  ManualFillData *mf_data = g_new0(ManualFillData, 1);
  mf_data->graph_data = data;
  mf_data->dialog = dialog;
  mf_data->entries = g_malloc(sizeof(GtkWidget *) * data->graph->num_vertices);

  g_signal_connect(dialog, "destroy", G_CALLBACK(on_manual_fill_destroy),
                   mf_data);

  // Création des champs de saisie pour chaque sommet
  for (int i = 0; i < data->graph->num_vertices; i++) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(content_area), row);

    char label_str[32];
    snprintf(label_str, sizeof(label_str), "Sommet %d :", i);
    GtkWidget *label = gtk_label_new(label_str);
    gtk_box_append(GTK_BOX(row), label);

    GtkWidget *entry = gtk_entry_new();

    // Valeur actuelle comme défaut
    char val_str[64];
    graph_get_node_label(data->graph, i, val_str, sizeof(val_str));
    gtk_editable_set_text(GTK_EDITABLE(entry), val_str);

    gtk_box_append(GTK_BOX(row), entry);
    mf_data->entries[i] = entry;
  }

  // Bouton Valider
  GtkWidget *validate_btn = gtk_button_new_with_label("Valider");
  gtk_widget_set_margin_top(validate_btn, 20);
  gtk_widget_add_css_class(validate_btn, "suggested-action");
  g_signal_connect(validate_btn, "clicked", G_CALLBACK(on_manual_fill_validate),
                   mf_data);
  gtk_box_append(GTK_BOX(content_area), validate_btn);

  gtk_widget_show(dialog);
}

// ============================================================================
// Fonction principale
// ============================================================================

void open_graphs_window(GtkWindow *parent) {
  GraphsWindowData *data = g_new0(GraphsWindowData, 1);
  data->graph = NULL;
  data->graph_drawn = false;
  data->is_dragging = false;
  data->drag_start_node = -1;
  data->drag_end_node = -1;

  // Créer la fenêtre (fenêtre indépendante avec tous les contrôles)
  data->window = gtk_window_new();
  // Ne PAS utiliser gtk_window_set_transient_for pour avoir tous les boutons
  gtk_window_set_title(GTK_WINDOW(data->window), "Visualisation de Graphes");
  gtk_window_set_default_size(GTK_WINDOW(data->window), 1000, 700);

  // Container principal horizontal
  GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_window_set_child(GTK_WINDOW(data->window), main_box);

  // ===== PANNEAU DE GAUCHE (Configuration) =====
  GtkWidget *left_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
  gtk_widget_set_size_request(left_panel, 280, -1);
  gtk_widget_set_margin_start(left_panel, 15);
  gtk_widget_set_margin_end(left_panel, 15);
  gtk_widget_set_margin_top(left_panel, 15);
  gtk_widget_set_margin_bottom(left_panel, 15);

  // Wrap the left panel in a scrolled window to ensure all content (especially
  // results) is visible
  GtkWidget *left_scrolled = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(left_scrolled),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  // Set a minimum width for the scrolled window
  gtk_widget_set_size_request(left_scrolled, 300, -1);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(left_scrolled), left_panel);

  gtk_box_append(GTK_BOX(main_box), left_scrolled);

  // Titre
  GtkWidget *title = gtk_label_new("Configuration");
  gtk_widget_add_css_class(title, "header-label");
  gtk_box_append(GTK_BOX(left_panel), title);

  // Type de données
  GtkWidget *data_type_label = gtk_label_new("Type de données:");
  gtk_widget_set_halign(data_type_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(left_panel), data_type_label);

  GtkStringList *data_types = gtk_string_list_new(NULL);
  gtk_string_list_append(data_types, "Entier");
  gtk_string_list_append(data_types, "Réel");
  gtk_string_list_append(data_types, "Caractère");
  gtk_string_list_append(data_types, "Chaîne");
  data->data_type_dropdown = gtk_drop_down_new(G_LIST_MODEL(data_types), NULL);
  gtk_drop_down_set_selected(GTK_DROP_DOWN(data->data_type_dropdown),
                             DATA_TYPE_CHAR);
  gtk_box_append(GTK_BOX(left_panel), data->data_type_dropdown);

  // Type de chemin
  GtkWidget *path_type_label = gtk_label_new("Type de chemin:");
  gtk_widget_set_halign(path_type_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(left_panel), path_type_label);

  GtkStringList *path_types = gtk_string_list_new(NULL);
  gtk_string_list_append(path_types, "Orienté");
  gtk_string_list_append(path_types, "Non orienté");
  data->path_type_dropdown = gtk_drop_down_new(G_LIST_MODEL(path_types), NULL);
  gtk_drop_down_set_selected(GTK_DROP_DOWN(data->path_type_dropdown),
                             0); // Par défaut orienté
  g_signal_connect(data->path_type_dropdown, "notify::selected",
                   G_CALLBACK(on_setting_changed), data);
  gtk_box_append(GTK_BOX(left_panel), data->path_type_dropdown);

  // Nombre de sommets
  GtkWidget *num_vertices_label = gtk_label_new("Nombre de sommets:");
  gtk_widget_set_halign(num_vertices_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(left_panel), num_vertices_label);

  data->num_vertices_spin = gtk_spin_button_new_with_range(2, 20, 1);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->num_vertices_spin), 5);
  gtk_box_append(GTK_BOX(left_panel), data->num_vertices_spin);

  // Bouton dessiner graphe au-dessus : Dropdown "Mode de remplissage"
  GtkWidget *fill_mode_label = gtk_label_new("Mode de Remplissage:");
  gtk_widget_set_halign(fill_mode_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(left_panel), fill_mode_label);

  GtkStringList *fill_modes = gtk_string_list_new(NULL);
  gtk_string_list_append(fill_modes, "Vide (Défaut)");
  gtk_string_list_append(fill_modes, "Aléatoire");
  gtk_string_list_append(fill_modes, "Manuel");
  data->filling_mode_dropdown =
      gtk_drop_down_new(G_LIST_MODEL(fill_modes), NULL);
  gtk_drop_down_set_selected(GTK_DROP_DOWN(data->filling_mode_dropdown), 0);
  gtk_box_append(GTK_BOX(left_panel), data->filling_mode_dropdown);

  // Bouton dessiner graphe
  data->draw_graph_btn = gtk_button_new_with_label("🎨 Dessiner Graphe");
  gtk_widget_add_css_class(data->draw_graph_btn, "success-btn");
  g_signal_connect(data->draw_graph_btn, "clicked",
                   G_CALLBACK(on_draw_graph_clicked), data);
  gtk_box_append(GTK_BOX(left_panel), data->draw_graph_btn);

  // Bouton vider graphe
  GtkWidget *clear_btn = gtk_button_new_with_label("🗑️ Vider Graphe");
  gtk_widget_add_css_class(clear_btn, "destructive-btn");
  g_signal_connect(clear_btn, "clicked", G_CALLBACK(on_clear_graph_clicked),
                   data);
  gtk_box_append(GTK_BOX(left_panel), clear_btn);

  // Sommet de départ
  GtkWidget *start_label = gtk_label_new("Sommet de départ:");
  gtk_widget_set_halign(start_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(left_panel), start_label);

  GtkStringList *empty_list1 = gtk_string_list_new(NULL);
  data->start_node_dropdown =
      gtk_drop_down_new(G_LIST_MODEL(empty_list1), NULL);
  gtk_box_append(GTK_BOX(left_panel), data->start_node_dropdown);

  // Sommet de fin
  GtkWidget *end_label = gtk_label_new("Sommet d'arrivée:");
  gtk_widget_set_halign(end_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(left_panel), end_label);

  GtkStringList *empty_list2 = gtk_string_list_new(NULL);
  data->end_node_dropdown = gtk_drop_down_new(G_LIST_MODEL(empty_list2), NULL);
  gtk_box_append(GTK_BOX(left_panel), data->end_node_dropdown);

  // Algorithme
  GtkWidget *algo_label = gtk_label_new("Algorithme:");
  gtk_widget_set_halign(algo_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(left_panel), algo_label);

  GtkStringList *algorithms = gtk_string_list_new(NULL);
  gtk_string_list_append(algorithms, "Dijkstra");
  gtk_string_list_append(algorithms, "Bellman-Ford");
  gtk_string_list_append(algorithms, "Floyd-Warshall");
  data->algorithm_dropdown = gtk_drop_down_new(G_LIST_MODEL(algorithms), NULL);
  gtk_box_append(GTK_BOX(left_panel), data->algorithm_dropdown);

  // Bouton exécuter
  data->execute_btn = gtk_button_new_with_label("🚀 Exécuter Algorithme");
  gtk_widget_add_css_class(data->execute_btn, "accent-btn");
  g_signal_connect(data->execute_btn, "clicked",
                   G_CALLBACK(on_execute_algorithm_clicked), data);
  gtk_box_append(GTK_BOX(left_panel), data->execute_btn);

  // Séparateur
  GtkWidget *separator3 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_append(GTK_BOX(left_panel), separator3);

  // Bouton Retour
  GtkWidget *btn_back = gtk_button_new_with_label("🔙 Retour Menu");
  g_signal_connect_swapped(btn_back, "clicked", G_CALLBACK(gtk_window_close),
                           data->window);
  gtk_box_append(GTK_BOX(left_panel), btn_back);

  // Séparateur
  GtkWidget *separator2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_append(GTK_BOX(left_panel), separator2);

  // Résultats
  GtkWidget *result_title = gtk_label_new("Résultat:");
  gtk_widget_add_css_class(result_title, "header-label");
  gtk_widget_set_halign(result_title, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(left_panel), result_title);

  data->result_label = gtk_label_new("En attente...");
  gtk_label_set_wrap(GTK_LABEL(data->result_label), TRUE);
  gtk_label_set_xalign(GTK_LABEL(data->result_label), 0);
  gtk_widget_set_vexpand(data->result_label, TRUE);
  gtk_widget_set_valign(data->result_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(left_panel), data->result_label);

  // ===== ZONE DE DESSIN (Droite) =====
  GtkWidget *right_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_hexpand(right_panel, TRUE);
  gtk_widget_set_vexpand(right_panel, TRUE);
  gtk_box_append(GTK_BOX(main_box), right_panel);

  data->drawing_area = gtk_drawing_area_new();
  gtk_widget_set_hexpand(data->drawing_area, TRUE);
  gtk_widget_set_vexpand(data->drawing_area, TRUE);
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(data->drawing_area),
                                 draw_graph, data, NULL);
  gtk_box_append(GTK_BOX(right_panel), data->drawing_area);

  // Gesture pour le drag (glisser-déposer pour créer les arêtes)
  GtkGesture *drag_gesture = gtk_gesture_drag_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(drag_gesture),
                                GDK_BUTTON_PRIMARY);
  g_signal_connect(drag_gesture, "drag-begin", G_CALLBACK(on_drag_begin), data);
  g_signal_connect(drag_gesture, "drag-update", G_CALLBACK(on_drag_update),
                   data);
  g_signal_connect(drag_gesture, "drag-end", G_CALLBACK(on_drag_end), data);
  gtk_widget_add_controller(data->drawing_area,
                            GTK_EVENT_CONTROLLER(drag_gesture));

  // Signal de destruction
  g_signal_connect(data->window, "destroy", G_CALLBACK(on_window_destroy),
                   data);

  // Afficher la fenêtre
  gtk_window_present(GTK_WINDOW(data->window));
}
