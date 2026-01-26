# Projet de Structures de Données (GTK4)

Ce projet est une application graphique développée en **langage C** avec la bibliothèque **GTK4**. Elle permet de visualiser et d'interagir avec différentes structures de données et algorithmes.

## 📂 Contenu du Dossier

Le projet est structuré comme suit :

### Fichiers Principaux
- **`main.c`** : Point d'entrée de l'application. Initialise la fenêtre principale et le menu.
- **`CMakeLists.txt`** : Fichier de configuration pour la compilation automatique avec CMake.
- **`style.css`** : Feuille de style CSS pour l'interface graphique GTK.

### Modules (Fenêtres et Algorithmes)
Chaque structure de données possède sa propre fenêtre et ses algorithmes associés :

1.  **Tableaux & Tris** (`arrays_window.c/h`, `sort_algorithms.c/h`)
    -   Visualisation des algorithmes de tri (Bulle, Insertion, Shell, Rapide).
    -   Analyse de complexité via **`curve_window.c/h`**.

2.  **Listes Chaînées** (`lists_window.c/h`, `list_algorithms.c/h`)
    -   Manipulation de listes (ajout, suppression, parcours).

3.  **Arbres Binaires** (`trees_window.c/h`, `tree_algorithms.c/h`)
    -   Visualisation d'arbres binaires de recherche (BST).

4.  **Graphes** (`graphs_window.c/h`, `graph_algorithms.c/h`)
    -   Algorithmes de graphes (Dijkstra, parcours, etc.).

---

## 🛠️ Prérequis

Pour compiler et exécuter ce projet, vous avez besoin de :
-   Un compilateur C (GCC via MinGW ou MSYS2 sur Windows).
-   La bibliothèque **GTK4** (incluant ses dépendances).
-   **CMake** (recommandé pour la compilation).
-   **Pkg-config**.

---

## 🚀 Installation et Compilation

Vous pouvez compiler le projet de deux manières : avec **CMake** (recommandé) ou manuellement avec **GCC**.

### Option 1 : Via CMake (Recommandé)
Cette méthode est plus simple et gère automatiquement les dépendances.

1.  Créez un dossier de build :
    ```bash
    mkdir build
    cd build
    ```
2.  Générez les fichiers de compilation et compilez :
    ```bash
    cmake ..
    cmake --build .
    ```
3.  L'exécutable `datastructures_app.exe` sera généré.

### Option 2 : Compilation Manuelle (GCC)
Si vous préférez compiler directement en ligne de commande :

```bash
gcc -g main.c arrays_window.c sort_algorithms.c curve_window.c lists_window.c list_algorithms.c graph_algorithms.c graphs_window.c tree_algorithms.c trees_window.c -o main.exe $(pkg-config --cflags --libs gtk4)
```

---

## ⚠️ Notes Importantes

-   **`style.css`** : Ce fichier doit impérativement se trouver dans le **même dossier** que l'exécutable (`main.exe`) pour que le style de l'application s'affiche correctement. Si vous compilez dans un dossier `build`, pensez à copier le fichier `style.css` à côté de l'exécutable.
