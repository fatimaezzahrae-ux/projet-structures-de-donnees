#include "trees_window.h"
#include "tree_algorithms.h"
#include <ctype.h>
#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define HORIZONTAL_SPACING 70.0
#define VERTICAL_SPACING 80.0
#define PADDING 50.0

typedef enum { MODE_NARY, MODE_BINARY } TreeMode;
typedef enum { INPUT_MANUAL, INPUT_RANDOM } InputMode;

typedef struct {
  GtkWidget *window;
  GtkWidget *drawing_area;
  GtkWidget *log_view;
  GtkTextBuffer *log_buffer;

  // Controls
  GtkWidget *combo_tree_type;
  GtkWidget *entry_value;
  GtkWidget *entry_insert_parent; // Dedicated Parent field
  GtkWidget *lbl_insert_parent;
  GtkWidget *entry_parent; // Dedicated New Value field (Modify)
  GtkWidget *lbl_parent_hint;
  GtkWidget *btn_transform;

  GtkWidget *combo_data_type; // Visual only
  GtkWidget *radio_manual;
  GtkWidget *radio_random;

  GtkWidget *frame_manual;
  GtkWidget *frame_random;

  GtkWidget *entry_rand_size;
  GtkWidget *box_nary_degree;
  GtkWidget *entry_nary_degree;
  GtkWidget *btn_generate;
  GtkWidget *btn_clear;

  GtkWidget *combo_trav_type;
  GtkWidget *combo_trav_method;
  GtkWidget *lbl_trav_result;

  GtkWidget *lbl_stats;

  // Data
  NaryNode *nary_root;
  BinaryNode *binary_root;
  TreeMode current_mode;
  InputMode input_mode;
} TreesWindowData;

// ============================================================================
// Logging
// ============================================================================

static void log_message(TreesWindowData *data, const char *fmt, ...) {
  char buffer[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  GtkTextIter iter;
  gtk_text_buffer_get_end_iter(data->log_buffer, &iter);
  gtk_text_buffer_insert(data->log_buffer, &iter, buffer, -1);
  gtk_text_buffer_insert(data->log_buffer, &iter, "\n", -1);
}

// ============================================================================
// Layout Calculation
// ============================================================================

static void layout_nary_robust(NaryNode *root, double *current_x, double y) {
  if (!root)
    return;

  // If leaf
  if (root->num_children == 0) {
    root->x = *current_x;
    root->y = y;
    *current_x += HORIZONTAL_SPACING;
  } else {
    // Layout all children first
    for (int i = 0; i < root->num_children; i++) {
      layout_nary_robust(root->children[i], current_x, y + VERTICAL_SPACING);
    }
    // Parent is centered over first and last child
    if (root->num_children > 0) {
      double first = root->children[0]->x;
      double last = root->children[root->num_children - 1]->x;
      root->x = (first + last) / 2.0;
      root->y = y;
    } else {
      // Should catch above, but safety
      root->x = *current_x;
      root->y = y;
      *current_x += HORIZONTAL_SPACING;
    }
  }
}

static void layout_binary_inorder(BinaryNode *root, double *current_x,
                                  double y) {
  if (!root)
    return;

  layout_binary_inorder(root->left, current_x, y + VERTICAL_SPACING);

  root->x = *current_x;
  root->y = y;
  *current_x += HORIZONTAL_SPACING;

  layout_binary_inorder(root->right, current_x, y + VERTICAL_SPACING);
}

// ============================================================================
// Drawing
// ============================================================================

static void draw_node(cairo_t *cr, double x, double y, int value,
                      bool is_binary) {
  if (is_binary)
    cairo_set_source_rgb(cr, 0.2, 0.6, 0.8);
  else
    cairo_set_source_rgb(cr, 0.2, 0.8, 0.4);
  cairo_arc(cr, x, y, 20, 0, 2 * M_PI);
  cairo_fill(cr);
  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
  cairo_set_line_width(cr, 2.0);
  cairo_arc(cr, x, y, 20, 0, 2 * M_PI);
  cairo_stroke(cr);

  char text[32];
  snprintf(text, sizeof(text), "%d", value);
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 14);
  cairo_text_extents_t extents;
  cairo_text_extents(cr, text, &extents);
  cairo_move_to(cr, x - extents.width / 2, y + extents.height / 2);
  cairo_show_text(cr, text);
}

static void draw_nary_links_and_nodes(cairo_t *cr, NaryNode *node) {
  if (!node)
    return;
  for (int i = 0; i < node->num_children; i++) {
    NaryNode *child = node->children[i];
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr, node->x, node->y);
    cairo_line_to(cr, child->x, child->y);
    cairo_stroke(cr);
    draw_nary_links_and_nodes(cr, child);
  }
  draw_node(cr, node->x, node->y, node->value, false);
}

static void draw_binary_links_and_nodes(cairo_t *cr, BinaryNode *node) {
  if (!node)
    return;
  if (node->left) {
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr, node->x, node->y);
    cairo_line_to(cr, node->left->x, node->left->y);
    cairo_stroke(cr);
    draw_binary_links_and_nodes(cr, node->left);
  }
  if (node->right) {
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr, node->x, node->y);
    cairo_line_to(cr, node->right->x, node->right->y);
    cairo_stroke(cr);
    draw_binary_links_and_nodes(cr, node->right);
  }
  draw_node(cr, node->x, node->y, node->value, true);
}

static void draw_tree(GtkDrawingArea *area, cairo_t *cr, int width, int height,
                      gpointer user_data) {
  TreesWindowData *data = (TreesWindowData *)user_data;
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_paint(cr);

  if (data->current_mode == MODE_NARY && data->nary_root) {
    double current_x = PADDING;
    layout_nary_robust(data->nary_root, &current_x, PADDING);
    draw_nary_links_and_nodes(cr, data->nary_root);
  } else if (data->current_mode == MODE_BINARY && data->binary_root) {
    double current_x = PADDING;
    layout_binary_inorder(data->binary_root, &current_x, PADDING);
    draw_binary_links_and_nodes(cr, data->binary_root);
  } else {
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_font_size(cr, 20);
    cairo_text_extents_t extents;
    char *msg = "Arbre Vide";
    cairo_text_extents(cr, msg, &extents);
    cairo_move_to(cr, (width - extents.width) / 2, height / 2);
    cairo_show_text(cr, msg);
  }
}

// ============================================================================
// Logic Callbacks
// ============================================================================

// ============================================================================
// Layout Helpers
// ============================================================================

static int count_leaves_nary(NaryNode *root) {
  if (!root)
    return 0;
  if (root->num_children == 0)
    return 1;
  int count = 0;
  for (int i = 0; i < root->num_children; i++) {
    count += count_leaves_nary(root->children[i]);
  }
  return count;
}

static int count_leaves_binary(BinaryNode *root) {
  if (!root)
    return 0;
  if (!root->left && !root->right)
    return 1;
  return count_leaves_binary(root->left) + count_leaves_binary(root->right);
}

static void update_drawing_area_layout(TreesWindowData *data) {
  int width_units = 0;
  int height = 0;

  if (data->current_mode == MODE_NARY) {
    width_units = count_leaves_nary(data->nary_root);
    height = nary_get_height(data->nary_root);
  } else {
    // Binary layout is Inorder (one slot per node), so we need total size, not
    // just leaves
    width_units = bst_get_size(data->binary_root);
    height = bst_get_height(data->binary_root);
  }

  // Minimums
  if (width_units < 1)
    width_units = 1;
  if (height < 1)
    height = 1;

  // Heuristic: based on constant spacing
  int req_width = width_units * HORIZONTAL_SPACING + PADDING * 2;
  int req_height = height * VERTICAL_SPACING + PADDING * 2;

  if (req_width < 800)
    req_width = 800;
  if (req_height < 600)
    req_height = 600;

  gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(data->drawing_area),
                                     req_width);
  gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(data->drawing_area),
                                      req_height);
}

static void update_stats(TreesWindowData *data) {
  int size = 0, height = 0;
  if (data->current_mode == MODE_NARY) {
    size = nary_get_size(data->nary_root);
    height = nary_get_height(data->nary_root);
  } else {
    size = bst_get_size(data->binary_root);
    height = bst_get_height(data->binary_root);
  }
  char buf[64];
  snprintf(buf, sizeof(buf), "Taille: %d | Hauteur: %d", size, height);
  gtk_label_set_text(GTK_LABEL(data->lbl_stats), buf);
  update_drawing_area_layout(data);
}

static void on_insert_clicked(GtkWidget *btn, gpointer user_data) {
  TreesWindowData *data = (TreesWindowData *)user_data;
  const char *val_str = gtk_editable_get_text(GTK_EDITABLE(data->entry_value));
  int val = atoi(val_str);

  if (data->current_mode == MODE_BINARY) {
    data->binary_root = bst_insert(data->binary_root, val);
    log_message(data, "Inséré (BST): %d", val);
  } else {
    const char *parent_str =
        gtk_editable_get_text(GTK_EDITABLE(data->entry_insert_parent));
    if (strlen(parent_str) == 0 && data->nary_root == NULL) {
      data->nary_root = nary_insert(data->nary_root, val, 0);
      log_message(data, "Inséré Racine (N-ary): %d", val);
    } else {
      int parent_val = atoi(parent_str);
      data->nary_root = nary_insert(data->nary_root, val, parent_val);
      log_message(data, "Tentative insertion %d sous %d", val, parent_val);
    }
  }
  update_stats(data);
  gtk_widget_queue_draw(data->drawing_area);
}

static void on_delete_clicked(GtkWidget *btn, gpointer user_data) {
  TreesWindowData *data = (TreesWindowData *)user_data;
  const char *val_str = gtk_editable_get_text(GTK_EDITABLE(data->entry_value));
  int val = atoi(val_str);

  if (data->current_mode == MODE_BINARY) {
    data->binary_root = bst_delete(data->binary_root, val);
    log_message(data, "Supprimé (BST): %d", val);
  } else {
    data->nary_root = nary_delete(data->nary_root, val);
    log_message(data, "Supprimé (N-ary): %d", val);
  }
  update_stats(data);
  gtk_widget_queue_draw(data->drawing_area);
}

static void on_modify_clicked(GtkWidget *btn, gpointer user_data) {
  TreesWindowData *data = (TreesWindowData *)user_data;
  const char *old_str = gtk_editable_get_text(GTK_EDITABLE(data->entry_value));
  const char *new_str = gtk_editable_get_text(GTK_EDITABLE(data->entry_parent));

  int old_val = atoi(old_str);
  int new_val = atoi(new_str);

  if (data->current_mode == MODE_BINARY) {
    bst_modify(&data->binary_root, old_val, new_val);
    log_message(data, "Modifié (BST): %d -> %d", old_val, new_val);
  } else {
    nary_modify(data->nary_root, old_val, new_val);
    log_message(data, "Modifié (N-ary): %d -> %d", old_val, new_val);
  }
  update_stats(data);
  gtk_widget_queue_draw(data->drawing_area);
}

static void on_generate_clicked(GtkWidget *btn, gpointer user_data) {
  TreesWindowData *data = (TreesWindowData *)user_data;
  const char *size_str =
      gtk_editable_get_text(GTK_EDITABLE(data->entry_rand_size));
  int count = atoi(size_str);
  if (count <= 0)
    count = 10;

  srand(time(NULL));

  if (data->binary_root) {
    free_binary_tree(data->binary_root);
    data->binary_root = NULL;
  }
  if (data->nary_root) {
    free_nary_tree(data->nary_root);
    data->nary_root = NULL;
  }

  int *values = malloc(count * sizeof(int));
  for (int i = 0; i < count; i++)
    values[i] = rand() % 100 + 1;

  if (data->current_mode == MODE_BINARY) {
    for (int i = 0; i < count; i++)
      data->binary_root = bst_insert(data->binary_root, values[i]);
  } else {
    if (count > 0) {
      data->nary_root = nary_insert(NULL, values[0], 0);
      for (int i = 1; i < count; i++) {
        int parent_idx = rand() % i;
        nary_insert(data->nary_root, values[i], values[parent_idx]);
      }
    }
  }
  free(values);
  log_message(data, "Généré aléatoirement %d noeuds", count);
  update_stats(data);
  gtk_widget_queue_draw(data->drawing_area);
}

static void on_clear_clicked(GtkWidget *btn, gpointer user_data) {
  TreesWindowData *data = (TreesWindowData *)user_data;
  if (data->binary_root) {
    free_binary_tree(data->binary_root);
    data->binary_root = NULL;
  }
  if (data->nary_root) {
    free_nary_tree(data->nary_root);
    data->nary_root = NULL;
  }
  log_message(data, "Arbre vidé");
  update_stats(data);
  gtk_widget_queue_draw(data->drawing_area);
}

static void on_convert_clicked(GtkWidget *btn, gpointer user_data) {
  TreesWindowData *data = (TreesWindowData *)user_data;
  if (data->current_mode != MODE_NARY || !data->nary_root)
    return;

  if (data->binary_root)
    free_binary_tree(data->binary_root);
  data->binary_root = convert_nary_to_binary(data->nary_root);

  data->current_mode = MODE_BINARY;
  gtk_drop_down_set_selected(GTK_DROP_DOWN(data->combo_tree_type), 0);

  log_message(data, "Converti N-aire -> Binaire");
  update_stats(data);
  gtk_widget_queue_draw(data->drawing_area);
}

static void on_execute_traversal_clicked(GtkWidget *btn, gpointer user_data) {
  TreesWindowData *data = (TreesWindowData *)user_data;
  int type_idx =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(data->combo_trav_type));
  int method_idx =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(data->combo_trav_method));

  TraversalResult res = {0};
  char *method_name = "";

  if (data->current_mode == MODE_BINARY) {
    if (type_idx == 1) { // BFS
      res = bst_traverse_bfs(data->binary_root);
      method_name = "Largeur (BFS)";
    } else {
      if (method_idx == 0) {
        res = bst_traverse_preorder(data->binary_root);
        method_name = "Pré-ordre";
      } else if (method_idx == 1) {
        res = bst_traverse_inorder(data->binary_root);
        method_name = "In-ordre";
      } else {
        res = bst_traverse_postorder(data->binary_root);
        method_name = "Post-ordre";
      }
    }
  } else {
    if (type_idx == 1) { // BFS
      res = nary_traverse_bfs(data->nary_root);
      method_name = "Largeur (BFS)";
    } else {
      if (method_idx == 1)
        log_message(data, "Attention: In-ordre ignore pour N-aire");
      if (method_idx == 2) {
        res = nary_traverse_postorder(data->nary_root);
        method_name = "Post-ordre";
      } else {
        res = nary_traverse_preorder(data->nary_root);
        method_name = "Pré-ordre";
      }
    }
  }

  GString *str = g_string_new("");
  g_string_append_printf(str, "%s: [", method_name);
  for (int i = 0; i < res.count; i++) {
    g_string_append_printf(str, "%d%s", res.values[i],
                           (i < res.count - 1) ? ", " : "");
  }
  g_string_append(str, "]");

  gtk_label_set_text(GTK_LABEL(data->lbl_trav_result), str->str);
  log_message(data, "%s", str->str);

  g_string_free(str, TRUE);
  free_traversal_result(&res);
}

// ============================================================================
// Manual Input Dialog
// ============================================================================

typedef struct {
  TreesWindowData *data;
  GtkWidget *entry;
  GtkWidget *window;
} ManualDialogData;

static int validate_token(const char *token, int type_idx, char *error_msg,
                          size_t max_len) {
  char *endptr;
  // 0: Int, 1: Float, 2: Char, 3: String
  if (type_idx == 0) { // Int
    long val = strtol(token, &endptr, 10);
    if (*endptr != '\0') {
      snprintf(error_msg, max_len,
               "Erreur: '%s' n'est pas un entier valide (caractères ou "
               "décimaux détectés).",
               token);
      return 0;
    }
  } else if (type_idx == 1) { // Float
    double val = strtod(token, &endptr);
    if (*endptr != '\0') {
      snprintf(error_msg, max_len,
               "Erreur: '%s' n'est pas un nombre réel valide.", token);
      return 0;
    }
  } else if (type_idx == 2) { // Char
    if (strlen(token) != 1) {
      snprintf(error_msg, max_len,
               "Erreur: '%s' doit être un caractère unique.", token);
      return 0;
    }
    if (isdigit((unsigned char)token[0])) {
      snprintf(error_msg, max_len,
               "Erreur: '%s' est un chiffre, pas un caractère.", token);
      return 0;
    }
  } else if (type_idx == 3) { // String
    // Fail if it looks like a pure number
    double val = strtod(token, &endptr);
    if (*endptr == '\0') {
      snprintf(error_msg, max_len,
               "Erreur: '%s' est un nombre, pas une chaîne de caractères.",
               token);
      return 0;
    }
  }
  return 1;
}

static void on_manual_dialog_ok(GtkWidget *btn, gpointer user_data) {
  ManualDialogData *dlg_data = (ManualDialogData *)user_data;
  const char *text = gtk_editable_get_text(GTK_EDITABLE(dlg_data->entry));
  TreesWindowData *data = dlg_data->data;

  // Retrieve selected type
  int type_idx =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(data->combo_data_type));

  int n_degree = 3;
  if (data->current_mode == MODE_NARY && data->entry_nary_degree) {
    const char *n_str =
        gtk_editable_get_text(GTK_EDITABLE(data->entry_nary_degree));
    n_degree = atoi(n_str);
    if (n_degree < 1)
      n_degree = 1;
  }

  char *text_copy = strdup(text);
  // Separate ONLY by whitespace (spaces, tabs, newlines), NO COMMAS
  const char *delimiters = " \t\n";
  char *token = strtok(text_copy, delimiters);

  // First pass: Validation
  char error_msg[256];
  char *copy_for_validation = strdup(text);
  char *val_token = strtok(copy_for_validation, delimiters);
  int valid = 1;

  while (val_token != NULL) {
    if (!validate_token(val_token, type_idx, error_msg, sizeof(error_msg))) {
      valid = 0;
      break;
    }
    val_token = strtok(NULL, delimiters);
  }
  free(copy_for_validation);

  if (!valid) {
    GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(dlg_data->window),
                                            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                            GTK_BUTTONS_OK, "%s", error_msg);
    gtk_window_present(GTK_WINDOW(msg));
    g_signal_connect_swapped(msg, "response", G_CALLBACK(gtk_window_destroy),
                             msg);
    free(text_copy);
    return; // Do not close dialog, let user fix
  }

  // Second pass: Processing
  if (data->binary_root) {
    free_binary_tree(data->binary_root);
    data->binary_root = NULL;
  }
  if (data->nary_root) {
    free_nary_tree(data->nary_root);
    data->nary_root = NULL;
  }

  int capacity = 128;
  int *values = (int *)malloc(capacity * sizeof(int));
  int count = 0;

  // Re-tokenize since first pass consumed text_copy (partially or fully if we
  // didn't re-copy) Actually strtok modifies the string, so we need to restart
  // or use the original text again. Better: just re-copy from original 'text'
  free(text_copy);
  text_copy = strdup(text);
  token = strtok(text_copy, delimiters);

  while (token != NULL) {
    if (count >= capacity) {
      capacity *= 2;
      values = (int *)realloc(values, capacity * sizeof(int));
    }

    int val = 0;
    if (type_idx == 0 || type_idx == 1) { // Int or Float -> Store as Int
      double d = atof(token);
      val = (int)d;
    } else if (type_idx == 2) { // Char
      val = (int)token[0];
    } else { // String
      // Only placeholder supported for now
      val = 0;
    }

    values[count++] = val;
    token = strtok(NULL, delimiters);
  }

  if (count == 0) {
    // Empty input or only separators
    free(values);
    free(text_copy);
    gtk_window_destroy(GTK_WINDOW(dlg_data->window));
    g_free(dlg_data);
    return;
  }

  for (int i = 0; i < count; i++) {
    int val = values[i];
    if (data->current_mode == MODE_BINARY) {
      data->binary_root = bst_insert(data->binary_root, val);
    } else {
      if (i == 0) {
        data->nary_root = nary_insert(NULL, val, 0);
      } else {
        int parent_idx = (i - 1) / n_degree;
        if (parent_idx >= 0 && parent_idx < i) {
          nary_insert(data->nary_root, val, values[parent_idx]);
        }
      }
    }
  }

  free(values);
  free(text_copy);
  log_message(data, "Saisie manuelle: %d valeurs importées", count);
  update_stats(data);
  gtk_widget_queue_draw(data->drawing_area);
  gtk_window_destroy(GTK_WINDOW(dlg_data->window));
  g_free(dlg_data);
}

static void on_manual_dialog_cancel(GtkWidget *btn, gpointer user_data) {
  ManualDialogData *dlg_data = (ManualDialogData *)user_data;
  gtk_window_destroy(GTK_WINDOW(dlg_data->window));
  g_free(dlg_data);
}

static void on_manual_input_clicked(GtkWidget *btn, gpointer user_data) {
  TreesWindowData *data = (TreesWindowData *)user_data;
  GtkWidget *dialog = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(dialog), "Saisie Manuelle");
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(data->window));
  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 150);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_top(box, 20);
  gtk_widget_set_margin_bottom(box, 20);
  gtk_widget_set_margin_start(box, 20);
  gtk_widget_set_margin_end(box, 20);
  gtk_window_set_child(GTK_WINDOW(dialog), box);

  gtk_box_append(
      GTK_BOX(box),
      gtk_label_new("Entrez les valeurs (séparées par des ESPACES):"));
  GtkWidget *entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Ex: 10 5 15 3");
  gtk_box_append(GTK_BOX(box), entry);

  GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(btn_box, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(box), btn_box);

  GtkWidget *btn_ok = gtk_button_new_with_label("OK");
  GtkWidget *btn_cancel = gtk_button_new_with_label("Annuler");
  gtk_box_append(GTK_BOX(btn_box), btn_ok);
  gtk_box_append(GTK_BOX(btn_box), btn_cancel);

  ManualDialogData *dlg_data = g_new(ManualDialogData, 1);
  dlg_data->data = data;
  dlg_data->entry = entry;
  dlg_data->window = dialog;

  g_signal_connect(btn_ok, "clicked", G_CALLBACK(on_manual_dialog_ok),
                   dlg_data);
  g_signal_connect(btn_cancel, "clicked", G_CALLBACK(on_manual_dialog_cancel),
                   dlg_data);
  gtk_window_present(GTK_WINDOW(dialog));
}

// ============================================================================
// UI Callbacks
// ============================================================================

static void on_trav_type_changed(GObject *obj, GParamSpec *pspec,
                                 gpointer user_data) {
  TreesWindowData *data = (TreesWindowData *)user_data;
  int idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(data->combo_trav_type));

  // If BFS (index 1), disable method selection (Pre/In/Post not applicable)
  if (idx == 1) {
    gtk_widget_set_sensitive(data->combo_trav_method, FALSE);
  } else {
    gtk_widget_set_sensitive(data->combo_trav_method, TRUE);
  }
}

static void on_tree_type_changed(GObject *obj, GParamSpec *pspec,
                                 gpointer user_data) {
  TreesWindowData *data = (TreesWindowData *)user_data;
  int idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(data->combo_tree_type));
  data->current_mode = (idx == 0) ? MODE_BINARY : MODE_NARY;

  if (data->current_mode == MODE_BINARY) {
    gtk_widget_set_visible(data->btn_transform, FALSE);

    if (data->lbl_insert_parent)
      gtk_widget_set_visible(data->lbl_insert_parent, FALSE);
    if (data->entry_insert_parent)
      gtk_widget_set_visible(data->entry_insert_parent, FALSE);

    gtk_label_set_text(GTK_LABEL(data->lbl_parent_hint),
                       "Nouvelle Valeur (Modifier):");
    gtk_widget_set_visible(data->lbl_parent_hint, TRUE);

    if (data->box_nary_degree)
      gtk_widget_set_visible(data->box_nary_degree, FALSE);
  } else {
    gtk_widget_set_visible(data->btn_transform, TRUE);

    if (data->lbl_insert_parent)
      gtk_widget_set_visible(data->lbl_insert_parent, TRUE);
    if (data->entry_insert_parent)
      gtk_widget_set_visible(data->entry_insert_parent, TRUE);

    gtk_label_set_text(GTK_LABEL(data->lbl_parent_hint),
                       "Nouvelle Valeur (Modifier):");
    gtk_widget_set_visible(data->lbl_parent_hint, TRUE);

    if (data->box_nary_degree)
      gtk_widget_set_visible(data->box_nary_degree, TRUE);
  }
  update_stats(data);
  gtk_widget_queue_draw(data->drawing_area);
}

static void on_input_mode_changed(GtkCheckButton *btn, gpointer user_data) {
  if (!gtk_check_button_get_active(btn))
    return;

  TreesWindowData *data = (TreesWindowData *)user_data;
  if (gtk_check_button_get_active(GTK_CHECK_BUTTON(data->radio_manual))) {
    data->input_mode = INPUT_MANUAL;
    gtk_widget_set_visible(data->frame_manual, TRUE);
    gtk_widget_set_visible(data->frame_random, FALSE);
  } else {
    data->input_mode = INPUT_RANDOM;
    gtk_widget_set_visible(data->frame_manual, FALSE);
    gtk_widget_set_visible(data->frame_random, TRUE);
  }
}

static void on_window_destroy(GtkWidget *widget, gpointer user_data) {
  TreesWindowData *data = (TreesWindowData *)user_data;
  if (data->binary_root)
    free_binary_tree(data->binary_root);
  if (data->nary_root)
    free_nary_tree(data->nary_root);
  g_free(data);
}

static void apply_custom_css(void) {
  GtkCssProvider *provider = gtk_css_provider_new();
  const char *css = ".btn-insert { background-color: #2ecc71; color: white; "
                    "font-weight: bold; }"
                    ".btn-delete { background-color: #e74c3c; color: white; "
                    "font-weight: bold; }"
                    ".btn-modify { background-color: #f39c12; color: white; "
                    "font-weight: bold; }"
                    ".btn-generate { background-color: #9b59b6; color: white; "
                    "font-weight: bold; }"
                    ".btn-execute { background-color: #3498db; color: white; "
                    "font-weight: bold; }"
                    ".btn-clear { color: #c0392b; }"
                    "window { background-color: #f0f2f5; }"
                    "frame { background-color: white; border-radius: 8px; }";

  gtk_css_provider_load_from_data(provider, css, -1);
  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
}

// ============================================================================
// Main Window Construction
// ============================================================================

void open_trees_window(GtkWindow *parent) {
  apply_custom_css();

  TreesWindowData *data = g_new0(TreesWindowData, 1);

  data->window = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(data->window),
                       "Gestion de Structures d'Arbres 🌳");
  gtk_window_set_default_size(GTK_WINDOW(data->window), 1200, 800);

  GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_window_set_child(GTK_WINDOW(data->window), main_box);

  // --- Left Panel ---
  GtkWidget *left_scroll = gtk_scrolled_window_new();
  gtk_widget_set_size_request(left_scroll, 380,
                              -1); // Slightly wider for emojis
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(left_scroll),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_box_append(GTK_BOX(main_box), left_scroll);

  GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
  gtk_widget_set_margin_top(left_box, 15);
  gtk_widget_set_margin_bottom(left_box, 15);
  gtk_widget_set_margin_start(left_box, 15);
  gtk_widget_set_margin_end(left_box, 15);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(left_scroll), left_box);

  // Group 1: Configuration
  GtkWidget *frame_config = gtk_frame_new("⚙️ Configuration & Opérations");
  GtkWidget *box_config = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_set_margin_top(box_config, 10);
  gtk_widget_set_margin_bottom(box_config, 10);
  gtk_widget_set_margin_start(box_config, 10);
  gtk_widget_set_margin_end(box_config, 10);
  gtk_frame_set_child(GTK_FRAME(frame_config), box_config);
  gtk_box_append(GTK_BOX(left_box), frame_config);

  gtk_box_append(GTK_BOX(box_config), gtk_label_new("Type d'arbre:"));
  const char *tree_types[] = {"🌳 BST (Binaire)", "🌿 N-ary (Générique)", NULL};
  data->combo_tree_type = gtk_drop_down_new_from_strings(tree_types);
  g_signal_connect(data->combo_tree_type, "notify::selected",
                   G_CALLBACK(on_tree_type_changed), data);
  gtk_box_append(GTK_BOX(box_config), data->combo_tree_type);

  gtk_box_append(GTK_BOX(box_config), gtk_label_new("Valeur:"));
  data->entry_value = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(data->entry_value), "Ex: 42");
  gtk_box_append(GTK_BOX(box_config), data->entry_value);

  data->lbl_insert_parent = gtk_label_new("Parent (Insertion N-ary):");
  gtk_box_append(GTK_BOX(box_config), data->lbl_insert_parent);
  data->entry_insert_parent = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(data->entry_insert_parent), "Ex: 5");
  gtk_box_append(GTK_BOX(box_config), data->entry_insert_parent);
  gtk_widget_set_visible(data->lbl_insert_parent, FALSE);
  gtk_widget_set_visible(data->entry_insert_parent, FALSE);

  data->lbl_parent_hint = gtk_label_new("Nouvelle Valeur (Modifier):");
  gtk_box_append(GTK_BOX(box_config), data->lbl_parent_hint);
  data->entry_parent = gtk_entry_new();
  gtk_box_append(GTK_BOX(box_config), data->entry_parent);

  GtkWidget *btn_box_crud = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_append(GTK_BOX(box_config), btn_box_crud);

  GtkWidget *btn_ins = gtk_button_new_with_label("➕ Insérer");
  GtkWidget *btn_del = gtk_button_new_with_label("🗑️ Supprimer");
  GtkWidget *btn_mod = gtk_button_new_with_label("✏️ Modifier");

  gtk_widget_add_css_class(btn_ins, "btn-insert");
  gtk_widget_add_css_class(btn_del, "btn-delete");
  gtk_widget_add_css_class(btn_mod, "btn-modify");

  g_signal_connect(btn_ins, "clicked", G_CALLBACK(on_insert_clicked), data);
  g_signal_connect(btn_del, "clicked", G_CALLBACK(on_delete_clicked), data);
  g_signal_connect(btn_mod, "clicked", G_CALLBACK(on_modify_clicked), data);

  gtk_box_append(GTK_BOX(btn_box_crud), btn_ins);
  gtk_box_append(GTK_BOX(btn_box_crud), btn_del);
  gtk_box_append(GTK_BOX(btn_box_crud), btn_mod);

  // Group 2: Creation
  GtkWidget *frame_creation = gtk_frame_new("🌱 Création de l'Arbre");
  GtkWidget *box_creation = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_set_margin_top(box_creation, 10);
  gtk_widget_set_margin_bottom(box_creation, 10);
  gtk_widget_set_margin_start(box_creation, 10);
  gtk_widget_set_margin_end(box_creation, 10);
  gtk_frame_set_child(GTK_FRAME(frame_creation), box_creation);
  gtk_box_append(GTK_BOX(left_box), frame_creation);

  gtk_box_append(GTK_BOX(box_creation), gtk_label_new("Type de Données:"));
  const char *data_types[] = {"1️⃣ Entier", "🔢 Float", "🔤 Caractère",
                              "📝 Chaîne", NULL};
  data->combo_data_type =
      gtk_drop_down_new_from_strings(data_types); // Visual only for now
  gtk_box_append(GTK_BOX(box_creation), data->combo_data_type);

  data->radio_manual = gtk_check_button_new_with_label("✍️ Manuelle");
  data->radio_random = gtk_check_button_new_with_label("🎲 Aléatoire");
  gtk_check_button_set_group(GTK_CHECK_BUTTON(data->radio_random),
                             GTK_CHECK_BUTTON(data->radio_manual));
  gtk_check_button_set_active(GTK_CHECK_BUTTON(data->radio_manual), TRUE);

  GtkWidget *box_radios = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_box_append(GTK_BOX(box_radios), data->radio_manual);
  gtk_box_append(GTK_BOX(box_radios), data->radio_random);
  gtk_box_append(GTK_BOX(box_creation), box_radios);

  g_signal_connect(data->radio_manual, "toggled",
                   G_CALLBACK(on_input_mode_changed), data);
  g_signal_connect(data->radio_random, "toggled",
                   G_CALLBACK(on_input_mode_changed), data);

  // Manual Sub-frame
  data->frame_manual = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

  data->box_nary_degree = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_append(GTK_BOX(data->box_nary_degree), gtk_label_new("Degré (N) :"));
  data->entry_nary_degree = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(data->entry_nary_degree), "3");
  gtk_widget_set_size_request(data->entry_nary_degree, 60, -1);
  gtk_box_append(GTK_BOX(data->box_nary_degree), data->entry_nary_degree);
  gtk_box_append(GTK_BOX(data->frame_manual), data->box_nary_degree);
  gtk_widget_set_visible(data->box_nary_degree, FALSE);

  GtkWidget *btn_manual_dlg =
      gtk_button_new_with_label("📝 Saisir Manuellement (Dialog)");
  g_signal_connect(btn_manual_dlg, "clicked",
                   G_CALLBACK(on_manual_input_clicked), data);
  gtk_box_append(GTK_BOX(data->frame_manual), btn_manual_dlg);
  gtk_box_append(GTK_BOX(box_creation), data->frame_manual);

  // Random Sub-frame
  data->frame_random = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_visible(data->frame_random, FALSE);
  gtk_box_append(GTK_BOX(data->frame_random),
                 gtk_label_new("Taille Aléatoire:"));
  data->entry_rand_size = gtk_entry_new();
  gtk_editable_set_text(GTK_EDITABLE(data->entry_rand_size), "10");
  gtk_box_append(GTK_BOX(data->frame_random), data->entry_rand_size);

  GtkWidget *box_gen_btns = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  data->btn_generate = gtk_button_new_with_label("⚡ Générer");
  gtk_widget_add_css_class(data->btn_generate, "btn-generate");

  g_signal_connect(data->btn_generate, "clicked",
                   G_CALLBACK(on_generate_clicked), data);
  gtk_box_append(GTK_BOX(box_gen_btns), data->btn_generate);
  gtk_box_append(GTK_BOX(data->frame_random), box_gen_btns);

  // Bouton Vider (Global)
  data->btn_clear = gtk_button_new_with_label("🧹 Vider");
  gtk_widget_add_css_class(data->btn_clear, "btn-clear");
  g_signal_connect(data->btn_clear, "clicked", G_CALLBACK(on_clear_clicked),
                   data);
  gtk_widget_set_margin_top(data->btn_clear, 5);
  gtk_box_append(GTK_BOX(box_creation), data->frame_random);
  gtk_box_append(GTK_BOX(box_creation), data->btn_clear);

  // Helper buttons
  data->btn_transform = gtk_button_new_with_label("🔄 Convertir -> Binaire");
  g_signal_connect(data->btn_transform, "clicked",
                   G_CALLBACK(on_convert_clicked), data);
  gtk_widget_set_visible(data->btn_transform, FALSE);
  gtk_box_append(GTK_BOX(box_creation), data->btn_transform);

  // Group 3: Parcours
  GtkWidget *frame_trav = gtk_frame_new("🚶 Parcours");
  GtkWidget *box_trav = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_set_margin_top(box_trav, 10);
  gtk_widget_set_margin_bottom(box_trav, 10);
  gtk_widget_set_margin_start(box_trav, 10);
  gtk_widget_set_margin_end(box_trav, 10);
  gtk_frame_set_child(GTK_FRAME(frame_trav), box_trav);
  gtk_box_append(GTK_BOX(left_box), frame_trav);

  const char *trav_types[] = {"⬇️ Profondeur", "↔️ Largeur", NULL};
  const char *trav_methods[] = {"Prefixe (Pré-ordre)", "Infixe (In-ordre)",
                                "Postfixe (Post-ordre)", NULL};
  data->combo_trav_type = gtk_drop_down_new_from_strings(trav_types);
  data->combo_trav_method = gtk_drop_down_new_from_strings(trav_methods);

  g_signal_connect(data->combo_trav_type, "notify::selected",
                   G_CALLBACK(on_trav_type_changed), data);

  gtk_box_append(GTK_BOX(box_trav), gtk_label_new("Type:"));
  gtk_box_append(GTK_BOX(box_trav), data->combo_trav_type);
  gtk_box_append(GTK_BOX(box_trav), gtk_label_new("Méthode:"));
  gtk_box_append(GTK_BOX(box_trav), data->combo_trav_method);

  GtkWidget *btn_exec_trav =
      gtk_button_new_with_label("▶️ Exécuter le Parcours");
  gtk_widget_add_css_class(btn_exec_trav, "btn-execute");
  g_signal_connect(btn_exec_trav, "clicked",
                   G_CALLBACK(on_execute_traversal_clicked), data);
  gtk_box_append(GTK_BOX(box_trav), btn_exec_trav);

  data->lbl_trav_result = gtk_label_new("Résultat...");
  gtk_label_set_wrap(GTK_LABEL(data->lbl_trav_result), TRUE);
  gtk_widget_set_size_request(data->lbl_trav_result, -1, 40);
  gtk_box_append(GTK_BOX(box_trav), data->lbl_trav_result);

  // Logs
  data->log_view = gtk_text_view_new();
  data->log_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->log_view));
  gtk_text_view_set_editable(GTK_TEXT_VIEW(data->log_view), FALSE);
  GtkWidget *log_scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(log_scroll),
                                data->log_view);
  gtk_widget_set_size_request(log_scroll, -1, 150);

  GtkWidget *frame_log = gtk_frame_new("📜 Logs");
  gtk_frame_set_child(GTK_FRAME(frame_log), log_scroll);
  gtk_box_append(GTK_BOX(left_box), frame_log);

  // Bouton Retour
  GtkWidget *btn_back = gtk_button_new_with_label("🔙 Retour Menu");
  gtk_widget_set_margin_top(btn_back, 10);
  g_signal_connect_swapped(btn_back, "clicked", G_CALLBACK(gtk_window_close),
                           data->window);
  gtk_box_append(GTK_BOX(left_box), btn_back);

  // --- Right Panel ---
  GtkWidget *right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_hexpand(right_box, TRUE);
  gtk_box_append(GTK_BOX(main_box), right_box);

  data->lbl_stats = gtk_label_new("Taille: 0 | Hauteur: 0");
  gtk_widget_set_margin_top(data->lbl_stats, 10);
  gtk_box_append(GTK_BOX(right_box), data->lbl_stats);

  GtkWidget *scrolled_window = gtk_scrolled_window_new();
  gtk_widget_set_hexpand(scrolled_window, TRUE);
  gtk_widget_set_vexpand(scrolled_window, TRUE);
  gtk_box_append(GTK_BOX(right_box), scrolled_window);

  data->drawing_area = gtk_drawing_area_new();
  // Ensure we can draw on a large surface
  gtk_widget_set_hexpand(data->drawing_area, TRUE);
  gtk_widget_set_vexpand(data->drawing_area, TRUE);
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(data->drawing_area),
                                 draw_tree, data, NULL);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window),
                                data->drawing_area);

  // Init state
  on_tree_type_changed(NULL, NULL, data);

  g_signal_connect(data->window, "destroy", G_CALLBACK(on_window_destroy),
                   data);
  gtk_window_present(GTK_WINDOW(data->window));
}
