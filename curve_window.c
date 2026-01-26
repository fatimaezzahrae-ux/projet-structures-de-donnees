#include "curve_window.h"
#include "sort_algorithms.h"
#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Data for Plotting ---
#define NUM_POINTS 10
int TEST_SIZES[NUM_POINTS];
double BENCH_RESULTS[4][NUM_POINTS]; // 4 algos
char CHART_TITLE[128] = "Comparaison des Algorithmes de Tri (Type: Entier)";

const char *ALGO_NAMES[] = {"Tri à bulle", "Tri par insertion", "Tri Shell",
                            "Tri rapide"};

// --- Benchmarking ---
#define SLOW_ALGO_THRESHOLD 20000

static void run_benchmarks(int max_size) {
  if (max_size < NUM_POINTS)
    max_size = NUM_POINTS;

  for (int i = 0; i < NUM_POINTS; i++) {
    TEST_SIZES[i] = (max_size / NUM_POINTS) * (i + 1);
  }
  TEST_SIZES[NUM_POINTS - 1] = max_size;

  for (int s = 0; s < NUM_POINTS; s++) {
    int size = TEST_SIZES[s];

    ArrayData base = {0};
    generate_random_data(&base, size, TYPE_INT);

    for (int algo = 0; algo < 4; algo++) {
      if (size > SLOW_ALGO_THRESHOLD && (algo == 0 || algo == 1)) {
        BENCH_RESULTS[algo][s] = 0.0;
        continue;
      }

      ArrayData copy;
      copy.size = size;
      copy.type = TYPE_INT;
      copy.array = malloc(size * sizeof(int));
      memcpy(copy.array, base.array, size * sizeof(int));

      double t = sort_array(&copy, (SortAlgo)algo);
      BENCH_RESULTS[algo][s] = t;

      free_array_data(&copy);
    }
    free_array_data(&base);
  }
}

// --- Modern Drawing ---
static void on_draw(GtkDrawingArea *area, cairo_t *cr, int width, int height,
                    gpointer data) {
  // 1. White Background
  cairo_set_source_rgb(cr, 0.98, 0.98, 0.98);
  cairo_paint(cr);

  // Margins
  int margin_left = 80;
  int margin_right = 30;
  int margin_top = 60;
  int margin_bottom = 70;
  int graph_w = width - margin_left - margin_right;
  int graph_h = height - margin_top - margin_bottom;

  // 2. Title
  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 16);
  cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);

  cairo_text_extents_t title_extents;
  cairo_text_extents(cr, CHART_TITLE, &title_extents);
  cairo_move_to(cr, (width - title_extents.width) / 2, 30);
  cairo_show_text(cr, CHART_TITLE);

  // Find Max Time for scaling
  double max_time = 0;
  for (int a = 0; a < 4; a++) {
    for (int s = 0; s < NUM_POINTS; s++) {
      if (BENCH_RESULTS[a][s] > max_time)
        max_time = BENCH_RESULTS[a][s];
    }
  }
  if (max_time <= 0)
    max_time = 0.001;

  // 3. Grid Lines (horizontal)
  cairo_set_source_rgb(cr, 0.85, 0.85, 0.85);
  cairo_set_line_width(cr, 1);
  int num_grid_lines = 5;
  for (int i = 0; i <= num_grid_lines; i++) {
    double y = margin_top + (double)i / num_grid_lines * graph_h;
    cairo_move_to(cr, margin_left, y);
    cairo_line_to(cr, width - margin_right, y);
    cairo_stroke(cr);
  }

  // 4. Axes
  cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
  cairo_set_line_width(cr, 1.5);

  // Y Axis
  cairo_move_to(cr, margin_left, margin_top);
  cairo_line_to(cr, margin_left, height - margin_bottom);
  cairo_stroke(cr);

  // X Axis
  cairo_move_to(cr, margin_left, height - margin_bottom);
  cairo_line_to(cr, width - margin_right, height - margin_bottom);
  cairo_stroke(cr);

  // 5. Y-Axis Labels (Time values)
  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cr, 11);
  cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);

  for (int i = 0; i <= num_grid_lines; i++) {
    double value = max_time * (1.0 - (double)i / num_grid_lines);
    double y = margin_top + (double)i / num_grid_lines * graph_h;

    char num[32];
    if (max_time < 1.0) {
      snprintf(num, sizeof(num), "%.0f",
               value * 1000); // Convert to ms for display
    } else {
      snprintf(num, sizeof(num), "%.1f", value);
    }

    cairo_text_extents_t extents;
    cairo_text_extents(cr, num, &extents);
    cairo_move_to(cr, margin_left - extents.width - 10, y + extents.height / 2);
    cairo_show_text(cr, num);
  }

  // 6. X-Axis Labels (Sizes)
  for (int i = 0; i < NUM_POINTS; i++) {
    double x = margin_left + (double)i / (NUM_POINTS - 1) * graph_w;
    char num[16];
    snprintf(num, sizeof(num), "%d", TEST_SIZES[i]);

    cairo_text_extents_t extents;
    cairo_text_extents(cr, num, &extents);
    cairo_move_to(cr, x - (extents.width / 2), height - margin_bottom + 20);
    cairo_show_text(cr, num);
  }

  // 7. Axis Titles
  cairo_set_font_size(cr, 12);
  cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);

  // X-Axis Title
  const char *xlabel = "Taille du tableau (N)";
  cairo_text_extents_t extents;
  cairo_text_extents(cr, xlabel, &extents);
  cairo_move_to(cr, margin_left + (graph_w - extents.width) / 2, height - 15);
  cairo_show_text(cr, xlabel);

  // Y-Axis Title (rotated)
  const char *ylabel = "Temps d'exécution (secondes / s)";
  cairo_save(cr);
  cairo_translate(cr, 20, margin_top + graph_h / 2);
  cairo_rotate(cr, -M_PI / 2);
  cairo_text_extents(cr, ylabel, &extents);
  cairo_move_to(cr, -extents.width / 2, 0);
  cairo_show_text(cr, ylabel);
  cairo_restore(cr);

  // 8. Plot Lines with Markers
  // Colors: Red (Bubble), Orange (Insertion), Green (Shell), Blue (Quick)
  double colors[4][3] = {
      {0.9, 0.2, 0.2},  // Rouge - Bubble
      {0.95, 0.6, 0.1}, // Orange - Insertion
      {0.2, 0.7, 0.3},  // Vert - Shell
      {0.2, 0.4, 0.9}   // Bleu - Quick
  };

  cairo_set_line_width(cr, 2.5);

  for (int algo = 0; algo < 4; algo++) {
    cairo_set_source_rgb(cr, colors[algo][0], colors[algo][1], colors[algo][2]);

    // Draw lines
    int first_valid = 1;
    for (int s = 0; s < NUM_POINTS; s++) {
      if (BENCH_RESULTS[algo][s] <= 0 && TEST_SIZES[s] > SLOW_ALGO_THRESHOLD)
        continue; // Skip dropped points

      double x = margin_left + (double)s / (NUM_POINTS - 1) * graph_w;
      double y =
          margin_top + graph_h - (BENCH_RESULTS[algo][s] / max_time * graph_h);

      if (first_valid) {
        cairo_move_to(cr, x, y);
        first_valid = 0;
      } else {
        cairo_line_to(cr, x, y);
      }
    }
    cairo_stroke(cr);

    // Draw markers (circles)
    for (int s = 0; s < NUM_POINTS; s++) {
      if (BENCH_RESULTS[algo][s] <= 0 && TEST_SIZES[s] > SLOW_ALGO_THRESHOLD)
        continue;

      double x = margin_left + (double)s / (NUM_POINTS - 1) * graph_w;
      double y =
          margin_top + graph_h - (BENCH_RESULTS[algo][s] / max_time * graph_h);

      // Filled circle
      cairo_arc(cr, x, y, 5, 0, 2 * M_PI);
      cairo_fill(cr);
    }
  }

  // 9. Legend (top left, inside graph area)
  int lx = margin_left + 15;
  int ly = margin_top + 10;

  cairo_set_font_size(cr, 11);

  for (int algo = 0; algo < 4; algo++) {
    // Draw colored circle
    cairo_set_source_rgb(cr, colors[algo][0], colors[algo][1], colors[algo][2]);
    cairo_arc(cr, lx + 6, ly + (algo * 22) + 6, 5, 0, 2 * M_PI);
    cairo_fill(cr);

    // Draw text
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_move_to(cr, lx + 18, ly + (algo * 22) + 11);
    cairo_show_text(cr, ALGO_NAMES[algo]);
  }
}

static void on_close_clicked(GtkWidget *button, gpointer user_data) {
  GtkWidget *window = GTK_WIDGET(user_data);
  gtk_window_close(GTK_WINDOW(window));
}

void open_curve_window(GtkWindow *parent, int max_size) {
  // 0. Run Benchmarks
  run_benchmarks(max_size);

  // 1. Create Window
  GtkWidget *window = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(window), "Courbe de Performance");
  gtk_window_set_default_size(GTK_WINDOW(window), 900, 650);
  gtk_window_set_transient_for(GTK_WINDOW(window), parent);
  gtk_window_set_modal(GTK_WINDOW(window), FALSE);
  gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

  // Main container
  GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_window_set_child(GTK_WINDOW(window), main_box);

  // 2. Drawing Area
  GtkWidget *area = gtk_drawing_area_new();
  gtk_widget_set_vexpand(area, TRUE);
  gtk_widget_set_hexpand(area, TRUE);
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(area), on_draw, NULL, NULL);
  gtk_box_append(GTK_BOX(main_box), area);

  // 3. Bottom bar with Close button
  GtkWidget *bottom_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_margin_top(bottom_bar, 10);
  gtk_widget_set_margin_bottom(bottom_bar, 15);
  gtk_widget_set_margin_start(bottom_bar, 20);
  gtk_widget_set_margin_end(bottom_bar, 20);
  gtk_widget_set_halign(bottom_bar, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(main_box), bottom_bar);

  GtkWidget *btn_close = gtk_button_new_with_label("Fermer");
  gtk_widget_set_size_request(btn_close, 150, 40);
  gtk_widget_add_css_class(btn_close, "suggested-action");
  g_signal_connect(btn_close, "clicked", G_CALLBACK(on_close_clicked), window);
  gtk_box_append(GTK_BOX(bottom_bar), btn_close);

  gtk_window_present(GTK_WINDOW(window));
}
