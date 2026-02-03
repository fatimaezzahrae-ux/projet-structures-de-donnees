# Mini Système - Gestion de Structures de Données

Une application graphique interactive pour la visualisation et la manipulation de structures de données (Listes, Arbres, Graphes).

## 🌟 Fonctionnalités

### 📋 Gestion de Listes
- **Types de données supportés** : Entier, Réel, Caractère, Chaîne de caractères
- **Structures** : Liste simple et liste doublement chaînée
- **Modes de remplissage** : Manuel ou Aléatoire
- **Opérations** :
  - Insertion (début, fin, indice)
  - Suppression par indice
  - Modification d'éléments
  - Tri avec plusieurs algorithmes
- **Algorithmes de tri** :
  - Tri à bulle (Bubble Sort)
  - Tri par insertion (Insertion Sort)
  - Tri Shell (Shell Sort)
  - Tri rapide (Quick Sort)
- **Comparaison de performance** entre les différents algorithmes de tri
- **Visualisation graphique** avec défilement horizontal/vertical

### 🌳 Gestion d'Arbres
- **Types d'arbres** :
  - BST (Arbre Binaire de Recherche)
  - Arbre N-aire
- **Opérations** :
  - Insertion de nœuds
  - Suppression de nœuds
  - Modification de valeurs
  - Conversion Arbre N-aire → Arbre Binaire
- **Parcours** :
  - Profondeur : Préfixe, Infixe, Postfixe
  - Largeur (BFS)
- **Génération aléatoire** d'arbres
- **Visualisation graphique** interactive avec NetworkX et Matplotlib
- **Export** au format JSON

### 🕸️ Gestion de Graphes
- **Types de graphes** :
  - Orienté / Non-orienté
  - Pondéré / Non-pondéré
- **Opérations** :
  - Ajout/Suppression de sommets
  - Ajout/Suppression d'arêtes
  - Modification de poids
- **Algorithmes** :
  - Parcours en profondeur (DFS)
  - Parcours en largeur (BFS)
  - Plus court chemin (Dijkstra)
  - Arbre couvrant minimal (Prim, Kruskal)
  - Détection de cycles
- **Visualisation interactive** avec mise en évidence des chemins

## 🚀 Installation

### Prérequis
- Python 3.8 ou supérieur
- pip (gestionnaire de paquets Python)

### Installation des dépendances

```bash
pip install -r requirements.txt
```

## 💻 Utilisation

### Lancer l'application

```bash
python miniSysteme.py
```

### Interface principale
L'application s'ouvre avec un menu principal permettant de choisir la structure de données à manipuler :
- **Listes** : Gestion de listes chaînées
- **Arbres** : Gestion d'arbres binaires et N-aires
- **Graphes** : Gestion de graphes

## 📦 Structure du projet

```
projet-en-python/
│
├── miniSysteme.py          # Application principale
├── miniSysteme.ipynb.ipynb # Version Jupyter Notebook
├── requirements.txt        # Dépendances Python
├── README.md              # Documentation
└── .gitignore             # Fichiers à ignorer par Git
```

## 🛠️ Technologies utilisées

- **CustomTkinter** : Interface graphique moderne
- **Tkinter** : Widgets GUI natifs
- **NetworkX** : Manipulation et visualisation de graphes
- **Matplotlib** : Visualisation graphique
- **Python Standard Library** : collections, uuid, json, etc.

## 📸 Captures d'écran

*(Ajoutez vos captures d'écran ici)*

## 🎯 Cas d'usage

- **Apprentissage** : Idéal pour comprendre visuellement les structures de données
- **Enseignement** : Outil pédagogique pour les cours d'algorithmique
- **Prototypage** : Test rapide d'algorithmes sur différentes structures

## 🤝 Contribution

Les contributions sont les bienvenues! N'hésitez pas à :
1. Fork le projet
2. Créer une branche (`git checkout -b feature/amelioration`)
3. Commit vos changements (`git commit -m 'Ajout d'une fonctionnalité'`)
4. Push vers la branche (`git push origin feature/amelioration`)
5. Ouvrir une Pull Request

## 📝 Licence

Ce projet est sous licence MIT - voir le fichier LICENSE pour plus de détails.

## 👨‍💻 Auteur

**Moustaoui Fatimaezzahrae**

## 🔗 Liens

- [Version C/GTK4](https://github.com/votre-username/votre-repo-c) *(Ajoutez le lien vers votre repo C)*
- [Documentation Python](https://docs.python.org/3/)
- [CustomTkinter Documentation](https://github.com/TomSchimansky/CustomTkinter)

## 📋 Notes de version

### Version 1.0.0
- Interface graphique complète avec CustomTkinter
- Gestion de listes chaînées (simple/double)
- Gestion d'arbres (BST, N-aire)
- Gestion de graphes (orienté/non-orienté, pondéré/non-pondéré)
- Algorithmes de tri et de parcours
- Visualisation interactive
- Export de données

---

⭐ Si ce projet vous a été utile, n'hésitez pas à lui donner une étoile sur GitHub!
