#include "arrays_window.h"
#include "graphs_window.h"
#include "lists_window.h"
#include "trees_window.h"
#include <gtk/gtk.h>

// Callbacks pour les boutons
static void on_tableaux_clicked(GtkWidget *widget, gpointer data) {
  // Récupérer la fenêtre parente (celle du bouton)
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_native(widget));
  open_arrays_window(window);
}

static void on_listes_clicked(GtkWidget *widget, gpointer data) {
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_native(widget));
  open_lists_window(window);
}

static void on_arbres_clicked(GtkWidget *widget, gpointer data) {
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_native(widget));
  open_trees_window(window);
}

static void on_graphes_clicked(GtkWidget *widget, gpointer data) {
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_native(widget));
  open_graphs_window(window);
}

static void on_activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *btn_tableaux, *btn_listes, *btn_arbres, *btn_graphes;
  GtkCssProvider *cssProvider;

  // Create application window
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Structures de Données");
  gtk_window_set_default_size(GTK_WINDOW(window), 600, 500);
  gtk_widget_add_css_class(window, "main-window");

  // CSS Provider
  cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(cssProvider, "style.css");
  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), GTK_STYLE_PROVIDER(cssProvider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  // Main Container (Vertical Box) - holds Title and Grid
  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 30);
  gtk_widget_set_margin_top(box, 30);
  gtk_widget_set_margin_bottom(box, 30);
  gtk_widget_set_margin_start(box, 30);
  gtk_widget_set_margin_end(box, 30);
  // Remove strict centering so children can expand
  // gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  // gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

  gtk_window_set_child(GTK_WINDOW(window), box);

  // Title Section
  GtkWidget *title_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  GtkWidget *title = gtk_label_new("Bienvenue");
  gtk_widget_add_css_class(title, "title-label");

  GtkWidget *subtitle =
      gtk_label_new("Choisissez une structure de données à explorer");
  gtk_widget_add_css_class(subtitle, "subtitle-label");

  // Center the title text horizontally
  gtk_widget_set_halign(title_box, GTK_ALIGN_CENTER);

  gtk_box_append(GTK_BOX(title_box), title);
  gtk_box_append(GTK_BOX(title_box), subtitle);
  gtk_box_append(GTK_BOX(box), title_box);

  // Grid for Buttons
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 30);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 30);

  // Make the grid fill the remaining space in the box
  gtk_widget_set_vexpand(grid, TRUE);
  gtk_widget_set_hexpand(grid, TRUE);
  gtk_widget_set_halign(grid, GTK_ALIGN_FILL);
  gtk_widget_set_valign(grid, GTK_ALIGN_FILL);

  // Ensure the grid columns/rows share space equally (homogeneous)
  gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

  // Buttons with Descriptions
  btn_tableaux = gtk_button_new_with_label("Tableaux 🔢");
  btn_listes = gtk_button_new_with_label("Listes ✅");
  btn_arbres = gtk_button_new_with_label("Arbres 🌳");
  btn_graphes = gtk_button_new_with_label("Graphes 📈");

  // Add classes
  gtk_widget_add_css_class(btn_tableaux, "menu-button");
  gtk_widget_add_css_class(btn_tableaux, "btn-tableaux");

  gtk_widget_add_css_class(btn_listes, "menu-button");
  gtk_widget_add_css_class(btn_listes, "btn-listes");

  gtk_widget_add_css_class(btn_arbres, "menu-button");
  gtk_widget_add_css_class(btn_arbres, "btn-arbres");

  gtk_widget_add_css_class(btn_graphes, "menu-button");
  gtk_widget_add_css_class(btn_graphes, "btn-graphes");

  // Make buttons expand to fill their grid cells
  gtk_widget_set_hexpand(btn_tableaux, TRUE);
  gtk_widget_set_vexpand(btn_tableaux, TRUE);

  gtk_widget_set_hexpand(btn_listes, TRUE);
  gtk_widget_set_vexpand(btn_listes, TRUE);

  gtk_widget_set_hexpand(btn_arbres, TRUE);
  gtk_widget_set_vexpand(btn_arbres, TRUE);

  gtk_widget_set_hexpand(btn_graphes, TRUE);
  gtk_widget_set_vexpand(btn_graphes, TRUE);

  // Sizing
  gtk_widget_set_size_request(btn_tableaux, 200, 120);
  gtk_widget_set_size_request(btn_listes, 200, 120);
  gtk_widget_set_size_request(btn_arbres, 200, 120);
  gtk_widget_set_size_request(btn_graphes, 200, 120);

  // Connection
  g_signal_connect(btn_tableaux, "clicked", G_CALLBACK(on_tableaux_clicked),
                   NULL);
  g_signal_connect(btn_listes, "clicked", G_CALLBACK(on_listes_clicked), NULL);
  g_signal_connect(btn_arbres, "clicked", G_CALLBACK(on_arbres_clicked), NULL);
  g_signal_connect(btn_graphes, "clicked", G_CALLBACK(on_graphes_clicked),
                   NULL);

  // Attach to Grid (0,0), (1,0), (0,1), (1,1)
  gtk_grid_attach(GTK_GRID(grid), btn_tableaux, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_listes, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_arbres, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_graphes, 1, 1, 1, 1);

  gtk_box_append(GTK_BOX(box), grid);

  // Display
  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("com.example.datastructures",
                            G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
