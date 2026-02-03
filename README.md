# Projet Structures de Données

Application de gestion et visualisation de structures de données avec deux implémentations : **C/GTK4** et **Python/CustomTkinter**.

## 📁 Structure du Projet

```
projet-structures-de-donnees/
│
├── c-gtk4/              # Version C avec GTK4
│   └── ...
│
└── python/              # Version Python avec CustomTkinter
    ├── miniSysteme.py
    ├── requirements.txt
    └── README.md
```

## 🌟 Fonctionnalités

Les deux versions implémentent les mêmes fonctionnalités :

### 📋 Gestion de Listes
- Listes chaînées (simple et double)
- Opérations : insertion, suppression, modification
- Algorithmes de tri (Bubble, Insertion, Shell, Quick Sort)
- Comparaison de performance
- Visualisation graphique

### 🌳 Gestion d'Arbres
- Arbres Binaires de Recherche (BST)
- Arbres N-aires
- Parcours (Profondeur, Largeur)
- Conversion N-aire → Binaire
- Visualisation interactive

### 🕸️ Gestion de Graphes
- Graphes orientés/non-orientés
- Graphes pondérés/non-pondérés
- Algorithmes : DFS, BFS, Dijkstra, Prim, Kruskal
- Détection de cycles
- Visualisation des chemins

## 🚀 Versions

### Version C/GTK4

Interface native utilisant GTK4 pour une performance optimale.

**Technologies** :
- Langage C
- GTK4 (interface graphique)
- Cairo (rendu graphique)

**Voir** : [Documentation C](./c-gtk4/README.md)

### Version Python/CustomTkinter

Interface moderne et cross-platform avec Python.

**Technologies** :
- Python 3.8+
- CustomTkinter (interface graphique moderne)
- NetworkX (manipulation de graphes)
- Matplotlib (visualisation)

**Voir** : [Documentation Python](./python/README.md)

## 💻 Installation

### Version C
Consultez le [README C](./c-gtk4/README.md) pour les instructions d'installation.

### Version Python

```bash
cd python
pip install -r requirements.txt
python miniSysteme.py
```

## 📊 Comparaison des Versions

| Critère | C/GTK4 | Python/CustomTkinter |
|---------|--------|----------------------|
| **Performance** | ⭐⭐⭐⭐⭐ Très rapide | ⭐⭐⭐⭐ Rapide |
| **Portabilité** | ⭐⭐⭐ Nécessite GTK4 | ⭐⭐⭐⭐⭐ Cross-platform |
| **Facilité d'installation** | ⭐⭐⭐ Dépendances système | ⭐⭐⭐⭐⭐ pip install |
| **Développement** | ⭐⭐⭐ Plus verbeux | ⭐⭐⭐⭐⭐ Rapide et concis |
| **Taille exécutable** | ⭐⭐⭐⭐ ~19 MB | ⭐⭐⭐ ~150 KB (+ Python) |

## 🎯 Cas d'Usage

- **Version C** : Idéale pour les environnements où la performance est critique
- **Version Python** : Parfaite pour l'apprentissage, le prototypage rapide et la portabilité

## 🤝 Contribution

Les contributions sont les bienvenues sur les deux versions! 

1. Fork le projet
2. Créez une branche (`git checkout -b feature/amelioration`)
3. Commit vos changements (`git commit -m 'Ajout fonctionnalité'`)
4. Push vers la branche (`git push origin feature/amelioration`)
5. Ouvrez une Pull Request

## 📝 Licence

Ce projet est sous licence MIT.

## 👨‍💻 Auteur

**Moustaoui Fatimaezzahrae**
- GitHub: [@fatimaezzahrae-ux](https://github.com/fatimaezzahrae-ux)

## 📚 Ressources

- [Documentation GTK4](https://docs.gtk.org/gtk4/)
- [Documentation Python](https://docs.python.org/3/)
- [CustomTkinter](https://github.com/TomSchimansky/CustomTkinter)
- [NetworkX](https://networkx.org/)

---

⭐ Si ce projet vous a été utile, n'hésitez pas à lui donner une étoile!
