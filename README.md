# Projet Structures de Données 📚

Bienvenue dans ce dépôt dédié à la **visualisation et manipulation de structures de données**. 

Ce projet propose deux implémentations complètes d'une même application, permettant de comparer deux approches techniques différentes pour résoudre les mêmes problèmes algorithmiques.

## 🗂️ Choisissez votre version

Cliquez sur l'un des dossiers ci-dessous pour accéder à la documentation spécifique et au code source :

| 🚀 **[Version Native (C / GTK4)](./c-gtk4/README.md)** | 🐍 **[Version Moderne (Python)](./python/README.md)** |
| :--- | :--- |
| **Performance maximale** et gestion mémoire manuelle. Interface native fluide. | **Développement rapide** et portabilité. Interface moderne avec CustomTkinter. |
| [➡️ Voir la documentation C](./c-gtk4/README.md) | [➡️ Voir la documentation Python](./python/README.md) |

## 🌟 Ce que vous pouvez faire avec ce projet

Les deux versions permettent de visualiser et manipuler de manière interactive :

*   **Listes Chaînées** (Simples et Doubles)
*   **Algorithmes de Tri** (Visualisation pas à pas pour les tableaux)
*   **Arbres** (BST et N-aires avec parcours graphiques)
*   **Graphes** (Algorithmes de plus court chemin comme Dijkstra)

## 📊 Comparatif Technique

| Fonctionnalité | Version C / GTK4 | Version Python / CustomTkinter |
| :--- | :--- | :--- |
| **Langage** | C (Standard C11) | Python 3.8+ |
| **Interface Graphique** | GTK4 + Cairo | CustomTkinter + Matplotlib |
| **Gestion Graphes** | Implémentation manuelle (Matrices) | Librairie NetworkX |
| **Performance** | ⚡ Très Haute (~10ms chargement) | 🐢 Moyenne (Interprété) |
| **Installation** | Complexe (Nécessite compilation) | Simple (`pip install`) |
| **Taille Exécutable** | Légère (~19 Mo) | Lourde (si compilé en .exe) |

## 📁 Structure du Dépôt

```
projet-structures-de-donnees/
├── c-gtk4/              # Code source C, Headers, Makefiles
├── python/              # Script Python, Requirements
├── README.md            # Ce fichier
└── .gitignore
```



---
**Auteur** : Moustaoui Fatimaezzahrae ([@fatimaezzahrae-ux](https://github.com/fatimaezzahrae-ux))
