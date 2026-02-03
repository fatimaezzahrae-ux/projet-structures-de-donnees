cmake_minimum_required(VERSION 3.10)
project(DataStructuresApp C)

# Définir le standard C
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Trouver le paquet GTK 4
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

# Inclure les répertoires d'en-tête GTK 4
include_directories(${GTK4_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS})

# Créer l'exécutable
add_executable(datastructures_app 
    main.c 
    arrays_window.c 
    sort_algorithms.c 
    curve_window.c 
    lists_window.c 
    list_algorithms.c 
    graph_algorithms.c 
    graphs_window.c 
    tree_algorithms.c 
    trees_window.c
)

# Lier les bibliothèques GTK 4
target_link_libraries(datastructures_app ${GTK4_LIBRARIES})

# Drapeaux de compilation (optionnel, pour éviter certains warnings)
add_compile_options(${GTK4_CFLAGS_OTHER})
