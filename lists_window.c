#include "lists_window.h"
#include "list_algorithms.h"
#include <ctype.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Structure pour gérer l'état de la fenêtre
typedef struct {
  GtkWidget *window;

  // Sidebar widgets
  GtkWidget *type_combo;
  GtkWidget *structure_combo;
  GtkWidget *mode_combo;
  GtkWidget *size_spin;
  GtkWidget *sort_method_combo;

  // Main area widgets
  GtkWidget *results_text_view;
  GtkWidget *drawing_area_before;
  GtkWidget *drawing_area_after;
  GtkWidget *manual_input_box;
  GtkWidget *manual_input_entry;

  // Operations widgets
  GtkWidget *value_entry;
  GtkWidget *position_combo;
  GtkWidget *index_entry;

  // Data
  SimpleList *simple_list;
  SimpleList *simple_list_before;
  DoubleList *double_list;
  DoubleList *double_list_before;
  DataType current_type;
  bool is_double;
  bool is_manual_mode;
  bool has_sorted;
} ListsWindowData;

// ============================================================================
// Fonctions utilitaires
// ============================================================================

static void append_to_text_view(ListsWindowData *data, const char *text) {
  GtkTextBuffer *buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->results_text_view));
  GtkTextIter end;
  gtk_text_buffer_get_end_iter(buffer, &end);
  gtk_text_buffer_insert(buffer, &end, text, -1);
}

static void clear_text_view(ListsWindowData *data) {
  GtkTextBuffer *buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->results_text_view));
  gtk_text_buffer_set_text(buffer, "", -1);
}

static SimpleList *copy_simple_list(SimpleList *src) {
  if (!src)
    return NULL;
  SimpleList *dest = create_simple_list(src->type);
  SimpleNode *current = src->head;
  while (current) {
    insert_simple_at_end(dest, current->data);
    current = current->next;
  }
  return dest;
}

static DoubleList *copy_double_list(DoubleList *src) {
  if (!src)
    return NULL;
  DoubleList *dest = create_double_list(src->type);
  DoubleNode *current = src->head;
  while (current) {
    insert_double_at_end(dest, current->data);
    current = current->next;
  }
  return dest;
}

// ============================================================================
// Fonctions de dessin avec Cairo - FOND NOIR
// ============================================================================

static void draw_simple_list_colored(cairo_t *cr, SimpleList *list, int width,
                                     int height, bool is_after_sort) {
  if (!list || !list->head)
    return;

  const int node_width = 90;
  const int node_height = 50;
  const int arrow_length = 25;
  const int start_x = 30;
  const int start_y = height / 2;

  SimpleNode *current = list->head;
  int x = start_x;

  while (current) {
    // Diviser le nœud en deux parties: valeur (gauche) et pointeur (droite)
    const int data_width = node_width * 0.7;    // 70% pour la valeur
    const int pointer_width = node_width * 0.3; // 30% pour le pointeur

    // Couleur des nœuds: BLEU avant tri, VERT après tri
    if (is_after_sort) {
      cairo_set_source_rgb(cr, 0.2, 0.8, 0.4); // VERT
    } else {
      cairo_set_source_rgb(cr, 0.2, 0.5, 0.9); // BLEU
    }

    // Dessiner le rectangle complet du nœud
    cairo_rectangle(cr, x, start_y - node_height / 2, node_width, node_height);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // Bordure blanche
    cairo_set_line_width(cr, 2);
    cairo_stroke(cr);

    // Ligne de séparation entre valeur et pointeur
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, x + data_width, start_y - node_height / 2);
    cairo_line_to(cr, x + data_width, start_y + node_height / 2);
    cairo_stroke(cr);

    // Texte de la valeur (partie gauche)
    char *value_str = node_data_to_string(current->data, list->type);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);

    cairo_text_extents_t extents;
    cairo_text_extents(cr, value_str, &extents);
    cairo_move_to(cr, x + (data_width - extents.width) / 2,
                  start_y + extents.height / 2);
    cairo_show_text(cr, value_str);
    free(value_str);

    // Petit point dans la partie pointeur pour indiquer l'adresse
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_arc(cr, x + data_width + pointer_width / 2, start_y, 3, 0,
              2 * 3.14159);
    cairo_fill(cr);

    // Flèches blanches
    // Flèches blanches (Always draw forward arrow)
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, x + node_width, start_y);
    cairo_line_to(cr, x + node_width + arrow_length, start_y);
    cairo_stroke(cr);

    cairo_move_to(cr, x + node_width + arrow_length, start_y);
    cairo_line_to(cr, x + node_width + arrow_length - 8, start_y - 5);
    cairo_line_to(cr, x + node_width + arrow_length - 8, start_y + 5);
    cairo_close_path(cr);
    cairo_fill(cr);

    x += node_width + arrow_length;
    current = current->next;
  }

  // "NULL" en ROUGE
  cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 16);
  cairo_move_to(cr, x, start_y + 5);
  cairo_show_text(cr, "NULL");
}

static void draw_double_list_colored(cairo_t *cr, DoubleList *list, int width,
                                     int height, bool is_after_sort) {
  if (!list || !list->head)
    return;

  const int node_width = 90;
  const int node_height = 50;
  const int arrow_length = 25;
  const int start_x = 30;
  const int start_y = height / 2;

  DoubleNode *current = list->head;
  int x = start_x;

  while (current) {
    // Diviser le nœud en deux parties: valeur (gauche) et pointeur (droite)
    const int data_width = node_width * 0.7;    // 70% pour la valeur
    const int pointer_width = node_width * 0.3; // 30% pour le pointeur

    // Couleur des nœuds
    if (is_after_sort) {
      cairo_set_source_rgb(cr, 0.2, 0.8, 0.4); // VERT
    } else {
      cairo_set_source_rgb(cr, 0.2, 0.5, 0.9); // BLEU
    }

    cairo_rectangle(cr, x, start_y - node_height / 2, node_width, node_height);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 2);
    cairo_stroke(cr);

    // Ligne de séparation entre valeur et pointeur
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, x + data_width, start_y - node_height / 2);
    cairo_line_to(cr, x + data_width, start_y + node_height / 2);
    cairo_stroke(cr);

    char *value_str = node_data_to_string(current->data, list->type);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);

    cairo_text_extents_t extents;
    cairo_text_extents(cr, value_str, &extents);
    cairo_move_to(cr, x + (data_width - extents.width) / 2,
                  start_y + extents.height / 2);
    cairo_show_text(cr, value_str);
    free(value_str);

    // Petit point dans la partie pointeur
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_arc(cr, x + data_width + pointer_width / 2, start_y, 3, 0,
              2 * 3.14159);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 2);

    // Flèche next (Always draw forward arrow)
    cairo_move_to(cr, x + node_width, start_y - 10);
    cairo_line_to(cr, x + node_width + arrow_length, start_y - 10);
    cairo_stroke(cr);
    cairo_move_to(cr, x + node_width + arrow_length, start_y - 10);
    cairo_line_to(cr, x + node_width + arrow_length - 8, start_y - 15);
    cairo_line_to(cr, x + node_width + arrow_length - 8, start_y - 5);
    cairo_close_path(cr);
    cairo_fill(cr);

    // Flèche prev (Only if next exists)
    if (current->next) {
      cairo_move_to(cr, x + node_width + arrow_length, start_y + 10);
      cairo_line_to(cr, x + node_width, start_y + 10);
      cairo_stroke(cr);
      cairo_move_to(cr, x + node_width, start_y + 10);
      cairo_line_to(cr, x + node_width + 8, start_y + 5);
      cairo_line_to(cr, x + node_width + 8, start_y + 15);
      cairo_close_path(cr);
      cairo_fill(cr);
    }

    x += node_width + arrow_length;
    current = current->next;
  }

  cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 16);
  cairo_move_to(cr, x, start_y + 5);
  cairo_show_text(cr, "NULL");
}

static void on_draw_before(GtkDrawingArea *area, cairo_t *cr, int width,
                           int height, gpointer user_data) {
  ListsWindowData *data = (ListsWindowData *)user_data;

  // Fond NOIR
  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
  cairo_paint(cr);

  if (data->has_sorted) {
    if (data->is_double && data->double_list_before) {
      draw_double_list_colored(cr, data->double_list_before, width, height,
                               false);
    } else if (!data->is_double && data->simple_list_before) {
      draw_simple_list_colored(cr, data->simple_list_before, width, height,
                               false);
    }
  } else {
    if (data->is_double && data->double_list) {
      draw_double_list_colored(cr, data->double_list, width, height, false);
    } else if (!data->is_double && data->simple_list) {
      draw_simple_list_colored(cr, data->simple_list, width, height, false);
    }
  }
}

static void on_draw_after(GtkDrawingArea *area, cairo_t *cr, int width,
                          int height, gpointer user_data) {
  ListsWindowData *data = (ListsWindowData *)user_data;

  // Fond NOIR
  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
  cairo_paint(cr);

  if (data->has_sorted) {
    if (data->is_double && data->double_list) {
      draw_double_list_colored(cr, data->double_list, width, height, true);
    } else if (!data->is_double && data->simple_list) {
      draw_simple_list_colored(cr, data->simple_list, width, height, true);
    }
  }
}

// ============================================================================
// Callbacks - Fenêtre modale pour mode manuel
// ============================================================================

typedef struct {
  gboolean accepted;
  GtkWidget *text_view;
  ListsWindowData *list_data;
  int size;
  char *captured_text;
} DialogData;

static void on_dialog_ok_clicked(GtkWidget *widget, gpointer user_data) {
  DialogData *dialog_data = (DialogData *)user_data;

  // Capturer le texte AVANT que la fenêtre soit détruite
  GtkTextBuffer *buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(dialog_data->text_view));
  GtkTextIter start, end;
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  dialog_data->captured_text =
      gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
  dialog_data->accepted = TRUE;
}

static void show_manual_input_dialog(ListsWindowData *data, int size) {
  printf("=== DEBUG: show_manual_input_dialog appelée, size=%d ===\n", size);
  fflush(stdout);

  GtkWidget *dialog = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(dialog), "Saisie manuelle des valeurs");
  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(data->window));
  gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);

  GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_start(main_box, 20);
  gtk_widget_set_margin_end(main_box, 20);
  gtk_widget_set_margin_top(main_box, 20);
  gtk_widget_set_margin_bottom(main_box, 20);
  gtk_window_set_child(GTK_WINDOW(dialog), main_box);

  char instruction[256];
  snprintf(instruction, sizeof(instruction),
           "Entrez %d valeur(s) séparée(s) par des espaces :", size);
  GtkWidget *label = gtk_label_new(instruction);
  gtk_box_append(GTK_BOX(main_box), label);

  GtkWidget *scrolled = gtk_scrolled_window_new();
  gtk_widget_set_vexpand(scrolled, TRUE);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkWidget *text_view = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), text_view);
  gtk_box_append(GTK_BOX(main_box), scrolled);

  GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(button_box, GTK_ALIGN_END);
  gtk_box_append(GTK_BOX(main_box), button_box);

  GtkWidget *btn_cancel = gtk_button_new_with_label("Annuler");
  GtkWidget *btn_ok = gtk_button_new_with_label("OK");
  gtk_box_append(GTK_BOX(button_box), btn_cancel);
  gtk_box_append(GTK_BOX(button_box), btn_ok);

  DialogData *dialog_data = g_new0(DialogData, 1);
  dialog_data->accepted = FALSE;
  dialog_data->text_view = text_view;
  dialog_data->list_data = data;
  dialog_data->size = size;
  dialog_data->captured_text = NULL;

  g_signal_connect(btn_ok, "clicked", G_CALLBACK(on_dialog_ok_clicked),
                   dialog_data);
  g_signal_connect_swapped(btn_ok, "clicked", G_CALLBACK(gtk_window_destroy),
                           dialog);
  g_signal_connect_swapped(btn_cancel, "clicked",
                           G_CALLBACK(gtk_window_destroy), dialog);

  gtk_window_present(GTK_WINDOW(dialog));

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  g_signal_connect(dialog, "destroy", G_CALLBACK(g_main_loop_quit), loop);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  // Traiter le texte capturé
  if (dialog_data->accepted && dialog_data->captured_text &&
      strlen(dialog_data->captured_text) > 0) {
    printf("DEBUG: Texte capturé: '%s'\n", dialog_data->captured_text);
    printf("DEBUG: Taille demandée: %d\n", size);
    fflush(stdout);

    char *token = strtok(dialog_data->captured_text, " \n\t");
    int count = 0;

    while (token && count < size) {
      NodeData node_data;

      switch (data->current_type) {
      case TYPE_INT:
        node_data.int_val = atoi(token);
        printf("DEBUG: Ajout entier %d\n", node_data.int_val);
        fflush(stdout);
        break;
      case TYPE_FLOAT:
        node_data.float_val = atof(token);
        printf("DEBUG: Ajout réel %f\n", node_data.float_val);
        fflush(stdout);
        break;
      case TYPE_CHAR:
        node_data.char_val = token[0];
        printf("DEBUG: Ajout char %c\n", node_data.char_val);
        fflush(stdout);
        break;
      case TYPE_STRING:
        node_data.string_val = strdup(token);
        printf("DEBUG: Ajout string %s\n", node_data.string_val);
        fflush(stdout);
        break;
      }

      if (data->is_double) {
        insert_double_at_end(data->double_list, node_data);
        printf("DEBUG: Inséré dans double list, taille = %d\n",
               data->double_list->size);
        fflush(stdout);
      } else {
        insert_simple_at_end(data->simple_list, node_data);
        printf("DEBUG: Inséré dans simple list, taille = %d\n",
               data->simple_list->size);
        fflush(stdout);
      }

      count++;
      token = strtok(NULL, " \n\t");
    }
    printf("DEBUG: Total éléments insérés: %d\n", count);
    fflush(stdout);
  } else {
    printf("DEBUG: Pas de texte capturé ou annulé\n");
    printf("DEBUG: accepted=%d, captured_text=%p\n", dialog_data->accepted,
           dialog_data->captured_text);
    fflush(stdout);
  }

  if (dialog_data->captured_text) {
    g_free(dialog_data->captured_text);
  }
  g_free(dialog_data);
}

// ============================================================================
// Callbacks pour les boutons
// ============================================================================

static void on_create_list_clicked(GtkWidget *widget, gpointer user_data) {
  ListsWindowData *data = (ListsWindowData *)user_data;

  // 1. Nettoyage des listes existantes
  if (data->simple_list) {
    free_simple_list(data->simple_list);
    data->simple_list = NULL;
  }
  if (data->simple_list_before) {
    free_simple_list(data->simple_list_before);
    data->simple_list_before = NULL;
  }
  if (data->double_list) {
    free_double_list(data->double_list);
    data->double_list = NULL;
  }
  if (data->double_list_before) {
    free_double_list(data->double_list_before);
    data->double_list_before = NULL;
  }

  // 2. Lecture des paramètres UI
  int type_index = gtk_drop_down_get_selected(GTK_DROP_DOWN(data->type_combo));
  data->current_type = (DataType)type_index;

  int structure_index =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(data->structure_combo));
  data->is_double = (structure_index == 1);

  int mode_index = gtk_drop_down_get_selected(GTK_DROP_DOWN(data->mode_combo));
  data->is_manual_mode = (mode_index == 1);

  int size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->size_spin));

  // 3. Création de la structure
  if (data->is_double) {
    data->double_list = create_double_list(data->current_type);
  } else {
    data->simple_list = create_simple_list(data->current_type);
  }

  // 4. Remplissage
  // 4. Remplissage
  if (data->is_manual_mode) {
    const char *input_text =
        gtk_editable_get_text(GTK_EDITABLE(data->manual_input_entry));

    // Validation préliminaire
    char *text_copy = (input_text && strlen(input_text) > 0)
                          ? strdup(input_text)
                          : strdup("");

    // On compte d'abord les tokens et on valide
    // On utilise une copie temporaire pour ne pas casser la string pour le
    // parsing réel
    char *temp_str = strdup(text_copy);
    char *token = strtok(temp_str, " \n\t");
    bool validation_error = false;
    char error_message[512] = "";

    while (token) {
      if (data->current_type == TYPE_INT) {
        char *endptr;
        strtol(token, &endptr, 10);
        if (*endptr != '\0') {
          validation_error = true;
          snprintf(error_message, sizeof(error_message),
                   "Erreur: valeur non entière détectée '%s'", token);
          break;
        }
      } else if (data->current_type == TYPE_FLOAT) {
        char *endptr;
        strtof(token, &endptr);
        if (*endptr != '\0') {
          validation_error = true;
          snprintf(error_message, sizeof(error_message),
                   "Erreur: valeur non réelle détectée '%s'", token);
          break;
        }
      } else if (data->current_type == TYPE_CHAR) {
        if (strlen(token) > 1) {
          validation_error = true;
          snprintf(error_message, sizeof(error_message),
                   "Erreur: chaîne de caractères détectée '%s' alors que "
                   "'Caractère' est attendu",
                   token);
          break;
        }
        if (isdigit(token[0])) {
          validation_error = true;
          snprintf(
              error_message, sizeof(error_message),
              "Erreur: nombre détecté '%s' alors que 'Caractère' est attendu",
              token);
          break;
        }
      } else if (data->current_type == TYPE_STRING) {
        // Check si c'est un nombre (entier ou réel)
        char *endptr;
        strtod(token, &endptr);
        if (*endptr == '\0') { // Tout le token a été consommé comme nombre
          validation_error = true;
          snprintf(
              error_message, sizeof(error_message),
              "Erreur: nombre détecté '%s' alors que 'Chaîne' est attendue",
              token);
          break;
        }
      }
      token = strtok(NULL, " \n\t");
    }
    free(temp_str);

    if (validation_error) {
      GtkWidget *dialog = gtk_message_dialog_new(
          GTK_WINDOW(data->window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
          GTK_BUTTONS_OK, "%s", error_message);
      gtk_window_present(GTK_WINDOW(dialog));
      g_signal_connect_swapped(dialog, "response",
                               G_CALLBACK(gtk_window_destroy), dialog);

      // Nettoyage si on a déjà alloué
      if (data->simple_list) {
        free_simple_list(data->simple_list);
        data->simple_list = NULL;
      }
      if (data->double_list) {
        free_double_list(data->double_list);
        data->double_list = NULL;
      }
      free(text_copy);
      return; // STOP
    }

    // Réinitialisation pour parsing réel
    token = strtok(text_copy, " \n\t");
    int count = 0;

    // Mode MANUEL : La taille est déterminée par le nombre d'inputs, on ignore
    // 'size' du spinner
    while (token) {
      NodeData node_data = {0};

      switch (data->current_type) {
      case TYPE_INT:
        node_data.int_val = atoi(token);
        break;
      case TYPE_FLOAT:
        node_data.float_val = atof(token);
        break;
      case TYPE_CHAR:
        node_data.char_val = token[0];
        break;
      case TYPE_STRING:
        node_data.string_val = strdup(token);
        break;
      }

      if (data->is_double) {
        insert_double_at_end(data->double_list, node_data);
      } else {
        insert_simple_at_end(data->simple_list, node_data);
      }
      count++;
      token = strtok(NULL, " \n\t");
    }
    free(text_copy);

    // Log
    clear_text_view(data);
    append_to_text_view(data, "Liste créée (Manuel) :\n");

  } else {
    // Mode Aléatoire
    // Ici on utilise la taille du spinner
    if (data->is_double) {
      fill_double_list_random(data->double_list, size);
    } else {
      fill_simple_list_random(data->simple_list, size);
    }
    clear_text_view(data);
    append_to_text_view(data, "Liste créée (Aléatoire) :\n");
  }

  // 5. Affichage Textuel de la liste
  char buffer[512];
  if (data->is_double && data->double_list) {
    DoubleNode *current = data->double_list->head;
    while (current) {
      char *str = node_data_to_string(current->data, data->current_type);
      snprintf(buffer, sizeof(buffer), "%s -> ", str);
      append_to_text_view(data, buffer);
      free(str);
      current = current->next;
    }
  } else if (data->simple_list) {
    SimpleNode *current = data->simple_list->head;
    while (current) {
      char *str = node_data_to_string(current->data, data->current_type);
      snprintf(buffer, sizeof(buffer), "%s -> ", str);
      append_to_text_view(data, buffer);
      free(str);
      current = current->next;
    }
  }
  append_to_text_view(data, "NULL\n");

  data->has_sorted = false;

  // Calcul largeur requise pour le dessin
  int list_count = 0;
  if (data->is_double && data->double_list)
    list_count = data->double_list->size;
  else if (data->simple_list)
    list_count = data->simple_list->size;

  int required_width = 30 + (list_count * 115) + 150;
  if (required_width < 2000)
    required_width = 2000;

  gtk_widget_set_size_request(data->drawing_area_before, required_width, 200);
  gtk_widget_set_size_request(data->drawing_area_after, required_width, 200);

  gtk_widget_queue_draw(data->drawing_area_before);
  gtk_widget_queue_draw(data->drawing_area_after);
}

static void on_sort_clicked(GtkWidget *widget, gpointer user_data) {
  ListsWindowData *data = (ListsWindowData *)user_data;

  if (!data->simple_list && !data->double_list) {
    return;
  }

  if (data->simple_list_before) {
    free_simple_list(data->simple_list_before);
    data->simple_list_before = NULL;
  }
  if (data->double_list_before) {
    free_double_list(data->double_list_before);
    data->double_list_before = NULL;
  }

  if (data->is_double) {
    data->double_list_before = copy_double_list(data->double_list);
  } else {
    data->simple_list_before = copy_simple_list(data->simple_list);
  }

  int method_index =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(data->sort_method_combo));

  const char *method_names[] = {"Tri à bulle", "Tri par insertion",
                                "Tri Shell"};

  clock_t start = clock();

  if (data->is_double) {
    switch (method_index) {
    case 0:
      bubble_sort_double(data->double_list);
      break;
    case 1:
      insertion_sort_double(data->double_list);
      break;
    case 2:
      selection_sort_double(data->double_list);
      break;
    }
  } else {
    switch (method_index) {
    case 0:
      bubble_sort_simple(data->simple_list);
      break;
    case 1:
      insertion_sort_simple(data->simple_list);
      break;
    case 2:
      selection_sort_simple(data->simple_list);
      break;
    }
  }

  clock_t end = clock();
  double time_s = ((double)(end - start) / CLOCKS_PER_SEC);
  double time_ms = time_s * 1000; // Convertir en millisecondes

  data->has_sorted = true;

  clear_text_view(data);
  char buffer[512];
  snprintf(buffer, sizeof(buffer),
           "Liste triée (méthode %s), Temps = %.6f ms\n",
           method_names[method_index], time_ms);
  append_to_text_view(data, buffer);

  gtk_widget_queue_draw(data->drawing_area_before);
  gtk_widget_queue_draw(data->drawing_area_after);
}

static void on_compare_sorts_clicked(GtkWidget *widget, gpointer user_data) {
  ListsWindowData *data = (ListsWindowData *)user_data;

  if (!data->simple_list && !data->double_list) {
    return;
  }

  clear_text_view(data);
  append_to_text_view(data, "--- Comparaison des Tris ---\n");

  const char *methods[] = {"Tri à bulle", "Tri par insertion", "Tri Shell",
                           "Tri rapide"};
  char buffer[256];

  // Répéter le tri plusieurs fois pour obtenir une mesure précise
  int iterations = 1000; // Augmenté pour plus de précision

  for (int i = 0; i < 4; i++) {
    clock_t total_time = 0;

    for (int iter = 0; iter < iterations; iter++) {
      SimpleList *test_simple = NULL;
      DoubleList *test_double = NULL;

      if (data->is_double) {
        test_double = copy_double_list(data->double_list);
      } else {
        test_simple = copy_simple_list(data->simple_list);
      }

      clock_t start = clock();

      if (data->is_double) {
        switch (i) {
        case 0:
          bubble_sort_double(test_double);
          break;
        case 1:
          insertion_sort_double(test_double);
          break;
        case 2:
          selection_sort_double(test_double);
          break;
        case 3:
          selection_sort_double(test_double); // Utiliser selection pour rapide
          break;
        }
      } else {
        switch (i) {
        case 0:
          bubble_sort_simple(test_simple);
          break;
        case 1:
          insertion_sort_simple(test_simple);
          break;
        case 2:
          selection_sort_simple(test_simple);
          break;
        case 3:
          selection_sort_simple(test_simple); // Utiliser selection pour rapide
          break;
        }
      }

      clock_t end = clock();
      total_time += (end - start);

      if (test_simple)
        free_simple_list(test_simple);
      if (test_double)
        free_double_list(test_double);
    }

    // Calculer le temps moyen
    double avg_time_s = ((double)total_time / CLOCKS_PER_SEC) / iterations;
    double avg_time_ms = avg_time_s * 1000; // Convertir en millisecondes

    snprintf(buffer, sizeof(buffer), "%s: %.6f ms\n", methods[i], avg_time_ms);
    append_to_text_view(data, buffer);
  }

  snprintf(buffer, sizeof(buffer), "\n(Moyenne sur %d itérations)\n",
           iterations);
  append_to_text_view(data, buffer);
}

static void on_reset_clicked(GtkWidget *widget, gpointer user_data) {
  ListsWindowData *data = (ListsWindowData *)user_data;

  if (data->simple_list) {
    free_simple_list(data->simple_list);
    data->simple_list = NULL;
  }
  if (data->simple_list_before) {
    free_simple_list(data->simple_list_before);
    data->simple_list_before = NULL;
  }
  if (data->double_list) {
    free_double_list(data->double_list);
    data->double_list = NULL;
  }
  if (data->double_list_before) {
    free_double_list(data->double_list_before);
    data->double_list_before = NULL;
  }

  data->has_sorted = false;
  clear_text_view(data);

  gtk_widget_queue_draw(data->drawing_area_before);
  gtk_widget_queue_draw(data->drawing_area_after);
}

static void on_save_clicked(GtkWidget *widget, gpointer user_data) {
  ListsWindowData *data = (ListsWindowData *)user_data;

  if (!data->simple_list && !data->double_list) {
    return;
  }

  FILE *file = fopen("resultats_liste.txt", "w");
  if (!file) {
    return;
  }

  fprintf(file, "=== Résultats Liste ===\n");
  fprintf(file, "Type: %s\n",
          data->current_type == TYPE_INT     ? "Entier"
          : data->current_type == TYPE_FLOAT ? "Réel"
          : data->current_type == TYPE_CHAR  ? "Caractère"
                                             : "Chaîne");
  fprintf(file, "Structure: %s\n", data->is_double ? "Double" : "Simple");

  if (data->is_double && data->double_list) {
    fprintf(file, "Taille: %d\n", data->double_list->size);
    fprintf(file, "Éléments: ");
    DoubleNode *current = data->double_list->head;
    while (current) {
      char *str = node_data_to_string(current->data, data->current_type);
      fprintf(file, "%s ", str);
      free(str);
      current = current->next;
    }
  } else if (data->simple_list) {
    fprintf(file, "Taille: %d\n", data->simple_list->size);
    fprintf(file, "Éléments: ");
    SimpleNode *current = data->simple_list->head;
    while (current) {
      char *str = node_data_to_string(current->data, data->current_type);
      fprintf(file, "%s ", str);
      free(str);
      current = current->next;
    }
  }

  fprintf(file, "\n");
  fclose(file);
}

// Opérations individuelles
static void on_insert_operation(GtkWidget *widget, gpointer user_data) {
  ListsWindowData *data = (ListsWindowData *)user_data;

  if (!data->simple_list && !data->double_list)
    return;

  const char *value_text =
      gtk_editable_get_text(GTK_EDITABLE(data->value_entry));
  int position_idx =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(data->position_combo));
  const char *index_text =
      gtk_editable_get_text(GTK_EDITABLE(data->index_entry));
  int index = atoi(index_text);

  NodeData node_data;
  switch (data->current_type) {
  case TYPE_INT:
    node_data.int_val = atoi(value_text);
    break;
  case TYPE_FLOAT:
    node_data.float_val = atof(value_text);
    break;
  case TYPE_CHAR:
    node_data.char_val = value_text[0];
    break;
  case TYPE_STRING:
    node_data.string_val = strdup(value_text);
    break;
  }

  if (data->is_double) {
    if (position_idx == 0) {
      insert_double_at_beginning(data->double_list, node_data);
    } else if (position_idx == 1) {
      insert_double_at_end(data->double_list, node_data);
    } else {
      insert_double_at_position(data->double_list, node_data, index);
    }
  } else {
    if (position_idx == 0) {
      insert_simple_at_beginning(data->simple_list, node_data);
    } else if (position_idx == 1) {
      insert_simple_at_end(data->simple_list, node_data);
    } else {
      insert_simple_at_position(data->simple_list, node_data, index);
    }
  }

  gtk_editable_set_text(GTK_EDITABLE(data->value_entry), "");
  data->has_sorted = false;
  gtk_widget_queue_draw(data->drawing_area_before);
  gtk_widget_queue_draw(data->drawing_area_after);
}

static void on_delete_operation(GtkWidget *widget, gpointer user_data) {
  ListsWindowData *data = (ListsWindowData *)user_data;

  if (!data->simple_list && !data->double_list)
    return;

  const char *index_text =
      gtk_editable_get_text(GTK_EDITABLE(data->index_entry));
  int index = atoi(index_text);

  if (data->is_double) {
    delete_double_at_position(data->double_list, index);
  } else {
    delete_simple_at_position(data->simple_list, index);
  }

  gtk_editable_set_text(GTK_EDITABLE(data->index_entry), "");
  data->has_sorted = false;
  gtk_widget_queue_draw(data->drawing_area_before);
  gtk_widget_queue_draw(data->drawing_area_after);
}

static void on_modify_operation(GtkWidget *widget, gpointer user_data) {
  ListsWindowData *data = (ListsWindowData *)user_data;

  if (!data->simple_list && !data->double_list)
    return;

  const char *value_text =
      gtk_editable_get_text(GTK_EDITABLE(data->value_entry));
  const char *index_text =
      gtk_editable_get_text(GTK_EDITABLE(data->index_entry));
  int index = atoi(index_text);

  NodeData node_data;
  switch (data->current_type) {
  case TYPE_INT:
    node_data.int_val = atoi(value_text);
    break;
  case TYPE_FLOAT:
    node_data.float_val = atof(value_text);
    break;
  case TYPE_CHAR:
    node_data.char_val = value_text[0];
    break;
  case TYPE_STRING:
    node_data.string_val = strdup(value_text);
    break;
  }

  if (data->is_double) {
    modify_double_at_position(data->double_list, node_data, index);
  } else {
    modify_simple_at_position(data->simple_list, node_data, index);
  }

  gtk_editable_set_text(GTK_EDITABLE(data->value_entry), "");
  gtk_editable_set_text(GTK_EDITABLE(data->index_entry), "");
  data->has_sorted = false;
  gtk_widget_queue_draw(data->drawing_area_before);
  gtk_widget_queue_draw(data->drawing_area_after);
}

static void on_window_destroy(GtkWidget *widget, gpointer user_data) {
  ListsWindowData *data = (ListsWindowData *)user_data;

  if (data->simple_list) {
    free_simple_list(data->simple_list);
  }
  if (data->simple_list_before) {
    free_simple_list(data->simple_list_before);
  }
  if (data->double_list) {
    free_double_list(data->double_list);
  }
  if (data->double_list_before) {
    free_double_list(data->double_list_before);
  }
  free(data);
}

static void on_mode_changed(GtkDropDown *dropdown, GParamSpec *pspec,
                            gpointer user_data) {
  ListsWindowData *data = (ListsWindowData *)user_data;
  int mode_idx = gtk_drop_down_get_selected(dropdown);
  gtk_widget_set_visible(data->manual_input_box, mode_idx == 1);
  // Si mode manuel (1), on désactive le spin button de taille
  // Si mode aléatoire (0), on l'active
  gtk_widget_set_sensitive(data->size_spin, mode_idx == 0);
}

// ============================================================================
// Edition Interactive (Click & Edit)
// ============================================================================

typedef struct {
  ListsWindowData *data;
  int index;
  GtkWidget *entry;
  GtkWidget *window;
} EditDialogData;

static void on_edit_dialog_ok(GtkWidget *btn, gpointer user_data) {
  EditDialogData *dlg = (EditDialogData *)user_data;
  const char *text = gtk_editable_get_text(GTK_EDITABLE(dlg->entry));
  NodeData val = {0};
  bool valid = true;

  switch (dlg->data->current_type) {
  case TYPE_INT:
    val.int_val = atoi(text);
    break;
  case TYPE_FLOAT:
    val.float_val = atof(text);
    break;
  case TYPE_CHAR:
    val.char_val = text[0];
    break;
  case TYPE_STRING:
    val.string_val = strdup(text);
    break;
  }

  if (valid) {
    // Si la liste était triée, on annule le tri et on modifie la liste
    // originale
    if (dlg->data->has_sorted) {
      if (dlg->data->is_double) {
        if (dlg->data->double_list)
          free_double_list(dlg->data->double_list); // Free sorted
        dlg->data->double_list =
            dlg->data->double_list_before; // Restore original
        dlg->data->double_list_before = NULL;

        modify_double_at_position(dlg->data->double_list, val, dlg->index);
      } else {
        if (dlg->data->simple_list)
          free_simple_list(dlg->data->simple_list); // Free sorted
        dlg->data->simple_list =
            dlg->data->simple_list_before; // Restore original
        dlg->data->simple_list_before = NULL;

        modify_simple_at_position(dlg->data->simple_list, val, dlg->index);
      }
      dlg->data->has_sorted = false;

      // Mettre à jour l'affichage txt
      clear_text_view(dlg->data);
      append_to_text_view(dlg->data,
                          "Modification effectuée. Tri réinitialisé.\n");

      // Réafficher la liste courante
      char buffer[512];
      if (dlg->data->is_double && dlg->data->double_list) {
        DoubleNode *current = dlg->data->double_list->head;
        while (current) {
          char *str =
              node_data_to_string(current->data, dlg->data->current_type);
          snprintf(buffer, sizeof(buffer), "%s -> ", str);
          append_to_text_view(dlg->data, buffer);
          free(str);
          current = current->next;
        }
      } else if (dlg->data->simple_list) {
        SimpleNode *current = dlg->data->simple_list->head;
        while (current) {
          char *str =
              node_data_to_string(current->data, dlg->data->current_type);
          snprintf(buffer, sizeof(buffer), "%s -> ", str);
          append_to_text_view(dlg->data, buffer);
          free(str);
          current = current->next;
        }
      }
      append_to_text_view(dlg->data, "NULL\n");

    } else {
      // Cas normal : modification directe
      if (dlg->data->is_double) {
        modify_double_at_position(dlg->data->double_list, val, dlg->index);
      } else {
        modify_simple_at_position(dlg->data->simple_list, val, dlg->index);
      }

      // Rafraichir le log
      append_to_text_view(dlg->data, "Valeur modifiée.\n");
    }
  }

  gtk_widget_queue_draw(dlg->data->drawing_area_before);
  gtk_widget_queue_draw(dlg->data->drawing_area_after);
  gtk_window_destroy(GTK_WINDOW(dlg->window));
  g_free(dlg);
}

static void prompt_node_edit(ListsWindowData *data, int index) {
  GtkWidget *dialog = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(dialog), "Modifier Valeur");
  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(data->window));
  gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 100);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_top(box, 10);
  gtk_widget_set_margin_bottom(box, 10);
  gtk_widget_set_margin_start(box, 10);
  gtk_widget_set_margin_end(box, 10);
  gtk_window_set_child(GTK_WINDOW(dialog), box);

  char buf[64];
  snprintf(buf, sizeof(buf), "Nouvelle valeur pour l'indice %d :", index);
  gtk_box_append(GTK_BOX(box), gtk_label_new(buf));

  GtkWidget *entry = gtk_entry_new();
  gtk_box_append(GTK_BOX(box), entry);

  GtkWidget *btn = gtk_button_new_with_label("Valider");
  gtk_box_append(GTK_BOX(box), btn);

  EditDialogData *dlg = g_new0(EditDialogData, 1);
  dlg->data = data;
  dlg->index = index;
  dlg->entry = entry;
  dlg->window = dialog;

  g_signal_connect(btn, "clicked", G_CALLBACK(on_edit_dialog_ok), dlg);
  gtk_window_present(GTK_WINDOW(dialog));
}

static void on_list_click(GtkGestureClick *gesture, int n_press, double x,
                          double y, gpointer user_data) {
  ListsWindowData *data = (ListsWindowData *)user_data;

  // Vérifier qu'il y a une liste
  if (!data->simple_list && !data->double_list)
    return;

  // Paramètres de dessin (doivent correspondre avec on_draw)
  const int start_x = 30;
  const int node_width = 90;
  const int arrow_length = 25;
  const int total_width = node_width + arrow_length;
  const int node_height = 50;

  // Obtenir la hauteur réelle du widget
  int widget_height = gtk_widget_get_height(data->drawing_area_before);
  if (widget_height <= 0)
    widget_height = 250; // Fallback

  const int center_y = widget_height / 2;
  const int half_height = node_height / 2;

  // Vérifier si le clic est dans la zone verticale des nœuds
  if (y < center_y - half_height || y > center_y + half_height)
    return;
  if (x < start_x)
    return;

  int rel_x = (int)x - start_x;
  int index = rel_x / total_width;
  int offset = rel_x % total_width;

  if (offset > node_width)
    return; // Clic sur la flèche

  int size = 0;
  if (data->is_double && data->double_list)
    size = data->double_list->size;
  else if (data->simple_list)
    size = data->simple_list->size;

  if (index >= 0 && index < size) {
    prompt_node_edit(data, index);
  }
}

// ============================================================================
// Fonction principale - NOUVEAU LAYOUT AVEC SIDEBAR
// ============================================================================

void open_lists_window(GtkWindow *parent) {
  ListsWindowData *data = g_new0(ListsWindowData, 1);

  // Fenêtre principale
  data->window = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(data->window), "Gestion de Listes");
  gtk_window_set_default_size(GTK_WINDOW(data->window), 1200, 750);
  // Ne pas définir comme transient pour avoir tous les boutons de contrôle
  // gtk_window_set_transient_for(GTK_WINDOW(data->window), parent);

  // Conteneur horizontal principal avec panneau divisible
  GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_window_set_child(GTK_WINDOW(data->window), paned);

  // ========== SIDEBAR GAUCHE ==========
  GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
  gtk_widget_set_size_request(sidebar, 280, -1);
  gtk_widget_set_margin_start(sidebar, 15);
  gtk_widget_set_margin_end(sidebar, 15);
  gtk_widget_set_margin_top(sidebar, 15);
  gtk_widget_set_margin_bottom(sidebar, 15);
  gtk_widget_set_valign(sidebar, GTK_ALIGN_FILL);
  gtk_widget_set_vexpand(sidebar, TRUE);
  gtk_paned_set_start_child(GTK_PANED(paned), sidebar);

  // Type de données
  GtkWidget *type_label = gtk_label_new("Type de données:");
  gtk_widget_set_halign(type_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(sidebar), type_label);

  const char *types[] = {"Entier", "Réel", "Caractère", "Chaîne", NULL};
  data->type_combo = gtk_drop_down_new_from_strings(types);
  gtk_box_append(GTK_BOX(sidebar), data->type_combo);

  // Structure
  GtkWidget *struct_label = gtk_label_new("Structure:");
  gtk_widget_set_halign(struct_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(sidebar), struct_label);

  const char *structures[] = {"Simple", "Double", NULL};
  data->structure_combo = gtk_drop_down_new_from_strings(structures);
  gtk_box_append(GTK_BOX(sidebar), data->structure_combo);

  // Mode de remplissage
  GtkWidget *mode_label = gtk_label_new("Mode de remplissage:");
  gtk_widget_set_halign(mode_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(sidebar), mode_label);

  const char *modes[] = {"Aléatoire", "Manuel", NULL};
  data->mode_combo = gtk_drop_down_new_from_strings(modes);
  gtk_box_append(GTK_BOX(sidebar), data->mode_combo);

  // Champ de saisie manuelle (caché par défaut)
  data->manual_input_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_append(GTK_BOX(sidebar), data->manual_input_box);

  GtkWidget *manual_label = gtk_label_new("Valeurs (séparées par espaces):");
  gtk_widget_set_halign(manual_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(data->manual_input_box), manual_label);

  data->manual_input_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(data->manual_input_entry),
                                 "Ex: 9 3 7 1 5");
  gtk_box_append(GTK_BOX(data->manual_input_box), data->manual_input_entry);

  gtk_widget_set_visible(data->manual_input_box, FALSE);

  // Callback pour montrer/cacher le champ manuel
  g_signal_connect_swapped(data->mode_combo, "notify::selected",
                           G_CALLBACK(gtk_widget_queue_draw),
                           data->manual_input_box);

  // Connecter un signal pour afficher/masquer selon le mode
  g_object_set_data(G_OBJECT(data->mode_combo), "manual_box",
                    data->manual_input_box);
  g_signal_connect(data->mode_combo, "notify::selected",
                   G_CALLBACK(on_mode_changed), data);

  // Taille de la liste
  GtkWidget *size_label = gtk_label_new("Taille de la liste:");
  gtk_widget_set_halign(size_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(sidebar), size_label);

  data->size_spin = gtk_spin_button_new_with_range(0, 1000000, 1);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->size_spin), 0);
  gtk_box_append(GTK_BOX(sidebar), data->size_spin);

  // Méthode de tri
  GtkWidget *method_label = gtk_label_new("Méthode de tri:");
  gtk_widget_set_halign(method_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(sidebar), method_label);

  const char *sort_methods[] = {"Tri à bulle", "Tri par insertion", "Tri Shell",
                                "Tri rapide", NULL};
  data->sort_method_combo = gtk_drop_down_new_from_strings(sort_methods);
  gtk_box_append(GTK_BOX(sidebar), data->sort_method_combo);

  // ========== BOUTONS DANS LA SIDEBAR ==========
  // Ligne de boutons 1
  GtkWidget *btn_row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_widget_set_margin_top(btn_row1, 20);
  gtk_box_append(GTK_BOX(sidebar), btn_row1);

  GtkWidget *btn_create = gtk_button_new_with_label("Créer");
  g_signal_connect(btn_create, "clicked", G_CALLBACK(on_create_list_clicked),
                   data);
  gtk_widget_set_hexpand(btn_create, TRUE);
  gtk_box_append(GTK_BOX(btn_row1), btn_create);

  GtkWidget *btn_sort = gtk_button_new_with_label("Trier");
  g_signal_connect(btn_sort, "clicked", G_CALLBACK(on_sort_clicked), data);
  gtk_widget_set_hexpand(btn_sort, TRUE);
  gtk_box_append(GTK_BOX(btn_row1), btn_sort);

  // Ligne de boutons 2
  GtkWidget *btn_row2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_append(GTK_BOX(sidebar), btn_row2);

  GtkWidget *btn_compare = gtk_button_new_with_label("Comparer");
  g_signal_connect(btn_compare, "clicked", G_CALLBACK(on_compare_sorts_clicked),
                   data);
  gtk_widget_set_hexpand(btn_compare, TRUE);
  gtk_box_append(GTK_BOX(btn_row2), btn_compare);

  GtkWidget *btn_reset = gtk_button_new_with_label("Reinit.");
  g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_reset_clicked), data);
  gtk_widget_set_hexpand(btn_reset, TRUE);
  gtk_box_append(GTK_BOX(btn_row2), btn_reset);

  // Ligne de boutons 3
  GtkWidget *btn_row3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_append(GTK_BOX(sidebar), btn_row3);

  GtkWidget *btn_save = gtk_button_new_with_label("Enregistrer");
  g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save_clicked), data);
  gtk_widget_set_hexpand(btn_save, TRUE);
  gtk_box_append(GTK_BOX(btn_row3), btn_save);

  GtkWidget *btn_return = gtk_button_new_with_label("🔙 Retour");
  g_signal_connect_swapped(btn_return, "clicked", G_CALLBACK(gtk_window_close),
                           data->window);
  gtk_widget_set_hexpand(btn_return, TRUE);
  gtk_box_append(GTK_BOX(btn_row3), btn_return);

  // ========== ZONE PRINCIPALE DROITE ==========
  GtkWidget *main_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_margin_start(main_area, 10);
  gtk_widget_set_margin_end(main_area, 10);
  gtk_widget_set_margin_top(main_area, 10);
  gtk_widget_set_margin_bottom(main_area, 10);
  gtk_paned_set_end_child(GTK_PANED(paned), main_area);
  gtk_paned_set_shrink_end_child(GTK_PANED(paned), FALSE);

  // Zone de résultats
  GtkWidget *results_frame =
      gtk_frame_new("Résultats de la Comparaison des Tris :");
  gtk_box_append(GTK_BOX(main_area), results_frame);

  GtkWidget *results_scrolled = gtk_scrolled_window_new();
  gtk_widget_set_size_request(results_scrolled, -1, 120);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(results_scrolled),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_frame_set_child(GTK_FRAME(results_frame), results_scrolled);

  data->results_text_view = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(data->results_text_view), FALSE);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(data->results_text_view), TRUE);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(results_scrolled),
                                data->results_text_view);

  // Liste AVANT tri
  GtkWidget *label_before = gtk_label_new("Liste AVANT tri:");
  gtk_widget_set_halign(label_before, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(main_area), label_before);

  GtkWidget *scrolled_before = gtk_scrolled_window_new();
  gtk_widget_set_size_request(scrolled_before, -1, 150);
  gtk_widget_set_vexpand(scrolled_before, TRUE);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_before),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
  gtk_box_append(GTK_BOX(main_area), scrolled_before);

  data->drawing_area_before = gtk_drawing_area_new();
  gtk_widget_set_size_request(data->drawing_area_before, 2000, 250);
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(data->drawing_area_before),
                                 on_draw_before, data, NULL);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_before),
                                data->drawing_area_before);

  // GESTURE CLICK pour modification interactive
  GtkGesture *gesture = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),
                                GDK_BUTTON_PRIMARY);
  g_signal_connect(gesture, "pressed", G_CALLBACK(on_list_click), data);
  gtk_widget_add_controller(data->drawing_area_before,
                            GTK_EVENT_CONTROLLER(gesture));

  // Liste APRÈS tri
  GtkWidget *label_after = gtk_label_new("Liste APRÈS tri:");
  gtk_widget_set_halign(label_after, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(main_area), label_after);

  GtkWidget *scrolled_after = gtk_scrolled_window_new();
  gtk_widget_set_size_request(scrolled_after, -1, 150);
  gtk_widget_set_vexpand(scrolled_after, TRUE);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_after),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
  gtk_box_append(GTK_BOX(main_area), scrolled_after);

  data->drawing_area_after = gtk_drawing_area_new();
  gtk_widget_set_size_request(data->drawing_area_after, 2000, 250);
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(data->drawing_area_after),
                                 on_draw_after, data, NULL);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_after),
                                data->drawing_area_after);

  // Section OPÉRATIONS
  GtkWidget *operations_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_box_append(GTK_BOX(main_area), operations_box);

  GtkWidget *value_label = gtk_label_new("Valeur :");
  gtk_box_append(GTK_BOX(operations_box), value_label);

  data->value_entry = gtk_entry_new();
  gtk_widget_set_size_request(data->value_entry, 100, -1);
  gtk_entry_set_placeholder_text(GTK_ENTRY(data->value_entry), "00");
  gtk_box_append(GTK_BOX(operations_box), data->value_entry);

  GtkWidget *position_label = gtk_label_new("Position :");
  gtk_box_append(GTK_BOX(operations_box), position_label);

  const char *positions[] = {"Début", "Fin", "Indice", NULL};
  data->position_combo = gtk_drop_down_new_from_strings(positions);
  gtk_box_append(GTK_BOX(operations_box), data->position_combo);

  GtkWidget *index_label = gtk_label_new("Indice :");
  gtk_box_append(GTK_BOX(operations_box), index_label);

  data->index_entry = gtk_entry_new();
  gtk_widget_set_size_request(data->index_entry, 100, -1);
  gtk_entry_set_placeholder_text(GTK_ENTRY(data->index_entry), "Indice");
  gtk_box_append(GTK_BOX(operations_box), data->index_entry);

  // Boutons d'opérations COLORÉS
  GtkWidget *btn_insert = gtk_button_new_with_label("Insérer");
  gtk_widget_add_css_class(btn_insert, "suggested-action"); // VERT
  g_signal_connect(btn_insert, "clicked", G_CALLBACK(on_insert_operation),
                   data);
  gtk_box_append(GTK_BOX(operations_box), btn_insert);

  GtkWidget *btn_delete = gtk_button_new_with_label("Supprimer");
  gtk_widget_add_css_class(btn_delete, "destructive-action"); // ROUGE
  g_signal_connect(btn_delete, "clicked", G_CALLBACK(on_delete_operation),
                   data);
  gtk_box_append(GTK_BOX(operations_box), btn_delete);

  GtkWidget *btn_modify = gtk_button_new_with_label("Modifier");
  g_signal_connect(btn_modify, "clicked", G_CALLBACK(on_modify_operation),
                   data);
  gtk_box_append(GTK_BOX(operations_box), btn_modify);

  // Initialiser les données
  data->simple_list = NULL;
  data->simple_list_before = NULL;
  data->double_list = NULL;
  data->double_list_before = NULL;
  data->current_type = TYPE_INT;
  data->is_double = false;
  data->is_manual_mode = false;
  data->has_sorted = false;

  g_signal_connect(data->window, "destroy", G_CALLBACK(on_window_destroy),
                   data);

  gtk_window_present(GTK_WINDOW(data->window));
}
