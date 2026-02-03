# Projet Structures de Données - Version C/GTK4

Application native haute performance pour la visualisation et la manipulation de structures de données fondamentales. Développée en C pur avec une interface graphique moderne basée sur GTK4.

## 🚀 Fonctionnalités Détaillées

### 1. 📊 Gestion et Tri de Tableaux (`arrays_window`)
Visualisation d'algorithmes de tri sur différents types de données.
- **Types de données supportés** : `Entier`, `Réel` (float), `Caractère`, `Chaîne de caractères`.
- **Algorithmes implémentés** :
  - **Tri à Bulle (Bubble Sort)** : Classique et simple.
  - **Tri par Insertion (Insertion Sort)** : Efficace pour les petits tableaux.
  - **Tri Shell (Shell Sort)** : Amélioration du tri par insertion.
  - **Tri Rapide (Quick Sort)** : L'un des plus rapides (Divide & Conquer).
- **Courbes de Performance** (`curve_window`) : Comparaison graphique des temps d'exécution des différents algorithmes.

### 2. 🔗 Listes Chaînées (`lists_window`)
Manipulation dynamique de listes avec visualisation des nœuds et pointeurs.
- **Structures** :
  - **Liste Simplement Chaînée** : `[Data | Next] -> ...`
  - **Liste Doublement Chaînée** : `... <- [Prev | Data | Next] -> ...`
- **Opérations** :
  - Insertion (Début, Fin, Position arbitraire).
  - Suppression (Par position).
  - Modification de valeur.
  - Recherche d'élément.
- **Algorithmes de Tri dédiés aux listes** : Bulle, Insertion, Sélection.

### 3. 🌳 Arbres (`trees_window`)
Visualisation hiérarchique avec rendu graphique précis.
- **Types d'Arbres** :
  - **BST (Binary Search Tree)** : Arbre binaire de recherche ordonné.
  - **Arbre N-aire** : Arbre générique où chaque nœud peut avoir N enfants.
- **Opérations** :
  - Insertion (Automatique selon l'ordre pour BST, avec choix du parent pour N-aire).
  - Suppression de nœuds.
  - Modification de valeurs.
- **Parcours** :
  - Profondeur : Préfixe (Pre-order), Infixe (In-order), Postfixe (Post-order).
  - Largeur (BFS - Breadth First Search).
- **Conversion** : Transformation automatique d'un Arbre N-aire en Arbre Binaire.

### 4. 🕸️ Graphes (`graphs_window`)
Manipulation complexe de graphes pondérés et visualisation des chemins.
- **Modèle** : Graphe orienté/non-orienté, pondéré/non-pondéré.
- **Représentation** : Matrice d'adjacence.
- **Algorithmes de Plus Court Chemin** :
  - **Dijkstra** : Pour graphes à poids positifs.
  - **Bellman-Ford** : Gère les poids négatifs.
  - **Floyd-Warshall** : Tous les chemins les plus courts (All-pairs).
- **Fonctionnalités** : Ajout/Suppression de sommets et d'arêtes, pondérations dynamiques.

## 🛠️ Architecture Technique

- **Langage** : C11 Standard.
- **GUI Toolkit** : GTK4 (GIMP Toolkit version 4).
- **Dessin** : Cairo Graphics (pour le rendu vectoriel des nœuds et arêtes).
- **Style** : CSS (GTK CSS Provider) pour le thème sombre/moderne (`style.css`).

## 💻 Compilation et Installation

### Prérequis Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential libgtk-4-dev pkg-config
```

### Prérequis Windows (MSYS2)
1. Installez MSYS2.
2. Ouvrez le terminal `MINGW64`.
3. Exécutez :
```bash
pacman -S mingw-w64-x86_64-gtk4 mingw-w64-x86_64-toolchain base-devel
```

### Compilation

#### Méthode 1 : GCC Direct
```bash
gcc -o app main.c arrays_window.c curve_window.c graphs_window.c lists_window.c trees_window.c sort_algorithms.c list_algorithms.c tree_algorithms.c graph_algorithms.c launcher.c `pkg-config --cflags --libs gtk4`
```

#### Méthode 2 : CMake (Recommandé)
```bash
mkdir build
cd build
cmake ..
make
```

## 🚀 Exécution

```bash
./app
```
L'interface principale (`launcher.c`) s'ouvrira pour vous permettre de choisir le module à lancer.

---
**Auteur** : Moustaoui Fatimaezzahrae
