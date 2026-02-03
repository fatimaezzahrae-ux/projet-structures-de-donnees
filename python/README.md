# Projet Structures de Données - Version Python

Application graphique interactive et moderne pour visualiser et manipuler les structures de données. Cette version mise sur la flexibilité de Python et la richesse des bibliothèques de visualisation.

## 🚀 Fonctionnalités Détaillées

### 1. 📋 Gestion de Listes
Manipulation de listes chaînées avec une interface intuitive.
- **Types de données** : Entier, Réel, Caractère, Chaîne.
- **Structures** : Liste Simple, Liste Doublement Chaînée.
- **Fonctionnalités** :
  - Insertion (Début, Fin, Indice).
  - Suppression et Modification dynamiques.
  - **Comparaison de Tris** : Visualisation des temps d'exécution pour :
    - Bubble Sort
    - Insertion Sort
    - Quick Sort
    - Shell Sort

### 2. 🌳 Gestion d'Arbres
Visualisation puissante grâce à `NetworkX` et `Matplotlib`.
- **Types** :
  - **BST (Arbre Binaire de Recherche)** : Insertion et équilibrage visuel.
  - **Arbre N-aire** : Arbres généraux avec N enfants par nœud.
- **Parcours Visuels** :
  - Profondeur : Pré-ordre, In-ordre, Post-ordre.
  - Largeur (BFS).
- **Fonctionnalités avancées** :
  - Génération aléatoire d'arbres.
  - **Conversion** automatique N-aire vers Binaire.
  - **Export JSON** : Sauvegardez vos arbres pour les réutiliser.

### 3. 🕸️ Gestion de Graphes
Outil complet pour l'analyse de graphes.
- **Modélisation** : Graphes Orientés/Non-orientés, Pondérés.
- **Algorithmes visuels** :
  - **Parcours** : DFS (Profondeur) et BFS (Largeur).
  - **Chemins** : Dijkstra (Plus court chemin).
  - **Arbres Couvrants** : Prim et Kruskal (MST).
  - **Analyse** : Détection de cycles.

## 🛠️ Stack Technique

- **Langage** : Python 3.8+
- **Interface Graphique** : `CustomTkinter` (Wrapper moderne pour Tkinter avec mode sombre/clair).
- **Structures de Graphe** : `NetworkX` (Standard industriel pour les graphes).
- **Rendu** : `Matplotlib` (Intégré dans Tkinter via `FigureCanvasTkAgg`).

## 💻 Installation

### 1. Créer un environnement virtuel (Recommandé)
```bash
python -m venv venv
# Windows
.\venv\Scripts\activate
# Linux/Mac
source venv/bin/activate
```

### 2. Installer les dépendances
```bash
pip install -r requirements.txt
```
*Le fichier `requirements.txt` contient : `customtkinter`, `matplotlib`, `networkx`.*

## 🚀 Utilisation

Lancez simplement le script principal :

```bash
python miniSysteme.py
```

L'application ouvrira un menu principal vous permettant de naviguer entre les modules (Listes, Arbres, Graphes).

## 📂 Organisation du Code
- `miniSysteme.py` : Le fichier unique contenant toute la logique (Classes GUI, Algorithmes, Visualisation) pour une portabilité maximale.

---
**Auteur** : Moustaoui Fatimaezzahrae
