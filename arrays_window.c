#include "arrays_window.h"
#include "curve_window.h"
#include "sort_algorithms.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  GtkWidget *dropdown_type;
  GtkWidget *dropdown_mode;
  GtkWidget *dropdown_algo;
  GtkWidget *spin_size;

  // Champs de saisie manuelle
  GtkWidget *box_manual_input; // Conteneur pour cacher/montrer
  GtkWidget *entry_manual;

  // Zones de texte pour l'affichage
  GtkWidget *text_view_input;
  GtkWidget *text_view_output;

  GtkWidget *label_time;
  GtkWidget *window; // Reference to main window

  // Données actuelles
  ArrayData current_data;

  // Flag pour savoir si un tri a été effectué
  int has_sorted;
} ArraysAppWidgets;

static ArraysAppWidgets widgets_data = {0};

// --- Helpers ---

// Valide et parse une chaîne séparée par espaces en ArrayData
// Retourne 1 si valide, 0 si erreur (et remplit error_msg)
static int parse_manual_input_with_validation(const char *text, DataType type,
                                              char *error_msg,
                                              size_t error_size) {
  if (!text || strlen(text) == 0) {
    snprintf(error_msg, error_size, "Aucune valeur saisie.");
    return 0;
  }

  // Compter les tokens
  char *copy = strdup(text);
  char *token = strtok(copy, " \n\t");
  int count = 0;
  while (token) {
    count++;
    token = strtok(NULL, " \n\t");
  }
  free(copy);

  if (count == 0) {
    snprintf(error_msg, error_size, "Aucune valeur valide trouvée.");
    return 0;
  }

  // Allouer le tableau
  if (widgets_data.current_data.array)
    free(widgets_data.current_data.array);

  widgets_data.current_data.type = type;
  widgets_data.current_data.size = count;
  widgets_data.current_data.array = malloc(
      count * sizeof(double)); // Utiliser double pour tous les types numériques

  // Valider et parser
  copy = strdup(text);
  token = strtok(copy, " \n\t");
  int i = 0;

  while (token && i < count) {
    char *endptr;

    switch (type) {
    case TYPE_INT: {
      long val = strtol(token, &endptr, 10);
      if (*endptr != '\0') {
        snprintf(error_msg, error_size,
                 "Erreur: '%s' n'est pas un entier valide.", token);
        free(copy);
        return 0;
      }
      ((int *)widgets_data.current_data.array)[i] = (int)val;
      break;
    }
    case TYPE_FLOAT: {
      double val = strtod(token, &endptr);
      if (*endptr != '\0') {
        snprintf(error_msg, error_size,
                 "Erreur: '%s' n'est pas un nombre réel valide.", token);
        free(copy);
        return 0;
      }
      ((double *)widgets_data.current_data.array)[i] = val;
      break;
    }
    case TYPE_CHAR: {
      if (strlen(token) != 1) {
        snprintf(error_msg, error_size,
                 "Erreur: '%s' n'est pas un caractère unique.", token);
        free(copy);
        return 0;
      }
      if (isdigit(token[0])) {
        snprintf(error_msg, error_size,
                 "Erreur: '%s' est un nombre, pas un caractère.", token);
        free(copy);
        return 0;
      }
      ((char *)widgets_data.current_data.array)[i] = token[0];
      break;
    }
    case TYPE_STRING: {
      // Vérifier si c'est un nombre
      double val = strtod(token, &endptr);
      (void)val; // Supprimer warning unused
      if (*endptr == '\0') {
        snprintf(error_msg, error_size,
                 "Erreur: '%s' est un nombre, pas une chaîne.", token);
        free(copy);
        return 0;
      }
      // Pour les strings, on stocke des pointeurs (simplifié ici)
      ((char **)widgets_data.current_data.array)[i] = strdup(token);
      break;
    }
    }
    i++;
    token = strtok(NULL, " \n\t");
  }

  widgets_data.current_data.size = i;
  free(copy);
  return 1;
}

// Ancienne fonction pour compatibilité
static void parse_manual_input(const char *text) {
  char error_msg[256];
  parse_manual_input_with_validation(text, TYPE_INT, error_msg,
                                     sizeof(error_msg));
}

static int prepare_data() {
  int size =
      gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets_data.spin_size));
  int type_idx =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(widgets_data.dropdown_type));
  int mode_idx = gtk_drop_down_get_selected(
      GTK_DROP_DOWN(widgets_data.dropdown_mode)); // 0=Aleatoire, 1=Manuel

  // Récupérer la string du mode pour être sûr (ou utiliser l'index 0/1)
  // Ici on utilise l'index : 0 = Aléatoire, 1 = Manuel

  if (mode_idx == 0) { // Aléatoire
    generate_random_data(&widgets_data.current_data, size, (DataType)type_idx);
  } else { // Manuel
    GtkEntryBuffer *buf =
        gtk_entry_get_buffer(GTK_ENTRY(widgets_data.entry_manual));
    const char *text = gtk_entry_buffer_get_text(buf);

    char error_msg[256] = "";
    if (!parse_manual_input_with_validation(text, (DataType)type_idx, error_msg,
                                            sizeof(error_msg))) {
      // Clear displays on error
      GtkTextBuffer *empty_buf = gtk_text_buffer_new(NULL);
      gtk_text_view_set_buffer(GTK_TEXT_VIEW(widgets_data.text_view_input),
                               empty_buf);
      g_object_unref(empty_buf);

      empty_buf = gtk_text_buffer_new(NULL);
      gtk_text_view_set_buffer(GTK_TEXT_VIEW(widgets_data.text_view_output),
                               empty_buf);
      g_object_unref(empty_buf);

      // Clear internal data
      if (widgets_data.current_data.array) {
        free(widgets_data.current_data.array);
        widgets_data.current_data.array = NULL;
      }
      widgets_data.current_data.size = 0;
      widgets_data.has_sorted = 0;
      gtk_label_set_text(GTK_LABEL(widgets_data.label_time), "Temps: -");

      // Afficher message d'erreur
      GtkWidget *dialog = gtk_message_dialog_new(
          GTK_WINDOW(widgets_data.window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
          GTK_BUTTONS_OK, "%s", error_msg);
      gtk_window_present(GTK_WINDOW(dialog));
      g_signal_connect_swapped(dialog, "response",
                               G_CALLBACK(gtk_window_destroy), dialog);
      return 0;
    }
  }

  // Afficher input
  char *str = array_to_string(&widgets_data.current_data);
  char *full_text = g_strdup_printf("Tableau avant tri :\n\n%s", str);
  GtkTextBuffer *buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets_data.text_view_input));
  gtk_text_buffer_set_text(buffer, full_text, -1);
  g_free(full_text);
  free(str);
  return 1;
}

// Handler pour changement de mode (montrer/cacher champ manuel)
static void on_mode_changed(GtkDropDown *dropdown, GParamSpec *pspec,
                            gpointer data) {
  int selected = gtk_drop_down_get_selected(dropdown);
  if (selected == 1) { // Manuel
    gtk_widget_set_visible(widgets_data.box_manual_input, TRUE);
    gtk_widget_set_visible(
        widgets_data.spin_size,
        FALSE); // Cacher taille car déterminé par input ? Ou garder ?
    // On garde taille visible car l'user peut vouloir dire "Taille 10" et
    // entrer 10 nombres? Le user a dit " Manuel ... il entre les valeurs. Aussi
    // une partie taille". On laisse les deux visibles.
  } else {
    gtk_widget_set_visible(widgets_data.box_manual_input, FALSE);
    gtk_widget_set_visible(widgets_data.spin_size, TRUE);
  }
}

// --- Callbacks Boutons ---

static void on_trier_clicked(GtkWidget *btn, gpointer data) {
  if (!prepare_data())
    return; // Stop si erreur

  if (widgets_data.current_data.array == NULL)
    return;

  int algo_idx =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(widgets_data.dropdown_algo));

  // Avertissement pour les tris lents sur grands tableaux
  if (widgets_data.current_data.size > 20000 &&
      (algo_idx == 0 || algo_idx == 1)) {
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(widgets_data.window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "Erreur: Ce tri (O(N²)) est trop lent pour %zu éléments.\n"
        "Veuillez choisir 'Tri Shell' ou 'QuickSort' pour éviter de geler "
        "l'interface.",
        widgets_data.current_data.size);

    gtk_widget_show(dialog);
    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_window_destroy),
                             dialog);
    return; // On arrête tout
  }

  // Copie pour tri
  ArrayData data_to_sort;
  data_to_sort.size = widgets_data.current_data.size;
  data_to_sort.type = widgets_data.current_data.type;
  data_to_sort.array =
      malloc(data_to_sort.size * sizeof(int)); // TODO: Fix type generic

  // Correction pour malloc selon type
  size_t elem_size = sizeof(int);
  if (data_to_sort.type == TYPE_FLOAT)
    elem_size = sizeof(float);
  else if (data_to_sort.type == TYPE_CHAR)
    elem_size = sizeof(char);

  free(data_to_sort.array); // Free le malloc int par defaut
  data_to_sort.array = malloc(data_to_sort.size * elem_size);

  memcpy(data_to_sort.array, widgets_data.current_data.array,
         data_to_sort.size * elem_size);

  double time_taken = sort_array(&data_to_sort, (SortAlgo)algo_idx);

  // Affichage
  char *str = array_to_string(&data_to_sort);
  char *full_text = g_strdup_printf("Tableau après tri :\n\n%s", str);
  GtkTextBuffer *buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets_data.text_view_output));
  gtk_text_buffer_set_text(buffer, full_text, -1);
  g_free(full_text);
  free(str);

  char time_str[64];
  snprintf(time_str, 64, "Temps: %.3f ms", time_taken * 1000.0);
  gtk_label_set_text(GTK_LABEL(widgets_data.label_time), time_str);

  // Marquer qu'un tri a été effectué
  widgets_data.has_sorted = 1;

  free_array_data(&data_to_sort);
}

static void on_compare_clicked(GtkWidget *btn, gpointer data) {
  if (!prepare_data())
    return; // Stop si erreur

  if (widgets_data.current_data.array == NULL)
    return;

  char result_buffer[2048] = "Comparaison des Méthodes :\n\n";
  const char *algo_names[] = {"Bulle", "Insertion", "Shell", "QuickSort"};

  for (int i = 0; i < 4; i++) {
    // Skip slow sorts for large data
    if (widgets_data.current_data.size > 20000 && (i == 0 || i == 1)) {
      char line[128];
      snprintf(line, 128, "%s : Skippé (>20k éléments)\n", algo_names[i]);
      strcat(result_buffer, line);
      continue;
    }

    // Copie fraiche
    ArrayData temp;
    temp.type = widgets_data.current_data.type;
    temp.size = widgets_data.current_data.size;

    size_t elem_size = sizeof(int);
    if (temp.type == TYPE_FLOAT)
      elem_size = sizeof(float);
    else if (temp.type == TYPE_CHAR)
      elem_size = sizeof(char);

    temp.array = malloc(temp.size * elem_size);
    memcpy(temp.array, widgets_data.current_data.array, temp.size * elem_size);

    double t = sort_array(&temp, (SortAlgo)i);

    char line[128];
    snprintf(line, 128, "%s : %.3f ms\n", algo_names[i], t * 1000.0);
    strcat(result_buffer, line);

    free_array_data(&temp);
  }

  // Afficher dans Output View
  GtkTextBuffer *buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets_data.text_view_output));
  gtk_text_buffer_set_text(buffer, result_buffer, -1);

  gtk_label_set_text(GTK_LABEL(widgets_data.label_time),
                     "Comparaison terminée.");

  // Marquer qu'un tri a été effectué
  widgets_data.has_sorted = 1;
}

static void save_results_to_file(const char *filename) {
  FILE *f = fopen(filename, "w");
  if (!f) {
    // Should show error dialog here ideally
    return;
  }

  fprintf(f, "Rapport de Tri\n");
  fprintf(f, "--------------\n");

  // Input
  char *in_str = array_to_string(&widgets_data.current_data);
  fprintf(f, "Donnees Originales: %s\n", in_str);
  free(in_str);

  fprintf(f, "\n(Voir l'interface pour les résultats détaillés lors de la "
             "comparaison)\n");

  fclose(f);

  char msg[256];
  snprintf(msg, sizeof(msg), "Sauvegardé dans %s", filename);
  gtk_label_set_text(GTK_LABEL(widgets_data.label_time), msg);
}

static void on_save_response(GtkNativeDialog *dialog, int response,
                             gpointer user_data) {
  if (response == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    GFile *file = gtk_file_chooser_get_file(chooser);
#pragma GCC diagnostic pop
    char *filename = g_file_get_path(file);

    save_results_to_file(filename);

    g_free(filename);
    g_object_unref(file);
  }
  g_object_unref(dialog);
}

static void on_save_clicked(GtkWidget *btn, gpointer data) {
  GtkFileChooserNative *native;
  GtkWindow *parent = GTK_WINDOW(gtk_widget_get_native(btn));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  native = gtk_file_chooser_native_new("Enregistrer sous", parent,
                                       GTK_FILE_CHOOSER_ACTION_SAVE,
                                       "_Enregistrer", "_Annuler");
#pragma GCC diagnostic pop

  GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  gtk_file_chooser_set_current_name(chooser, "resultats_tri.txt");
#pragma GCC diagnostic pop

  g_signal_connect(native, "response", G_CALLBACK(on_save_response), NULL);
  gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
}

static void on_reset_clicked(GtkWidget *btn, gpointer data) {
  // 1. Vider les données
  if (widgets_data.current_data.array) {
    free(widgets_data.current_data.array);
    widgets_data.current_data.array = NULL;
    widgets_data.current_data.size = 0;
  }

  // 2. Vider les affichages
  GtkTextBuffer *empty_buf = gtk_text_buffer_new(NULL);
  gtk_text_view_set_buffer(GTK_TEXT_VIEW(widgets_data.text_view_input),
                           empty_buf);
  g_object_unref(empty_buf);

  empty_buf = gtk_text_buffer_new(NULL);
  gtk_text_view_set_buffer(GTK_TEXT_VIEW(widgets_data.text_view_output),
                           empty_buf);
  g_object_unref(empty_buf);

  // 3. Reset labels et flags
  gtk_label_set_text(GTK_LABEL(widgets_data.label_time), "Temps: -");
  widgets_data.has_sorted = 0;

  // 4. Vider le champ de saisie manuel
  gtk_editable_set_text(GTK_EDITABLE(widgets_data.entry_manual), "");
}

static void on_curve_clicked(GtkWidget *btn, gpointer data) {
  // Vérifier si un tri a été effectué
  if (!widgets_data.has_sorted) {
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(widgets_data.window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING,
        GTK_BUTTONS_OK,
        "Veuillez d'abord effectuer un tri ou une comparaison avant de générer "
        "la courbe.");
    gtk_window_present(GTK_WINDOW(dialog));
    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_window_destroy),
                             dialog);
    return;
  }

  // Determine relevant size to use
  int size = 1000; // Default
  int mode_idx =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(widgets_data.dropdown_mode));

  if (mode_idx == 0) { // Random
    size = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(widgets_data.spin_size));
  } else { // Manual
    size = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(widgets_data.spin_size));
  }

  // Cap min size to avoid issues with graph
  if (size < 10)
    size = 10;

  open_curve_window(GTK_WINDOW(gtk_widget_get_native(btn)), size);
}

// helper widget
static GtkWidget *create_labeled_widget(const char *label_text,
                                        GtkWidget *child) {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  GtkWidget *lbl = gtk_label_new(label_text);
  gtk_widget_set_halign(lbl, GTK_ALIGN_START);
  gtk_widget_add_css_class(lbl, "header-label");
  gtk_box_append(GTK_BOX(box), lbl);
  gtk_box_append(GTK_BOX(box), child);
  return box;
}

void open_arrays_window(GtkWindow *parent) {
  GtkWidget *window, *box_main;
  GtkWidget *left_scrolled, *right_scrolled;
  GtkWidget *left_panel, *right_panel;

  // Réinitialiser le flag
  widgets_data.has_sorted = 0;

  window = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(window), "Programme de Tri - Tableaux");
  gtk_window_set_default_size(GTK_WINDOW(window), 900,
                              600); // Mieux adapté au layout horizontal
  gtk_widget_add_css_class(window, "main-window");
  // gtk_window_set_transient_for(GTK_WINDOW(window), parent);
  gtk_window_set_modal(GTK_WINDOW(window),
                       FALSE); // Non-modal pour permettre plus de flexibilité
  gtk_window_set_resizable(GTK_WINDOW(window),
                           TRUE); // Autoriser le redimensionnement
  gtk_window_maximize(GTK_WINDOW(window));

  // Stocker la référence à la fenêtre
  widgets_data.window = window;

  // Main container (Horizontal Box)
  box_main = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_window_set_child(GTK_WINDOW(window), box_main);

  // ===== LEFT PANEL (Configuration) =====
  left_scrolled = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(left_scrolled),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(left_scrolled, 300, -1); // Fixed width
  gtk_box_append(GTK_BOX(box_main), left_scrolled);

  left_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
  gtk_widget_set_margin_top(left_panel, 20);
  gtk_widget_set_margin_bottom(left_panel, 20);
  gtk_widget_set_margin_start(left_panel, 20);
  gtk_widget_set_margin_end(left_panel, 20);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(left_scrolled), left_panel);

  GtkWidget *title_conf = gtk_label_new("Configuration");
  gtk_widget_add_css_class(title_conf, "header-label");
  gtk_box_append(GTK_BOX(left_panel), title_conf);

  // 1. Type
  const char *types[] = {"Entier", "Réel", "Caractère", "Chaîne", NULL};
  widgets_data.dropdown_type = gtk_drop_down_new_from_strings(types);
  gtk_box_append(
      GTK_BOX(left_panel),
      create_labeled_widget("Type de tableau :", widgets_data.dropdown_type));

  // 2. Mode
  const char *modes[] = {"Aléatoire", "Manuel", NULL};
  widgets_data.dropdown_mode = gtk_drop_down_new_from_strings(modes);
  g_signal_connect(widgets_data.dropdown_mode, "notify::selected",
                   G_CALLBACK(on_mode_changed), NULL);
  gtk_box_append(GTK_BOX(left_panel),
                 create_labeled_widget("Mode de remplissage :",
                                       widgets_data.dropdown_mode));

  // 2b. Zone Manuelle (cachée par défaut)
  widgets_data.box_manual_input = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  widgets_data.entry_manual = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(widgets_data.entry_manual),
                                 "Ex: 10 5 3 8");
  gtk_box_append(
      GTK_BOX(widgets_data.box_manual_input),
      create_labeled_widget("Saisie manuelle (séparée par espaces) :",
                            widgets_data.entry_manual));
  gtk_box_append(GTK_BOX(left_panel), widgets_data.box_manual_input);
  gtk_widget_set_visible(widgets_data.box_manual_input,
                         FALSE); // Caché car défaut = Aléatoire

  // 3. Taille
  widgets_data.spin_size = gtk_spin_button_new_with_range(0, 1000000, 1);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets_data.spin_size), 0);
  gtk_box_append(
      GTK_BOX(left_panel),
      create_labeled_widget("Taille du tableau :", widgets_data.spin_size));

  // 4. Algo
  const char *algos[] = {"Tri à bulle", "Tri par insertion", "Tri Shell",
                         "QuickSort", NULL};
  widgets_data.dropdown_algo = gtk_drop_down_new_from_strings(algos);
  gtk_box_append(
      GTK_BOX(left_panel),
      create_labeled_widget("Méthode de tri :", widgets_data.dropdown_algo));

  gtk_box_append(GTK_BOX(left_panel),
                 gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

  // 5. Boutons
  GtkWidget *btn_trier = gtk_button_new_with_label("▶ Trier");
  gtk_widget_add_css_class(btn_trier, "success-btn");
  g_signal_connect(btn_trier, "clicked", G_CALLBACK(on_trier_clicked), NULL);
  gtk_box_append(GTK_BOX(left_panel), btn_trier);

  GtkWidget *btn_compare = gtk_button_new_with_label("⚖ Comparer méthodes");
  gtk_widget_add_css_class(btn_compare, "info-btn");
  g_signal_connect(btn_compare, "clicked", G_CALLBACK(on_compare_clicked),
                   NULL);
  gtk_box_append(GTK_BOX(left_panel), btn_compare);

  GtkWidget *btn_save = gtk_button_new_with_label("💾 Enregistrer résultats");
  gtk_widget_add_css_class(btn_save, "save-btn");
  g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save_clicked), NULL);
  gtk_box_append(GTK_BOX(left_panel), btn_save);

  GtkWidget *btn_curve = gtk_button_new_with_label("📈 Générer courbe");
  gtk_widget_add_css_class(btn_curve, "accent-btn");
  g_signal_connect(btn_curve, "clicked", G_CALLBACK(on_curve_clicked), NULL);
  gtk_box_append(GTK_BOX(left_panel), btn_curve);

  GtkWidget *btn_reset = gtk_button_new_with_label("🔄 Réinitialiser");
  g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_reset_clicked), NULL);
  gtk_box_append(GTK_BOX(left_panel), btn_reset);

  // Espacement
  gtk_box_append(GTK_BOX(left_panel),
                 gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

  // Bouton Retour
  GtkWidget *btn_back = gtk_button_new_with_label("🔙 Retour Menu");
  g_signal_connect_swapped(btn_back, "clicked", G_CALLBACK(gtk_window_close),
                           window);
  gtk_box_append(GTK_BOX(left_panel), btn_back);

  // ===== RIGHT PANEL (Results) =====
  right_scrolled = gtk_scrolled_window_new();
  gtk_widget_set_hexpand(right_scrolled, TRUE);
  gtk_box_append(GTK_BOX(box_main), right_scrolled);

  right_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
  gtk_widget_set_margin_top(right_panel, 20);
  gtk_widget_set_margin_bottom(right_panel, 20);
  gtk_widget_set_margin_start(right_panel, 20);
  gtk_widget_set_margin_end(right_panel, 20);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(right_scrolled),
                                right_panel);

  GtkWidget *title_res = gtk_label_new("Résultats");
  gtk_widget_add_css_class(title_res, "header-label");
  gtk_box_append(GTK_BOX(right_panel), title_res);

  widgets_data.label_time = gtk_label_new("Temps: -");
  gtk_widget_add_css_class(widgets_data.label_time, "result-label");
  gtk_box_append(GTK_BOX(right_panel), widgets_data.label_time);

  // Paned split for Before/After
  GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  gtk_widget_set_vexpand(paned, TRUE);
  gtk_widget_set_hexpand(paned, TRUE);         // Ensure it fills width
  gtk_widget_set_size_request(paned, -1, 400); // Min height
  gtk_box_append(GTK_BOX(right_panel), paned);

  // Top Area: Input
  GtkWidget *vbox_input = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_append(GTK_BOX(vbox_input), gtk_label_new("Données Initiales:"));

  widgets_data.text_view_input = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(widgets_data.text_view_input),
                             FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widgets_data.text_view_input),
                              GTK_WRAP_WORD_CHAR);
  GtkWidget *s1 = gtk_scrolled_window_new();
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(s1),
                                widgets_data.text_view_input);
  gtk_widget_set_vexpand(s1, TRUE);
  gtk_widget_set_hexpand(s1, TRUE);
  gtk_box_append(GTK_BOX(vbox_input), s1);

  gtk_paned_set_start_child(GTK_PANED(paned), vbox_input);
  gtk_paned_set_resize_start_child(GTK_PANED(paned), TRUE);

  // Bottom Area: Output
  GtkWidget *vbox_output = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_append(GTK_BOX(vbox_output), gtk_label_new("Résultat du Tri:"));

  widgets_data.text_view_output = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(widgets_data.text_view_output),
                             FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widgets_data.text_view_output),
                              GTK_WRAP_WORD_CHAR);
  GtkWidget *s2 = gtk_scrolled_window_new();
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(s2),
                                widgets_data.text_view_output);
  gtk_widget_set_vexpand(s2, TRUE);
  gtk_widget_set_hexpand(s2, TRUE);
  gtk_box_append(GTK_BOX(vbox_output), s2);

  gtk_paned_set_end_child(GTK_PANED(paned), vbox_output);
  gtk_paned_set_resize_end_child(GTK_PANED(paned), TRUE);

  gtk_window_present(GTK_WINDOW(window));
}
