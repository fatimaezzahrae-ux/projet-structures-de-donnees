# Projet Structures de Données - Version C/GTK4

Cette partie du dépôt contient l'implémentation native de l'application de visualisation de structures de données en utilisant le langage C et la bibliothèque graphique GTK4.

## 🚀 Fonctionnalités

- **Performances natives** grâce au C
- Interface graphique moderne avec **GTK4**
- Visualisation fluide avec **Cairo**
- Gestion complète des structures :
  - Listes chaînées
  - Arbres (BST, N-aires)
  - Graphes (Algorithmes de plus court chemin, etc.)

## 🛠️ Prérequis pour la compilation

### Sur Windows (MSYS2 / MinGW)

1. Installer MSYS2
2. Installer les paquets GTK4 :
   ```bash
   pacman -S mingw-w64-x86_64-gtk4 mingw-w64-x86_64-toolchain base-devel
   ```

### Sur Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install libgtk-4-dev build-essential
```

## 💻 Compilation et Exécution

Vous pouvez utiliser `cmake` ou compiler directement avec `gcc`.

### Avec GCC (Exemple simple)

```bash
gcc -o app main.c (autres fichiers .c) `pkg-config --cflags --libs gtk4`
./app
```

*(Note : Ajustez selon votre configuration de build exacte, voir CMakeLists.txt si disponible)*

## 📂 Structure des fichiers

- **main.c** : Point d'entrée de l'application
- **[structure]_window.c/h** : Gestion de l'interface pour chaque structure
- **[structure]_algorithms.c/h** : Implémentation des algorithmes
- **style.css** : Feuille de style GTK

---
Pour la version Python, voir le dossier `../python`.
