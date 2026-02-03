#!/usr/bin/env python
# coding: utf-8


import customtkinter as ctk
import tkinter as tk
from tkinter import ttk, messagebox, filedialog, simpledialog
import networkx as nx
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import random
import time
from collections import deque
import uuid
import string
import os
import json
import math
import heapq
from typing import Dict, List, Tuple, Optional

# Configuration de l'apparence
ctk.set_appearance_mode("System")
ctk.set_default_color_theme("blue")

# ============================
# Fonctions de tri
# ============================
def bubble_sort(arr):
    n = len(arr)
    for i in range(n):
        swapped = False
        for j in range(0, n-i-1):
            if arr[j] > arr[j+1]:
                arr[j], arr[j+1] = arr[j+1], arr[j]
                swapped = True
        if not swapped:
            break
    return arr

def insertion_sort(arr):
    for i in range(1, len(arr)):
        key = arr[i]
        j = i - 1
        while j >= 0 and key < arr[j]:
            arr[j + 1] = arr[j]
            j -= 1
        arr[j + 1] = key
    return arr

def shell_sort(arr):
    n = len(arr)
    gap = n // 2
    while gap > 0:
        for i in range(gap, n):
            temp = arr[i]
            j = i
            while j >= gap and arr[j - gap] > temp:
                arr[j] = arr[j - gap]
                j -= gap
            arr[j] = temp
        gap //= 2
    return arr

def quick_sort(arr):
    if len(arr) <= 1:
        return arr
    pivot = arr[len(arr) // 2]
    left = [x for x in arr if x < pivot]
    middle = [x for x in arr if x == pivot]
    right = [x for x in arr if x > pivot]
    return quick_sort(left) + middle + quick_sort(right)



# Nouvelle Classe pour la Saisie Manuelle en Bloc
class ManualInputWindow(ctk.CTkToplevel):
    def __init__(self, master, n_elements, data_type):
        super().__init__(master)
        self.master = master
        self.n_elements = n_elements
        self.data_type = data_type
        self.result_data = None  # Pour stocker la liste finale
        self.title("Saisie Manuelle des Valeurs")
        self.geometry("500x350")

        # Rendre la fenêtre modale
        self.lift()
        self.attributes('-topmost', True)
        self.focus_force()
        self.grab_set()

        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(2, weight=1)

        # Message d'instruction
        instruction = (
            f"Veuillez entrer les **{self.n_elements}** valeurs de type **{self.data_type}** "
            f"séparées par un **espace** ou une **nouvelle ligne**."
        )
        ctk.CTkLabel(self, text=instruction, justify="left", wraplength=480, font=("Arial", 12, "bold")).grid(
            row=0, column=0, padx=20, pady=(10, 5), sticky="w")
        
        # Exemple de format attendu
        example_text = self._get_example_text()
        ctk.CTkLabel(self, text=f"Exemple : {example_text}", text_color="#3498db").grid(
            row=1, column=0, padx=20, pady=(0, 5), sticky="w")

        # Zone de texte pour la saisie
        self.input_textbox = ctk.CTkTextbox(self, width=450, height=150)
        self.input_textbox.grid(row=2, column=0, padx=20, pady=10, sticky="nsew")

        # Cadre pour les boutons
        button_frame = ctk.CTkFrame(self)
        button_frame.grid(row=3, column=0, padx=20, pady=(5, 10), sticky="ew")
        button_frame.grid_columnconfigure((0, 1), weight=1)

        ctk.CTkButton(button_frame, text="Valider", command=self._validate_input).grid(
            row=0, column=0, padx=10, pady=10, sticky="ew")
        ctk.CTkButton(button_frame, text="Annuler", command=self.destroy, fg_color="#c0392b").grid(
            row=0, column=1, padx=10, pady=10, sticky="ew")
            
    def _get_example_text(self):
        """Fournit un exemple de format basé sur le type de données."""
        if self.data_type == "Entier":
            return "10 5 -20 15 8"
        elif self.data_type == "Réel":
            return "1.2 5.0 3.14 -0.5"
        elif self.data_type == "Caractère":
            return "a b c d e"
        elif self.data_type == "Chaîne de caractère":
            return "pomme banane kiwi orange"
        return "val1 val2 val3 ..."

    def _get_converted_value(self, value_str: str):
        """Tente de convertir une chaîne en son type cible."""
        value_str = value_str.strip()
        
        if self.data_type == "Entier":
            return int(value_str)
        elif self.data_type == "Réel":
            return float(value_str)
        elif self.data_type == "Caractère":
            if len(value_str) != 1 or value_str.isdigit(): # Validation pour caractère
                 raise ValueError("Doit être un seul caractère non numérique.")
            return value_str
        elif self.data_type == "Chaîne de caractère":
            if any(char.isdigit() for char in value_str): # Validation pour chaîne (pas de chiffres)
                raise ValueError("La chaîne ne doit contenir que des lettres.")
            return value_str
        return value_str

    def _validate_input(self):
        """Valide la saisie en bloc et convertit en liste."""
        raw_text = self.input_textbox.get("1.0", "end-1c").strip()
        
        # 1. Nettoyer et séparer les valeurs
        # Remplace les nouvelles lignes et les tabulations par des espaces, puis divise
        values_str = raw_text.replace('\n', ' ').replace('\t', ' ').split()

        # 2. Vérifier le nombre d'éléments
        if len(values_str) != self.n_elements:
            messagebox.showerror(
                "Erreur de Taille", 
                f"Le nombre d'éléments saisis est de {len(values_str)}, mais {self.n_elements} sont attendus. Veuillez corriger.", 
                parent=self
            )
            return

        # 3. Valider le type de chaque élément
        new_list = []
        error_found = False
        
        for i, val_str in enumerate(values_str):
            try:
                converted_val = self._get_converted_value(val_str)
                new_list.append(converted_val)
            except ValueError as e:
                # IMPORTANT: Afficher le message d'erreur avec le numéro d'élément
                error_msg = f"Erreur pour l'élément #{i+1} ('{val_str}'):\n"
                if self.data_type == "Entier" or self.data_type == "Réel":
                    error_msg += f"Valeur invalide. Un nombre de type **{self.data_type}** est requis."
                elif self.data_type == "Caractère":
                    error_msg += "Doit être un seul caractère (non numérique)."
                elif self.data_type == "Chaîne de caractère":
                    error_msg += "La chaîne ne doit contenir que des lettres (pas de chiffres)."
                else:
                    error_msg += str(e)

                messagebox.showerror("Erreur de Format", error_msg, parent=self)
                error_found = True
                break
            
        if not error_found:
            self.result_data = new_list
            self.destroy() # Fermer la fenêtre en cas de succès

    def show(self):
        """Attend que la fenêtre soit fermée et renvoie les données."""
        # Pour une modale simple, on peut utiliser le .wait_window()
        self.master.wait_window(self)
        return self.result_data


# In[5]:


 

# ATTENTION: Les fonctions de tri (bubble_sort, insertion_sort, etc.) 
# DOIVENT être définies séparément dans votre script pour que ce code fonctionne.
# Note: Dans cette version corrigée, elles sont intégrées comme méthodes de la classe.

class ListWindow(ctk.CTkToplevel):
    def __init__(self, master=None):
        super().__init__(master)
        self.title("Gestion de Listes")
        self.geometry("1000x650")
        
        # Correction: focus sur la fenêtre pour éviter qu'elle ne parte derrière
        self.lift()
        self.focus_force()
        self.grab_set()
        
        # Données
        self.list_data = []      
        self.sorted_data = []    
        self.structure_type = "Simple"
        self.data_type = "Entier"
        self.mode_type = "Aléatoire"
        self.sort_method = "Tri rapide"
        
        # Grille principale (3 lignes: Config/Boutons, Dessin, Opérations)
        self.grid_columnconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=2)
        self.grid_rowconfigure(0, weight=0) # Ligne 0: Config/Boutons et Résultats (Haut)
        self.grid_rowconfigure(1, weight=1) # Ligne 1: Dessin (Milieu, prend le plus d'espace)
        self.grid_rowconfigure(2, weight=0) # Ligne 2: Opérations (Bas)
        
        # --------------------------
        # Cadre de configuration (Colonne 0, Ligne 0)
        # --------------------------
        config_frame = ctk.CTkFrame(self)
        config_frame.grid(row=0, column=0, padx=10, pady=10, sticky="nsew")
        config_frame.grid_columnconfigure(0, weight=1)
        
        ctk.CTkLabel(config_frame, text="Type de données:").grid(row=0, column=0, sticky="w")
        self.type_menu = ctk.CTkComboBox(config_frame, values=["Entier", "Réel", "Caractère", "Chaîne de caractère"])
        self.type_menu.set("Entier")
        self.type_menu.grid(row=1, column=0, pady=5, sticky="ew")
        
        ctk.CTkLabel(config_frame, text="Structure:").grid(row=2, column=0, sticky="w")
        self.structure_menu = ctk.CTkComboBox(config_frame, values=["Simple", "Double"])
        self.structure_menu.set("Simple")
        self.structure_menu.grid(row=3, column=0, pady=5, sticky="ew")
        
        ctk.CTkLabel(config_frame, text="Mode de remplissage:").grid(row=4, column=0, sticky="w")
        self.mode_menu = ctk.CTkComboBox(config_frame, values=["Manuel", "Aléatoire"], command=self.toggle_input_mode)
        self.mode_menu.set("Aléatoire")
        self.mode_menu.grid(row=5, column=0, pady=5, sticky="ew")
        
        ctk.CTkLabel(config_frame, text="Taille de la liste:").grid(row=6, column=0, sticky="w")
        self.size_entry = ctk.CTkEntry(config_frame)
        self.size_entry.insert(0, "0")
        self.size_entry.grid(row=7, column=0, pady=5, sticky="ew")
        
        ctk.CTkLabel(config_frame, text="Méthode de tri:").grid(row=8, column=0, sticky="w")
        self.method_menu = ctk.CTkComboBox(config_frame, values=["Tri à bulle", "Tri par insertion", "Tri Shell", "Tri rapide"])
        self.method_menu.set("Tri rapide")
        self.method_menu.grid(row=9, column=0, pady=5, sticky="ew")
        
        # --------------------------
        # Cadre des boutons et Résultats (Colonne 1, Ligne 0)
        # --------------------------
        button_results_frame = ctk.CTkFrame(self)
        button_results_frame.grid(row=0, column=1, padx=10, pady=10, sticky="nsew")
        button_results_frame.grid_columnconfigure((0, 1, 2), weight=1)
        
        # --- Ligne 0 et 1: Les boutons ---
        ctk.CTkButton(button_results_frame, text="Créer liste", command=self.create_list).grid(row=0, column=0, padx=5, pady=5)
        ctk.CTkButton(button_results_frame, text="Trier liste", command=self.sort_list).grid(row=0, column=1, padx=5, pady=5)
        
        # BOUTON RETOUR AJOUTÉ ICI
        ctk.CTkButton(button_results_frame, text="🔙 Retour Menu",  command=self.destroy).grid(row=0, column=2, padx=5, pady=5)
        
        ctk.CTkButton(button_results_frame, text="Comparer tris", command=self.compare_sorts).grid(row=1, column=0, padx=5, pady=5)
        ctk.CTkButton(button_results_frame, text="Réinitialiser", command=self.reset_list).grid(row=1, column=1, padx=5, pady=5)
        ctk.CTkButton(button_results_frame, text="Enregistrer", command=self.save_list).grid(row=1, column=2, padx=5, pady=5)
        
        # --- Ligne 2 et 3: Zone de Texte pour les Résultats ---
        ctk.CTkLabel(button_results_frame, text="Résultats de la Comparaison des Tris :", font=("Arial", 12, "bold")).grid(
            row=2, column=0, columnspan=3, sticky="w", padx=5, pady=(10, 0))
            
        self.comparison_textbox = ctk.CTkTextbox(button_results_frame, height=120)
        self.comparison_textbox.grid(row=3, column=0, columnspan=3, padx=5, pady=(0, 5), sticky="nsew")
        self.comparison_textbox.insert("1.0", "Les temps de tri seront affichés ici après la comparaison.")
        self.comparison_textbox.configure(state="disabled") 
        
        # --------------------------
        # Cadre pour dessin de la liste (AVEC BARRE DE DÉFILEMENT) (Ligne 1)
        # --------------------------
        self.canvas_frame = ctk.CTkFrame(self)
        self.canvas_frame.grid(row=1, column=0, columnspan=2, padx=10, pady=10, sticky="nsew")
        self.canvas_frame.grid_rowconfigure(0, weight=1)
        self.canvas_frame.grid_columnconfigure(0, weight=1)
        
        # Canvas principal (L'ensemble du fond noir)
        self.list_canvas = ctk.CTkCanvas(self.canvas_frame, bg="#333333") 
        self.list_canvas.grid(row=0, column=0, sticky="nsew") 
        
        # Barre de défilement horizontale
        self.h_scrollbar = ctk.CTkScrollbar(self.canvas_frame, orientation="horizontal", command=self.list_canvas.xview)
        self.h_scrollbar.grid(row=1, column=0, sticky="ew")

        # Barre de défilement verticale (NOUVEAU)
        self.v_scrollbar = ctk.CTkScrollbar(self.canvas_frame, orientation="vertical", command=self.list_canvas.yview)
        self.v_scrollbar.grid(row=0, column=1, sticky="ns")

        # Lier le canvas aux barres de défilement
        self.list_canvas.configure(xscrollcommand=self.h_scrollbar.set, yscrollcommand=self.v_scrollbar.set)  # MODIFIÉ
        
        
        # --------------------------
        # Cadre des Opérations (Insertion/Suppression/Modification) (Ligne 2)
        # --------------------------
        self.operations_frame = ctk.CTkFrame(self, border_width=2, border_color="#3498db")
        self.operations_frame.grid(row=2, column=0, columnspan=2, padx=10, pady=(0, 10), sticky="ew")
        self.operations_frame.grid_columnconfigure((0, 1, 2, 3, 4, 5, 6), weight=1) 
        
        # --- Ligne 0: Libellés ---
        ctk.CTkLabel(self.operations_frame, text="Valeur :").grid(row=0, column=0, padx=5, pady=(10, 0), sticky="w")
        ctk.CTkLabel(self.operations_frame, text="Position :").grid(row=0, column=1, padx=5, pady=(10, 0), sticky="w")
        ctk.CTkLabel(self.operations_frame, text="Indice :").grid(row=0, column=2, padx=5, pady=(10, 0), sticky="w")
        
        # --- Ligne 1: Entrées et Contrôles ---
        
        # 1. Champ de Valeur (pour Insérer/Modifier)
        self.op_value_entry = ctk.CTkEntry(self.operations_frame, placeholder_text="00")
        self.op_value_entry.grid(row=1, column=0, padx=5, pady=5, sticky="ew")
        
        # 2. Menu de Position d'Insertion (Début/Fin/Indice)
        self.op_position_menu = ctk.CTkComboBox(self.operations_frame, values=["Fin", "Début", "Indice"], width=100)
        self.op_position_menu.set("Fin")
        self.op_position_menu.grid(row=1, column=1, padx=5, pady=5, sticky="ew")
        
        # 3. Champ d'Indice (pour Insérer/Supprimer/Modifier par Index)
        self.op_index_entry = ctk.CTkEntry(self.operations_frame, placeholder_text="Indice")
        self.op_index_entry.grid(row=1, column=2, padx=5, pady=5, sticky="ew")
        
        # 4. Boutons d'Opération 
        ctk.CTkButton(self.operations_frame, text="Insérer", fg_color="#27ae60", command=self.insert_element_gui).grid(
            row=1, column=3, padx=5, pady=5, sticky="ew")
        ctk.CTkButton(self.operations_frame, text="Supprimer", fg_color="#c0392b", command=self.delete_element_gui).grid(
            row=1, column=4, padx=5, pady=5, sticky="ew")
        ctk.CTkButton(self.operations_frame, text="Modifier", command=self.modify_element_gui).grid(
            row=1, column=5, padx=5, pady=5, sticky="ew")
            
        # État initial
        self.toggle_input_mode(self.mode_menu.get())
        
    # ===========================
    # Méthodes de logique
    # ===========================
    def toggle_input_mode(self, choice):
        self.mode_type = choice

    def _draw_list_labels(self, y_avant, y_apres):
        """Dessine les labels AVANT et APRES Tri sur le canvas."""
        self.list_canvas.create_text(
            10, y_avant - 50, # Position pour "Liste AVANT Tri"
            text="Liste AVANT Tri:", 
            anchor="w", 
            font=("Arial", 12, "bold"), 
            fill="white"
        )
        self.list_canvas.create_text(
            10, y_apres - 50, # Position pour "Liste APRES Tri"
            text="Liste APRES Tri:", 
            anchor="w", 
            font=("Arial", 12, "bold"), 
            fill="white"
        )

    def generate_random_data(self, size, data_type):
        random_list = []
        for _ in range(size):
            if data_type == "Entier":
                value = random.randint(0, 100)
            
            elif data_type == "Réel":
                # GARANTI 2 CHIFFRES APRÈS LA VIRGULE
                value = round(random.uniform(0, 100), 2)
            
            elif data_type == "Caractère":
                value = random.choice(string.ascii_lowercase)
            
            elif data_type == "Chaîne de caractère":
                # GARANTI SEULEMENT DES LETTRES
                letters = string.ascii_letters 
                value = ''.join(random.choice(letters) for i in range(4))
                
            else:
                value = 0
            
            random_list.append(value)
        
        return random_list
    
    
    def create_list(self):
        try:
            n = int(self.size_entry.get())
            if n <= 0:
                messagebox.showerror("Erreur de Taille", "La taille de la liste doit être supérieure à 0.", parent=self)
                return
        except ValueError:
            messagebox.showerror("Erreur de Format", "La taille de la liste doit être un entier valide.", parent=self)
            return

        dtype = self.type_menu.get()
        
        if self.mode_type == "Aléatoire":
            # Ancien comportement (aléatoire)
            self.list_data = self.generate_random_data(n, dtype)
            
        elif self.mode_type == "Manuel":
            # NOUVEAU COMPORTEMENT: Appeler la fenêtre modale de saisie en bloc
            manual_window = ManualInputWindow(self, n, dtype)
            new_data = manual_window.show() # Bloque l'exécution jusqu'à la fermeture de la modale

            if new_data is not None:
                # Si l'utilisateur a validé et que la validation a réussi
                self.list_data = new_data
                self.sorted_data = [] # Réinitialiser le tri
            else:
                # L'utilisateur a annulé la saisie modale. 
                # Ne rien faire ou laisser l'ancienne liste si elle existe.
                # Si c'est la première création, la liste sera vide.
                if not self.list_data:
                    messagebox.showinfo("Annulation", "La création de liste a été annulée.", parent=self)
                return

        self.draw_list()

    def draw_list(self):
        self.list_canvas.delete("all")
    
        # Hauteurs fixes pour les deux lignes
        Y_AVANT = 100  
        Y_APRES = 250 
        rect_height = 50
    
        self._draw_list_labels(Y_AVANT, Y_APRES)
    
        n_avant = len(self.list_data)
        n_apres = len(self.sorted_data)
        n_max = max(n_avant, n_apres)
        spacing = 40 
        rect_width = 70
        x_offset = 150

        if n_max == 0:
             total_content_width = self.list_canvas.winfo_width() 
        else:
            MARGE_FINALE = 200 
            total_content_width = (n_max * rect_width) + ((n_max - 1) * spacing) + x_offset + MARGE_FINALE
        
        total_content_height = 350  
        self.list_canvas.config(scrollregion=(0, 0, total_content_width, total_content_height))
        self._draw_list_labels(Y_AVANT, Y_APRES)
    
        # Dessiner la liste principale avec interaction
        self._draw_single_list(self.list_data, Y_AVANT, rect_width, rect_height, spacing, x_offset=x_offset, is_interactive=True)
        # Dessiner la liste triée (statique)
        self._draw_single_list(self.sorted_data, Y_APRES, rect_width, rect_height, spacing, x_offset=x_offset, color="#2ecc71")
        
    def _draw_single_list(self, data, center_y, rect_width, rect_height, spacing, x_offset=0, color="#2980b9", is_interactive=False):
        """Fonction utilitaire pour dessiner une seule liste à une position Y donnée."""
        n = len(data)
        current_x = spacing + x_offset
        last_dot_x = 0

        for i, val in enumerate(data):
            x0 = current_x
            y0 = center_y - rect_height // 2
            x1 = x0 + rect_width
            y1 = y0 + rect_height
            
            # Tags pour l'interaction
            rect_tag = f"node_rect_{i}" if is_interactive else ""
            text_tag = f"node_text_{i}" if is_interactive else ""
            
            # 1. Rectangle (Nœud) divisé
            # Partie gauche (Valeur) + Partie droite (Pointeur)
            split_x = x0 + int(rect_width * 0.7)
            
            # Dessin du rectangle global
            self.list_canvas.create_rectangle(x0, y0, x1, y1, 
                                              fill=color, outline="#3498db", width=2,
                                              tags=rect_tag)
            # Ligne de séparation
            self.list_canvas.create_line(split_x, y0, split_x, y1, fill="#3498db", width=2, tags=rect_tag)
            
            # Valeur (Centrée dans la partie gauche)
            self.list_canvas.create_text((x0+split_x)//2, (y0+y1)//2, 
                                         text=str(val), font=("Arial", 12, "bold"), fill="white",
                                         tags=text_tag)
            
            # Binding pour le clic (Uniquement si interactif)
            if is_interactive:
                self.list_canvas.tag_bind(rect_tag, "<Button-1>", lambda event, idx=i: self.on_node_click(idx))
                self.list_canvas.tag_bind(text_tag, "<Button-1>", lambda event, idx=i: self.on_node_click(idx))

            # Point/Cercle (Centré dans la partie droite - Pointeur)
            dot_x = (split_x + x1) // 2
            dot_r = 3
            self.list_canvas.create_oval(dot_x - dot_r, center_y - dot_r, dot_x + dot_r, center_y + dot_r, fill="white", outline="white")
            
            last_dot_x = dot_x

            # 2. Flèches vers le suivant
            if i < n - 1:
                next_x = x1 + spacing
                
                if self.structure_menu.get() == "Simple":
                    self.list_canvas.create_line(dot_x, center_y, next_x, center_y,
                                                 arrow=tk.LAST, width=2, fill="white")
                else:
                    self.list_canvas.create_line(dot_x, center_y - 5, next_x, center_y - 5,
                                                 arrow=tk.LAST, width=2, fill="white")
                    self.list_canvas.create_line(next_x, center_y + 5, x1, center_y + 5,
                                                 arrow=tk.LAST, width=2, fill="white")
                    
            current_x = x1 + spacing 

        # 3. Dernier nœud pointe vers NULL
        if n > 0:
            start_x = last_dot_x
            # Distance suffisante pour sortir de la boite et aller vers NULL
            end_x = x1 + spacing * 0.75 
            
            self.list_canvas.create_line(start_x, center_y,
                                         end_x, center_y,
                                         arrow="last",
                                         width=2, fill="white")
                                         
            self.list_canvas.create_text(end_x + 10, center_y,
                                         text="NULL", 
                                         font=("Arial", 12, "bold"), 
                                         fill="#e74c3c",
                                         anchor="w")

    def on_node_click(self, index):
        """Ouvre une boîte de dialogue pour modifier la valeur du nœud cliqué."""
        current_val = self.list_data[index]
        dtype = self.type_menu.get()
        
        self.focus_force() # Focus avant
        dialog = ctk.CTkInputDialog(
            text=f"Modifier l'élément à l'indice {index} ({dtype}) :\nValeur actuelle : {current_val}",
            title="Saisie / Modification"
        )
        new_val_str = dialog.get_input()
        
        if new_val_str is None: # Annulé
            return
            
        if new_val_str.strip() == "":
            messagebox.showerror("Erreur", "La valeur ne peut pas être vide.", parent=self)
            return

        # Validation rapide
        try:
            if dtype == "Entier":
                val = int(new_val_str)
            elif dtype == "Réel":
                val = float(new_val_str)
            elif dtype == "Caractère":
                if len(new_val_str) != 1:
                     raise ValueError("Un seul caractère requis")
                val = new_val_str
            else:
                val = new_val_str
                
            self.list_data[index] = val
            self.sorted_data = [] # Invalider le tri précédent
            self.draw_list()
            
        except ValueError:
            messagebox.showerror("Erreur Format", f"Valeur incorrecte pour le type {dtype}", parent=self)
            
    def sort_list(self):
        # 1. Créer une copie de la liste originale pour le tri
        self.sorted_data = list(self.list_data)
        method = self.method_menu.get()
    
        # Utiliser les méthodes de la classe (self.) au lieu des fonctions globales
        if method == "Tri rapide":
            self.quick_sort(self.sorted_data)  
        elif method == "Tri à bulle":
            self.bubble_sort(self.sorted_data)  
        elif method == "Tri par insertion":
            self.insertion_sort(self.sorted_data)  
        elif method == "Tri Shell":
            self.shell_sort(self.sorted_data)  
        
        self.draw_list()
        print(f"Liste triée avec {method}.")

    def _get_converted_value(self, value_str: str):
        dtype = self.type_menu.get()
        if dtype == "Entier":
            return int(value_str)
        elif dtype == "Réel":
            return float(value_str)
        elif dtype == "Caractère":
            return value_str[0] if value_str else ''
        else:
            return value_str
            
    def insert_element_gui(self):
        val_str = self.op_value_entry.get()
        position = self.op_position_menu.get()
        index_str = self.op_index_entry.get()
    
        if not val_str:
            print("Erreur: Veuillez entrer une valeur.")
            return
        try:
            val = self._get_converted_value(val_str)
        
            if position == "Début":
                index = 0
            elif position == "Fin":
                index = len(self.list_data)
            elif position == "Indice":
                if not index_str or index_str == "Indice":
                    print("Erreur: Indice manquant pour l'insertion par Indice.")
                    return
                index = int(index_str)
                if index < 0 or index > len(self.list_data):
                    print("Erreur: Indice hors limites pour l'insertion.")
                    return
        
            self.list_data.insert(index, val)
            self.sorted_data = []  
            self.draw_list()
            print(f"'{val}' inséré à l'indice {index}.")
        
        except ValueError as e:
            print(f"Erreur de conversion/valeur: {e}")
        except Exception as e:
            print(f"Erreur lors de l'insertion: {e}")
        
    def delete_element_gui(self):
        index_str = self.op_index_entry.get()
        if not index_str or index_str == "Indice":
            print("Erreur: Veuillez entrer l'indice à supprimer.")
            return
        
        try:
            index = int(index_str)
            if index < 0 or index >= len(self.list_data):
                print("Erreur: Indice hors limites.")
                return
        
            deleted_val = self.list_data.pop(index)
            self.sorted_data = []  
            self.draw_list()
            print(f"Élément '{deleted_val}' supprimé à l'indice {index}.")
        
        except ValueError:
            print("Erreur: L'indice doit être un nombre entier.")
        except Exception as e:
            print(f"Erreur lors de la suppression: {e}")

    def modify_element_gui(self):
        index_str = self.op_index_entry.get()
        val_str = self.op_value_entry.get()
    
        if not index_str or index_str == "Indice" or not val_str:
            print("Erreur: Veuillez entrer la Valeur ET l'Indice à modifier.")
            return
        
        try:
            index = int(index_str)
        
            if index < 0 or index >= len(self.list_data):
                print("Erreur: Indice hors limites.")
                return
            new_val = self._get_converted_value(val_str)
        
            old_val = self.list_data[index]
            self.list_data[index] = new_val
            self.sorted_data = []  
            self.draw_list()
            print(f"Indice {index} modifié : '{old_val}' remplacé par '{new_val}'.")
        
        except ValueError:
            print(f"Erreur: Vérifiez que la valeur et l'indice sont valides.")
        except Exception as e:
            print(f"Erreur lors de la modification: {e}")

    def reset_list(self):
        self.list_data = []
        self.sorted_data = [] 
        self.draw_list()

    def bubble_sort(self, data_list):
        n = len(data_list)
        for i in range(n - 1):
            for j in range(0, n - i - 1):
                if data_list[j] > data_list[j + 1]:
                    data_list[j], data_list[j + 1] = data_list[j + 1], data_list[j]

    def insertion_sort(self, data_list):
        n = len(data_list)
        for i in range(1, n):
            key = data_list[i]
            j = i - 1
            while j >= 0 and data_list[j] > key:
                data_list[j + 1] = data_list[j]
                j -= 1
            data_list[j + 1] = key

    def shell_sort(self, data_list):
        n = len(data_list)
        gap = n // 2
        while gap > 0:
            for i in range(gap, n):
                temp = data_list[i]
                j = i
                while j >= gap and data_list[j - gap] > temp:
                    data_list[j] = data_list[j - gap]
                    j -= gap
                data_list[j] = temp
            gap //= 2

    def quick_sort(self, data_list):
        self._quick_sort_recursive(data_list, 0, len(data_list) - 1)

    def _quick_sort_recursive(self, data_list, low, high):
        if low < high:
            pi = self._partition(self.sorted_data, low, high)
            self._quick_sort_recursive(data_list, low, pi - 1)
            self._quick_sort_recursive(data_list, pi + 1, high)

    def _partition(self, data_list, low, high):
        pivot = data_list[high]
        i = low - 1 
        for j in range(low, high):
            if data_list[j] < pivot:
                i = i + 1
                data_list[i], data_list[j] = data_list[j], data_list[i]
        data_list[i + 1], data_list[high] = data_list[high], data_list[i + 1]
        return i + 1
    
    def compare_sorts(self):
        sort_funcs = {
            "Tri à bulle": self.bubble_sort,       
            "Tri par insertion": self.insertion_sort,
            "Tri Shell": self.shell_sort,
            "Tri rapide": self.quick_sort
        }
        results = {}
    
        if not self.list_data:
            message = "Veuillez d'abord créer ou remplir une liste."
            self.comparison_textbox.configure(state="normal") 
            self.comparison_textbox.delete("1.0", "end")      
            self.comparison_textbox.insert("1.0", message) 
            self.comparison_textbox.configure(state="disabled") 
            return

        for name, func in sort_funcs.items():
            temp_list = list(self.list_data)
            start = time.perf_counter()
            func(temp_list)
            end = time.perf_counter()
            results[name] = (end - start) * 1000 
        
        output_text = "--- Comparaison des Tris ---\n"
        for name, time_taken in results.items():
            output_text += f"{name}: {time_taken:.4f} ms\n"
        
        self.comparison_textbox.configure(state="normal") 
        self.comparison_textbox.delete("1.0", "end")      
        self.comparison_textbox.insert("1.0", output_text) 
        self.comparison_textbox.configure(state="disabled") 
        print(output_text)

    def save_list(self):
        path = filedialog.asksaveasfilename(defaultextension=".txt")
        if path:
            with open(path,"w") as f:
                f.write(str(self.list_data))


# In[6]:




# =========================================================================
# 1. VISUALIZATION HELPERS
# =========================================================================

def hierarchy_pos(G, root=None, width=1., vert_gap=0.2, vert_loc=0, xcenter=0.5):
    """Calcule les positions des nœuds pour un arbre hiérarchique."""
    if not nx.is_tree(G):
        return nx.spring_layout(G)
    return _hierarchy_pos(G, root, width, vert_gap, vert_loc, xcenter)

def _hierarchy_pos(G, root, width=1., vert_gap=0.2, vert_loc=0, xcenter=0.5, pos=None, parent=None):
    if pos is None:
        pos = {root: (xcenter, vert_loc)}
    else:
        pos[root] = (xcenter, vert_loc)
    children = list(G.neighbors(root))
    if not isinstance(G, nx.DiGraph) and parent is not None:
        if parent in children:
             children.remove(parent)
    if len(children) != 0:
        dx = width / len(children)
        nextx = xcenter - width/2 + dx/2
        for child in children:
            pos = _hierarchy_pos(G, child, width=dx, vert_gap=vert_gap,
                                 vert_loc=vert_loc-vert_gap, xcenter=nextx,
                                 pos=pos, parent=root)
            nextx += dx
    return pos

# =========================================================================
# 2. DATA STRUCTURE CLASSES
# =========================================================================

class Node:
    def __init__(self, value):
        self.id = str(uuid.uuid4())
        self.value = value

class BSTNode(Node):
    def __init__(self, value):
        super().__init__(value)
        self.left = None
        self.right = None

class NaryNode(Node):
    def __init__(self, value):
        super().__init__(value)
        self.children = []

class TreeStructure:
    def __init__(self):
        self.root = None
        self.logs = []
    def log(self, message): self.logs.append(message)
    def clear_logs(self): self.logs = []
    def get_height(self, node):
        if not node: return 0
        if isinstance(node, BSTNode):
            return 1 + max(self.get_height(node.left), self.get_height(node.right))
        elif isinstance(node, NaryNode):
            if not node.children: return 1
            return 1 + max(self.get_height(child) for child in node.children)
        return 0
    def get_size(self, node):
        if not node: return 0
        if isinstance(node, BSTNode):
            return 1 + self.get_size(node.left) + self.get_size(node.right)
        elif isinstance(node, NaryNode):
            return 1 + sum(self.get_size(child) for child in node.children)
        return 0

class BST(TreeStructure):
    def insert(self, value):
        if not self.root:
            self.root = BSTNode(value)
            self.log(f"Inséré (BST): {value}")
        else:
            self._insert_recursive(self.root, value)
    def _insert_recursive(self, node, value):
        if value < node.value:
            if node.left is None:
                node.left = BSTNode(value)
                self.log(f"Inséré {value} à gauche de {node.value}")
            else:
                self._insert_recursive(node.left, value)
        elif value > node.value:
            if node.right is None:
                node.right = BSTNode(value)
                self.log(f"Inséré {value} à droite de {node.value}")
            else:
                self._insert_recursive(node.right, value)
        else:
            self.log(f"Valeur {value} existe déjà")
    def delete(self, value):
        self.root, deleted = self._delete_recursive(self.root, value)
        if deleted: self.log(f"Supprimé (BST): {value}")
        else: self.log(f"Valeur {value} non trouvée")
    def _delete_recursive(self, node, value):
        if node is None: return node, False
        deleted = False
        if value < node.value:
            node.left, deleted = self._delete_recursive(node.left, value)
        elif value > node.value:
            node.right, deleted = self._delete_recursive(node.right, value)
        else:
            deleted = True
            if node.left is None: return node.right, True
            elif node.right is None: return node.left, True
            temp = self._min_value_node(node.right)
            node.value = temp.value
            node.right, _ = self._delete_recursive(node.right, temp.value)
        return node, deleted
    def _min_value_node(self, node):
        current = node
        while current.left is not None: current = current.left
        return current
    def modify(self, old_value, new_value):
        self.log(f"Modifié (BST): {old_value} -> {new_value}")
        self.delete(old_value)
        self.insert(new_value)
    def traverse_preorder(self): res = []; self._preorder(self.root, res); return res
    def _preorder(self, node, res):
        if node: res.append(node.value); self._preorder(node.left, res); self._preorder(node.right, res)
    def traverse_inorder(self): res = []; self._inorder(self.root, res); return res
    def _inorder(self, node, res):
        if node: self._inorder(node.left, res); res.append(node.value); self._inorder(node.right, res)
    def traverse_postorder(self): res = []; self._postorder(self.root, res); return res
    def _postorder(self, node, res):
        if node: self._postorder(node.left, res); self._postorder(node.right, res); res.append(node.value)
    def traverse_bfs(self):
        if not self.root: return []
        res = []; queue = deque([self.root])
        while queue:
            node = queue.popleft(); res.append(node.value)
            if node.left: queue.append(node.left)
            if node.right: queue.append(node.right)
        return res
    def to_networkx(self):
        G = nx.DiGraph()
        if self.root: self._add_to_graph(G, self.root)
        return G
    def _add_to_graph(self, G, node):
        G.add_node(node.id, label=str(node.value))
        if node.left: G.add_edge(node.id, node.left.id); self._add_to_graph(G, node.left)
        if node.right: G.add_edge(node.id, node.right.id); self._add_to_graph(G, node.right)

class NaryTree(TreeStructure):
    def insert(self, value, parent_value=None):
        if not self.root:
            self.root = NaryNode(value); self.log(f"Inséré Racine (N-ary): {value}"); return
        if parent_value is None: self.log("Erreur: Valeur parent requise"); return
        parent = self._find_node(self.root, parent_value)
        if parent: parent.children.append(NaryNode(value)); self.log(f"Inséré {value} sous {parent_value}")
        else: self.log(f"Erreur: Parent {parent_value} non trouvé")
    def delete(self, value):
        if not self.root: return
        if self.root.value == value:
            if self.root.children:
                new_root = self.root.children[0]
                for child in self.root.children[1:]: new_root.children.append(child)
                self.root = new_root
                self.log(f"Supprimé {value}, promu {new_root.value}")
            else: self.root = None; self.log(f"Supprimé racine {value}")
            return
        parent = self._find_parent(self.root, value)
        if parent:
            for i, child in enumerate(parent.children):
                if child.value == value:
                    parent.children.extend(child.children); del parent.children[i]
                    self.log(f"Supprimé (N-ary): {value}"); return
        else: self.log(f"Valeur {value} non trouvée")
    def modify(self, old_value, new_value):
        node = self._find_node(self.root, old_value)
        if node: node.value = new_value; self.log(f"Modifié (N-ary): {old_value} -> {new_value}")
    def _find_node(self, node, value):
        if not node: return None
        if node.value == value: return node
        for child in node.children:
            res = self._find_node(child, value)
            if res: return res
        return None
    def _find_parent(self, node, value):
        if not node: return None
        for child in node.children:
            if child.value == value: return node
            res = self._find_parent(child, value); 
            if res: return res
        return None
    def traverse_preorder(self): res = []; self._preorder(self.root, res); return res
    def _preorder(self, node, res):
        if node: res.append(node.value); 
        for child in node.children: self._preorder(child, res)
    def traverse_postorder(self): res = []; self._postorder(self.root, res); return res
    def _postorder(self, node, res):
        if node: 
            for child in node.children: self._postorder(child, res)
            res.append(node.value)
    def traverse_bfs(self):
        if not self.root: return []
        res = []; queue = deque([self.root])
        while queue:
            node = queue.popleft(); res.append(node.value)
            for child in node.children: queue.append(child)
        return res
    def traverse_inorder(self): return self.traverse_preorder()
    def to_networkx(self):
        G = nx.DiGraph(); 
        if self.root: self._add_to_graph(G, self.root)
        return G
    def _add_to_graph(self, G, node):
        G.add_node(node.id, label=str(node.value))
        for child in node.children: G.add_edge(node.id, child.id); self._add_to_graph(G, child)
    def to_binary(self):
        if not self.root: return BST()
        bst = BST(); bst.root = self._transform_node(self.root)
        bst.log("Converti N-aire -> Binaire")
        return bst
    def _transform_node(self, nary_node):
        if not nary_node: return None
        b_node = BSTNode(nary_node.value)
        if nary_node.children:
            b_node.left = self._transform_node(nary_node.children[0])
            current_b_child = b_node.left
            for i in range(1, len(nary_node.children)):
                current_b_child.right = self._transform_node(nary_node.children[i])
                current_b_child = current_b_child.right
        return b_node

# =========================================================================
# 3. CLASS TREE WINDOW (Replicated C/GTK Style)
# =========================================================================

class TreeWindow(ctk.CTkToplevel):
    def __init__(self, master=None):
        super().__init__(master)
        self.title("Gestion de Structures d'Arbres 🌳")
        self.geometry("1100x800")
        self.resizable(True, True)
        
        # Correction z-order
        self.lift()
        self.focus_force()
        self.grab_set()
        
        # Colors from C Version
        self.COLOR_INSERT = "#2ecc71"
        self.COLOR_DELETE = "#e74c3c"
        self.COLOR_MODIFY = "#f39c12"
        self.COLOR_GENERATE = "#9b59b6"
        self.COLOR_EXECUTE = "#3498db"
        self.COLOR_CLEAR = "#c0392b" # Used for text in C, but will use button color here

        self.bst = BST()
        self.nary = NaryTree()
        self.current_tree_type = 'BST'
        self.current_tree = self.bst
        
        self._init_vars()
        self._setup_ui()

    def _init_vars(self):
        self.val_var = tk.StringVar(value="")
        self.insert_parent_var = tk.StringVar(value="") # For N-ary Insert
        self.new_val_var = tk.StringVar(value="")    # For Modify
        
        self.tree_var = ctk.StringVar(value='🌳 BST (Binaire)')
        self.data_type_var = ctk.StringVar(value='1️⃣ Entier')
        
        self.input_mode_var = ctk.StringVar(value='Manuelle') # Manuelle / Aléatoire
        self.nary_degree_var = tk.StringVar(value="3")
        self.rand_size_var = tk.StringVar(value="0")
        
        self.trav_type_var = ctk.StringVar(value='⬇️ Profondeur')
        self.trav_method_var = ctk.StringVar(value='Prefixe (Pré-ordre)')
        self.trav_res_var = ctk.StringVar(value="Résultat...")
        self.stats_var = ctk.StringVar(value="Taille: 0 | Hauteur: 0")

    def _setup_ui(self):
        # Main Layout: Sidebar (Left) + Visualization (Right)
        self.grid_columnconfigure(0, weight=0) # Sidebar fixed width approx
        self.grid_columnconfigure(1, weight=1) # Viz expands
        self.grid_rowconfigure(0, weight=1)

        # --- Sidebar ---
        self.sidebar = ctk.CTkScrollableFrame(self, width=380, label_text="Contrôles")
        self.sidebar.grid(row=0, column=0, sticky="nsew", padx=10, pady=10)

        # Frame 1: Configuration & Opérations
        self._build_config_frame()
        
        # Frame 2: Création de l'Arbre
        self._build_creation_frame()
        
        # Frame 3: Parcours
        self._build_trav_frame()
        
        # Frame 4: Logs
        self._build_log_frame()

        self.btn_save = ctk.CTkButton(
            self.sidebar, 
            text="💾 Enregistrer Arbre", 
            command=self._on_save, 
            fg_color=self.COLOR_EXECUTE # Utilisation d'une couleur d'exécution/action
        )
        self.btn_save.pack(fill="x", pady=5)
        
        # Retour Button
        self.btn_back = ctk.CTkButton(self.sidebar, text="🔙 Retour Menu", command=self.destroy)
        self.btn_back.pack(fill="x", pady=10)

        # --- Right Panel ---
        self.right_panel = ctk.CTkFrame(self, fg_color="transparent")
        self.right_panel.grid(row=0, column=1, sticky="nsew", padx=(0, 10), pady=10)
        self.right_panel.grid_columnconfigure(0, weight=1)
        self.right_panel.grid_rowconfigure(1, weight=1)

        # Stats Label
        self.lbl_stats = ctk.CTkLabel(self.right_panel, textvariable=self.stats_var, font=("Arial", 14, "bold"))
        self.lbl_stats.pack(fill="x", pady=5)

        # Drawing Area with Scrollbars
        self._build_drawing_area()
        
        # Initial State
        self._on_tree_type_change(None)
        self._on_input_mode_change()
        self._toggle_save_button()
        

    def _build_config_frame(self):
        fr = ctk.CTkFrame(self.sidebar)
        fr.pack(fill="x", pady=5, padx=5)
        
        ctk.CTkLabel(fr, text="⚙️ Configuration & Opérations", font=("Arial", 13, "bold"), anchor="w").pack(fill="x", padx=5, pady=5)
        
        # Type
        ctk.CTkLabel(fr, text="Type d'arbre:", anchor="w").pack(fill="x", padx=5)
        self.combo_tree_type = ctk.CTkComboBox(fr, variable=self.tree_var, 
                                               values=['🌳 BST (Binaire)', '🌿 N aire'],
                                               command=self._on_tree_type_change)
        self.combo_tree_type.pack(fill="x", padx=5, pady=2)
        
        # Value
        ctk.CTkLabel(fr, text="Valeur:", anchor="w").pack(fill="x", padx=5)
        self.entry_val = ctk.CTkEntry(fr, textvariable=self.val_var, placeholder_text="Ex: 42")
        self.entry_val.pack(fill="x", padx=5, pady=2)
        
        # Parent (Insert N-ary)
        self.lbl_insert_parent = ctk.CTkLabel(fr, text="Parent (Insertion N aire):", anchor="w")
        self.lbl_insert_parent.pack(fill="x", padx=5)
        self.entry_insert_parent = ctk.CTkEntry(fr, textvariable=self.insert_parent_var, placeholder_text="Ex: 5")
        self.entry_insert_parent.pack(fill="x", padx=5, pady=2)
        
        # New Value (Modify)
        self.lbl_new_val = ctk.CTkLabel(fr, text="Nouvelle Valeur (Modifier):", anchor="w")
        self.lbl_new_val.pack(fill="x", padx=5)
        self.entry_new_val = ctk.CTkEntry(fr, textvariable=self.new_val_var)
        self.entry_new_val.pack(fill="x", padx=5, pady=2)
        
        # Buttons Box
        bbox = ctk.CTkFrame(fr, fg_color="transparent")
        bbox.pack(fill="x", pady=10)
        
        ctk.CTkButton(bbox, text="➕ Insérer", width=80, fg_color=self.COLOR_INSERT, command=self._on_insert).pack(side="left", padx=2, expand=True)
        ctk.CTkButton(bbox, text="🗑️ Supprimer", width=80, fg_color=self.COLOR_DELETE, command=self._on_delete).pack(side="left", padx=2, expand=True)
        ctk.CTkButton(bbox, text="✏️ Modifier", width=80, fg_color=self.COLOR_MODIFY, command=self._on_modify).pack(side="left", padx=2, expand=True)

    def _build_creation_frame(self):
        fr = ctk.CTkFrame(self.sidebar)
        fr.pack(fill="x", pady=5, padx=5)
        
        ctk.CTkLabel(fr, text="🌱 Création de l'Arbre", font=("Arial", 13, "bold"), anchor="w").pack(fill="x", padx=5, pady=5)
        
        ctk.CTkLabel(fr, text="Type de Données:", anchor="w").pack(fill="x", padx=5)
        ctk.CTkComboBox(fr, variable=self.data_type_var, values=['1️⃣ Entier', '🔢 Float', '🔤 Caractère', '📝 Chaîne']).pack(fill="x", padx=5, pady=2)
        
        # Radios
        rbox = ctk.CTkFrame(fr, fg_color="transparent")
        rbox.pack(fill="x", pady=5)
        ctk.CTkRadioButton(rbox, text="✍️ Manuelle", variable=self.input_mode_var, value='Manuelle', command=self._on_input_mode_change).pack(side="left", padx=5)
        ctk.CTkRadioButton(rbox, text="🎲 Aléatoire", variable=self.input_mode_var, value='Aléatoire', command=self._on_input_mode_change).pack(side="left", padx=5)
        
        # Manual Container
        self.fr_manual = ctk.CTkFrame(fr, fg_color="transparent")
        self.fr_manual.pack(fill="x", padx=5)
        
        # Degree Box (N-ary only)
        self.box_degree = ctk.CTkFrame(self.fr_manual, fg_color="transparent")
        self.box_degree.pack(fill="x", pady=2)
        ctk.CTkLabel(self.box_degree, text="Degré (N):", width=60).pack(side="left")
        ctk.CTkEntry(self.box_degree, textvariable=self.nary_degree_var, width=50).pack(side="left")
        
        ctk.CTkButton(self.fr_manual, text="📝 Saisir Manuellement (Dialog)", command=self._open_manual_dialog).pack(fill="x", pady=5)
        
        # Random Container
        self.fr_random = ctk.CTkFrame(fr, fg_color="transparent")
        
        ctk.CTkLabel(self.fr_random, text="Taille Aléatoire:", anchor="w").pack(fill="x")
        ctk.CTkEntry(self.fr_random, textvariable=self.rand_size_var).pack(fill="x", pady=2)
        ctk.CTkButton(self.fr_random, text="⚡ Générer", fg_color=self.COLOR_GENERATE, command=self._on_generate).pack(fill="x", pady=5)

        # Bottom Buttons
        ctk.CTkButton(fr, text="🧹 Vider", fg_color=self.COLOR_CLEAR, command=self._on_clear).pack(fill="x", padx=5, pady=5)
        self.btn_convert = ctk.CTkButton(fr, text="🔄 Convertir -> Binaire", command=self._on_convert)
        self.btn_convert.pack(fill="x", padx=5, pady=5)

    def _build_trav_frame(self):
        fr = ctk.CTkFrame(self.sidebar)
        fr.pack(fill="x", pady=5, padx=5)
        
        ctk.CTkLabel(fr, text="🚶 Parcours", font=("Arial", 13, "bold"), anchor="w").pack(fill="x", padx=5, pady=5)
        
        ctk.CTkLabel(fr, text="Type:", anchor="w").pack(fill="x", padx=5)
        self.combo_trav_type = ctk.CTkComboBox(fr, variable=self.trav_type_var, 
                                               values=['⬇️ Profondeur', '↔️ Largeur'],
                                               command=self._on_trav_change)
        self.combo_trav_type.pack(fill="x", padx=5, pady=2)
        
        ctk.CTkLabel(fr, text="Méthode:", anchor="w").pack(fill="x", padx=5)
        self.combo_trav_method = ctk.CTkComboBox(fr, variable=self.trav_method_var, 
                                                 values=['Prefixe (Pré-ordre)', 'Infixe (In-ordre)', 'Postfixe (Post-ordre)'])
        self.combo_trav_method.pack(fill="x", padx=5, pady=2)
        
        ctk.CTkButton(fr, text="▶️ Exécuter le Parcours", fg_color=self.COLOR_EXECUTE, command=self._on_execute_trav).pack(fill="x", padx=5, pady=10)
        
        ctk.CTkLabel(fr, textvariable=self.trav_res_var, wraplength=350, justify="left").pack(fill="x", padx=5, pady=5)

    def _build_log_frame(self):
        fr = ctk.CTkFrame(self.sidebar)
        fr.pack(fill="x", pady=5, padx=5)
        ctk.CTkLabel(fr, text="📜 Logs", font=("Arial", 13, "bold"), anchor="w").pack(fill="x", padx=5, pady=5)
        self.log_text = ctk.CTkTextbox(fr, height=150)
        self.log_text.pack(fill="x", padx=5, pady=5)
        self.log_text.configure(state="disabled")

    def _build_drawing_area(self):
        # Container for layout
        viz_container = ctk.CTkFrame(self.right_panel)
        viz_container.pack(fill="both", expand=True) # expand to take all space
        viz_container.grid_rowconfigure(0, weight=1)
        viz_container.grid_columnconfigure(0, weight=1)
        
        # Helper canvas for scrolling
        self.scroll_canvas = tk.Canvas(viz_container, bg="white", highlightthickness=0)
        self.scroll_canvas.grid(row=0, column=0, sticky="nsew")
        
        h_scroll = ctk.CTkScrollbar(viz_container, orientation="horizontal", command=self.scroll_canvas.xview)
        h_scroll.grid(row=1, column=0, sticky="ew")
        
        v_scroll = ctk.CTkScrollbar(viz_container, orientation="vertical", command=self.scroll_canvas.yview)
        v_scroll.grid(row=0, column=1, sticky="ns")
        
        self.scroll_canvas.configure(xscrollcommand=h_scroll.set, yscrollcommand=v_scroll.set)
        
        # Matplotlib integration
        self.fig = plt.Figure(figsize=(5, 4), dpi=100)
        self.ax = self.fig.add_subplot(111)
        
        self.canvas_agg = FigureCanvasTkAgg(self.fig, master=self.scroll_canvas)
        self.canvas_widget = self.canvas_agg.get_tk_widget()
        
        self.scroll_canvas.create_window((0, 0), window=self.canvas_widget, anchor="nw")
        
        # Force refresh
        self._refresh_viz()

    # --- Logic ---

    # Dans la section Logique (après _parse_val, par exemple)
    def _toggle_save_button(self):
        """Active ou désactive le bouton Enregistrer en fonction de la présence de la racine."""
        if self.current_tree and self.current_tree.root:
            self.btn_save.configure(state="normal")
        else:
            self.btn_save.configure(state="disabled")

    def _log(self, msg):
        self.log_text.configure(state="normal")
        self.log_text.insert("end", msg + "\n")
        self.log_text.see("end")
        self.log_text.configure(state="disabled")

    def _get_clean_dtype(self):
        # Remove emojis from string
        raw = self.data_type_var.get()
        if "Entier" in raw: return "Entier"
        if "Float" in raw: return "Float"
        if "Caractère" in raw: return "Caractère"
        return "Chaîne"

    def _parse_val(self, txt):
        dt = self._get_clean_dtype()
        txt = txt.strip()
        if not txt: raise ValueError("Vide")
        try:
            if dt == "Entier": return int(txt)
            if dt == "Float": return float(txt)
            return txt # Char/String
        except: raise ValueError(f"Format invalide pour {dt}")

    def _on_tree_type_change(self, choice):
        if "BST" in self.tree_var.get():
            self.current_tree_type = 'BST'
            self.current_tree = self.bst
            
            # UI visibility
            self.lbl_insert_parent.pack_forget()
            self.entry_insert_parent.pack_forget()
            
            self.box_degree.pack_forget()
            self.btn_convert.pack_forget()
        else:
            self.current_tree_type = 'N-ary'
            self.current_tree = self.nary
            
            # UI visibility
            self.lbl_insert_parent.pack(fill="x", padx=5, before=self.lbl_new_val)
            self.entry_insert_parent.pack(fill="x", padx=5, pady=2, before=self.lbl_new_val)
            
            self.box_degree.pack(fill="x", pady=2, before=self.fr_manual.winfo_children()[1])
            self.btn_convert.pack(fill="x", padx=5, pady=5)
            
        self._refresh_viz()
        self._toggle_save_button()

    def _on_input_mode_change(self):
        v = self.input_mode_var.get()
        if v == 'Manuelle':
            self.fr_random.pack_forget()
            self.fr_manual.pack(fill="x", padx=5)
        else:
            self.fr_manual.pack_forget()
            self.fr_random.pack(fill="x", padx=5)

    def _on_trav_change(self, choice):
        # Disable method if BFS
        t = self.trav_type_var.get()
        if "Largeur" in t:
            self.combo_trav_method.configure(state="disabled")
        else:
            self.combo_trav_method.configure(state="readonly")
            
    # Actions
    def _on_insert(self):
        try:
            val = self._parse_val(self.val_var.get())
            if self.current_tree_type == 'BST':
                self.current_tree.insert(val)
            else:
                p_txt = self.insert_parent_var.get()
                if not p_txt and not self.current_tree.root:
                    self.current_tree.insert(val) # Root
                else:
                    pval = self._parse_val(p_txt)
                    self.current_tree.insert(val, pval)
            self._refresh_viz()
            self._toggle_save_button()
        except Exception as e: messagebox.showerror("Erreur", str(e))

    def _on_delete(self):
        try:
            val = self._parse_val(self.val_var.get())
            self.current_tree.delete(val)
            self._refresh_viz()
            self._toggle_save_button()
        except Exception as e: messagebox.showerror("Erreur", str(e))

    def _on_modify(self):
        try:
            old = self._parse_val(self.val_var.get())
            new = self._parse_val(self.new_val_var.get())
            self.current_tree.modify(old, new)
            self._refresh_viz()
            self._toggle_save_button()
        except Exception as e: messagebox.showerror("Erreur", str(e))

    def _on_clear(self):
        if self.current_tree_type == 'BST':
            self.bst = BST(); self.current_tree = self.bst
        else:
            self.nary = NaryTree(); self.current_tree = self.nary
        self.current_tree.clear_logs()
        self._refresh_viz()
        self._log("Arbre vidé")
        self._toggle_save_button()

    def _on_convert(self):
        if self.current_tree_type == 'N-ary':
            self.bst = self.nary.to_binary()
            self.tree_var.set('🌳 BST (Binaire)')
            self._on_tree_type_change(None)
            self._log("Converti N-aire -> Binaire")

    def _on_execute_trav(self):
        t = self.trav_type_var.get()
        m = self.trav_method_var.get()
        res = []
        name = ""
        
        if "Largeur" in t:
            res = self.current_tree.traverse_bfs()
            name = "Largeur (BFS)"
        else:
            if "Pré-ordre" in m: res = self.current_tree.traverse_preorder(); name = "Pré-ordre"
            elif "In-ordre" in m: res = self.current_tree.traverse_inorder(); name = "In-ordre"
            else: res = self.current_tree.traverse_postorder(); name = "Post-ordre"
            
        self.trav_res_var.set(f"{name}: {str(res)}")
        self._log(f"{name}: {str(res)}")

    def _on_generate(self):
        try:
            n = int(self.rand_size_var.get())
            self._on_clear()
            
            # Generate values
            dt = self._get_clean_dtype()
            vals = []
            if dt == "Entier": vals = random.sample(range(1, 1000), n)
            elif dt == "Float": vals = [round(random.uniform(1,100),2) for _ in range(n)]
            elif dt == "Caractère": vals = random.sample(string.ascii_letters, n)
            else: vals = ["".join(random.choices(string.ascii_letters, k=3)) for _ in range(n)]

            if self.current_tree_type == 'BST':
                for v in vals: self.current_tree.insert(v)
            else:
                if vals:
                    self.current_tree.insert(vals[0]) # Root
                    existing = [vals[0]]
                    for i in range(1, len(vals)):
                        p = random.choice(existing)
                        self.current_tree.insert(vals[i], p)
                        existing.append(vals[i])
            self._refresh_viz()
            self._toggle_save_button()
            self._log(f"Généré {n} noeuds ({dt})")
        except Exception as e: messagebox.showerror("Erreur", str(e), parent=self)


    def _on_save(self):
        """Ouvre une boîte de dialogue pour enregistrer l'arbre sous forme de fichier JSON."""
        if not self.current_tree or not self.current_tree.root:
            messagebox.showwarning("Alerte", "Aucune donnée d'arbre à enregistrer.", parent=self)
            return

        # Définition du nom par défaut et des types de fichiers
        initial_name = f"{self.current_tree_type}_export.json"
        
        path = filedialog.asksaveasfilename(
            defaultextension=".json",
            initialfile=initial_name,
            filetypes=[("Fichiers JSON", "*.json"), ("Tous les fichiers", "*.*")],
            parent=self
        )
        
        if path:
            try:
                # La classe d'arbre DOIT avoir une méthode to_dict()
                tree_data = {
                    "type": self.current_tree_type,
                    "data_type": self._get_clean_dtype(),
                    "degree": self.nary_degree_var.get() if self.current_tree_type == 'N-ary' else None,
                    "tree_structure": self.current_tree.to_dict() 
                }
                
                with open(path, "w", encoding="utf-8") as f:
                    json.dump(tree_data, f, indent=4)
                    
                self._log(f"Arbre enregistré dans : {os.path.basename(path)}")
                messagebox.showinfo("Succès", f"Arbre enregistré dans :\n{path}", parent=self)
            except Exception as e:
                self._log(f"Erreur d'enregistrement : {e}")
                messagebox.showerror("Erreur d'enregistrement", f"Une erreur s'est produite lors de l'enregistrement : {e}", parent=self)

                

    def _open_manual_dialog(self):
        d = ctk.CTkToplevel(self)
        d.title("Saisie Manuelle")
        d.geometry("400x150")
        d.transient(self)
        d.grab_set() # Make it modal
        
        # Le label est mis à jour pour indiquer la séparation par espace
        ctk.CTkLabel(d, text="Entrez les valeurs (séparées par des espaces):").pack(pady=10)
        e = ctk.CTkEntry(d, width=300)
        e.pack(pady=5)
        e.focus_force() # Focus input
        
        def ok():
            txt = e.get()
            try:
                # MODIFICATION ICI : Utiliser txt.split() pour séparer par un espace
                raws = [x.strip() for x in txt.split() if x.strip()]
                vals = []
                dt = self._get_clean_dtype()
                for r in raws:
                    if dt == "Entier": vals.append(int(r))
                    elif dt == "Float": vals.append(float(r))
                    else: vals.append(r)
                
                if not vals: return
                self._on_clear()
                
                if self.current_tree_type == 'BST':
                    for v in vals: self.current_tree.insert(v)
                else:
                    # Logic from C: 
                    # If i=0 -> root
                    # Else parent_idx = (i-1) / degree
                    deg = int(self.nary_degree_var.get())
                    self.current_tree.insert(vals[0])
                    for i in range(1, len(vals)):
                        pidx = (i - 1) // deg
                        if pidx < len(vals):
                            self.current_tree.insert(vals[i], vals[pidx])
                
                self._refresh_viz()
                self._log(f"Importé {len(vals)} valeurs")
                self._toggle_save_button()
                d.destroy()
            except Exception as ex: messagebox.showerror("Erreur", str(ex), parent=d)
            
        ctk.CTkButton(d, text="OK", command=ok).pack(pady=10)
        
    # --- Draw ---
    def _get_bst_pos(self, root):
        pos = {}; inorder = []
        def tr(n, d):
            if not n: return
            tr(n.left, d+1); inorder.append((n, d)); tr(n.right, d+1)
        tr(root, 0)
        for i, (n, d) in enumerate(inorder): pos[n.id] = (float(i), -float(d))
        return pos

    def _get_nary_pos_robust(self, root):
        pos = {}
        # We need to track the next available X coordinate for leaves to ensure no overlap
        # Using a list to pass by reference
        next_x = [0.0]
        
        def traverse(node, depth):
            if not node: return
            
            # Post-order traversal: process children first
            if not node.children:
                # Leaf node
                node_x = next_x[0]
                pos[node.id] = (node_x, -float(depth))
                next_x[0] += 1.0 # Advance cursor
            else:
                child_xs = []
                for child in node.children:
                    traverse(child, depth + 1)
                    if child.id in pos:
                        child_xs.append(pos[child.id][0])
                
                # Check if we have children results (should always be true if children list wasn't empty)
                if child_xs:
                    # Center parent over children
                    min_c = min(child_xs)
                    max_c = max(child_xs)
                    center_x = (min_c + max_c) / 2.0
                    
                    # Ensure parent doesn't overlap with existing nodes to the left (rare in this logic but good for safety)
                    # Because leaves drive the width, the parent will usually be in safe bounds.
                    # One potential issue: if this subtree is very narrow but 'next_x' was advanced further by a prior sibling traversal?
                    # Actually, since we traverse children in order, next_x is always "to the right" of the last placed node.
                    # So we don't need extra checks for simple trees.
                    
                    pos[node.id] = (center_x, -float(depth))
        
        traverse(root, 0)
        return pos
        
        
    def _refresh_viz(self):
        self.ax.clear()
        
        t = self.current_tree
        if not t.root:
            self.ax.text(0.5, 0.5, "Arbre Vide", ha='center', va='center')
            self.ax.axis('off')
            self.canvas_agg.draw()
            return

        size = t.get_size(t.root)
        height = t.get_height(t.root)
        self.stats_var.set(f"Taille: {size} | Hauteur: {height}")
        
        G = t.to_networkx()
        
        try:
            if self.current_tree_type == 'BST':
                pos = self._get_bst_pos(t.root)
            else:
                pos = self._get_nary_pos_robust(t.root)
                
            labels = nx.get_node_attributes(G, 'label')
            
            # Draw
            node_color = 'lightblue' if self.current_tree_type == 'BST' else 'lightgreen'
            nx.draw(G, pos, with_labels=True, labels=labels, ax=self.ax,
                    node_size=800, node_color=node_color, font_size=9, font_weight='bold', 
                    arrows=False, edge_color='#666666')
            
            # Resize logic
            xs = [p[0] for p in pos.values()]
            ys = [p[1] for p in pos.values()]
            if xs:
                wd_u = max(xs) - min(xs)
                ht_u = max(ys) - min(ys)
                
                # Dynamic sizing
                # Add margin
                w_in = max(6.0, (wd_u + 1) * 0.7)
                h_in = max(5.0, (ht_u + 1) * 0.9)
                
                self.fig.set_size_inches(w_in, h_in)
                self.ax.set_xlim(min(xs)-0.5, max(xs)+0.5)
                self.ax.set_ylim(min(ys)-0.5, max(ys)+0.5)
                
        except Exception as e:
            self.ax.text(0.5, 0.5, str(e), color='red')
            
        self.ax.axis('off')
        self.fig.tight_layout(pad=0.2)
        self.canvas_agg.draw()
        
        # Scroll update
        self.canvas_widget.update_idletasks()
        w = int(self.fig.get_figwidth() * self.fig.dpi)
        h = int(self.fig.get_figheight() * self.fig.dpi)
        self.canvas_widget.config(width=w, height=h)
        self.scroll_canvas.config(scrollregion=(0,0,w,h))


# In[7]:




# --- CLASSE GRAPH STRUCTURE ---
INFINITY_VAL = float('inf')

class GraphStructure:
    def __init__(self, is_directed=True, data_type='string'):
        self.is_directed = is_directed
        self.data_type = data_type
        self.nodes = []
        self.adj = {}    # {source: {dest: weight}}
        self.positions = {}
    
    def add_node(self, node_name: str, position: Tuple[int, int] = None):
        if node_name not in self.nodes:
            self.nodes.append(node_name)
            self.adj[node_name] = {}
            if position:
                self.positions[node_name] = position

    def delete_node(self, node_name: str) -> bool:
        if node_name not in self.nodes:
            return False
        self.nodes.remove(node_name)
        if node_name in self.positions:
            del self.positions[node_name]
        if node_name in self.adj:
            del self.adj[node_name]
        for source in self.nodes:
            if node_name in self.adj[source]:
                del self.adj[source][node_name]
        return True
    
    def add_edge(self, source: str, dest: str, weight: float):
        if source not in self.nodes: self.add_node(source)
        if dest not in self.nodes: self.add_node(dest)
        
        self.adj[source][dest] = weight
        if not self.is_directed:
            self.adj[dest][source] = weight
            
    def get_nodes(self) -> List[str]:
        return self.nodes.copy()
    
    def get_edges(self) -> List[Tuple[str, str, float]]:
        edges = []
        for source in self.nodes:
            for dest, weight in self.adj.get(source, {}).items():
                edges.append((source, dest, weight))
        return edges

    def set_position(self, node: str, x: int, y: int):
        self.positions[node] = (x, y)
    
    def get_position(self, node: str) -> Optional[Tuple[int, int]]:
        return self.positions.get(node)

    # --- Algorithmes ---
    def dijkstra(self, start_node: str):
        if start_node not in self.nodes: return {}, {}, "Erreur: Nœud invalide"
        for s in self.nodes:
            for d, w in self.adj.get(s, {}).items():
                if w < 0: return {}, {}, "Erreur: Poids négatifs interdits pour Dijkstra"

        distances = {node: INFINITY_VAL for node in self.nodes}
        predecessors = {node: None for node in self.nodes}
        distances[start_node] = 0
        pq = [(0, start_node)]
        
        while pq:
            current_dist, current_node = heapq.heappop(pq)
            if current_dist > distances[current_node]: continue
            
            for neighbor, weight in self.adj.get(current_node, {}).items():
                distance = current_dist + weight
                if distance < distances[neighbor]:
                    distances[neighbor] = distance
                    predecessors[neighbor] = current_node
                    heapq.heappush(pq, (distance, neighbor))
                    
        return distances, predecessors, "Succès"

    def bellman_ford(self, start_node: str):
        if start_node not in self.nodes: return {}, {}, "Erreur: Nœud invalide"
        distances = {node: INFINITY_VAL for node in self.nodes}
        predecessors = {node: None for node in self.nodes}
        distances[start_node] = 0
        
        for _ in range(len(self.nodes) - 1):
            for u in self.nodes:
                for v, w in self.adj.get(u, {}).items():
                    if distances[u] != INFINITY_VAL and distances[u] + w < distances[v]:
                        distances[v] = distances[u] + w
                        predecessors[v] = u
                        
        for u in self.nodes:
            for v, w in self.adj.get(u, {}).items():
                if distances[u] != INFINITY_VAL and distances[u] + w < distances[v]:
                    return {}, {}, "Erreur: Cycle négatif détecté"
                    
        return distances, predecessors, "Succès"

    def floyd_warshall(self):
        n = len(self.nodes)
        if n == 0: return [], [], "Erreur: Graphe vide"
        dist = [[INFINITY_VAL] * n for _ in range(n)]
        pred = [[None] * n for _ in range(n)]
        
        node_map = {node: i for i, node in enumerate(self.nodes)}
        
        for i in range(n): dist[i][i] = 0
        
        for u in self.nodes:
            for v, w in self.adj.get(u, {}).items():
                i, j = node_map[u], node_map[v]
                dist[i][j] = w
                pred[i][j] = u
                
        for k in range(n):
            for i in range(n):
                for j in range(n):
                    if dist[i][k] != INFINITY_VAL and dist[k][j] != INFINITY_VAL:
                        if dist[i][k] + dist[k][j] < dist[i][j]:
                            dist[i][j] = dist[i][k] + dist[k][j]
                            pred[i][j] = pred[k][j]
                            
        for i in range(n):
            if dist[i][i] < 0: return [], [], "Erreur: Cycle négatif"
            
        return dist, pred, "Succès"

    def reconstruct_path(self, predecessors, start, end):
        if start == end: return [start]
        if predecessors.get(end) is None: return []
        path = []
        curr = end
        while curr is not None and curr != start:
            path.append(curr)
            curr = predecessors.get(curr)
        if curr == start:
            path.append(start)
            return path[::-1]
        return []

    def to_networkx(self):
        # Utilisation de nx.Graph() (non orienté) si is_directed est False
        G = nx.DiGraph() if self.is_directed else nx.Graph()
        for n in self.nodes: G.add_node(n)
        for u, neighbors in self.adj.items():
            for v, w in neighbors.items():
                G.add_edge(u, v, weight=w, label=f"{w:.1f}")
        return G
        
    def generate_node_names(self, count, dtype):
        if dtype == 'int': return [str(i) for i in range(1, count+1)]
        if dtype == 'float': return [f"{i}.0" for i in range(1, count+1)]
        if dtype == 'char': return [chr(65+i) for i in range(count)]
        return [f"S{i}" for i in range(1, count+1)]

# --- INTERFACE GRAPHIQUE ---
class GraphWindow(ctk.CTkToplevel):
    def __init__(self, master=None):
        super().__init__(master)
        
        self.title("Visualisation de Graphes")
        self.geometry("1100x800")
        self.resizable(True, True)
        
        # Correction z-order
        # Correction z-order (Robust)
        self.after(200, lambda: self.lift())
        self.after(200, lambda: self.focus_force())
        self.after(200, lambda: self.grab_set())
        
        # --- Variables ---
        self.data_type_var = tk.StringVar(value="Caractère")
        self.num_nodes_var = tk.StringVar(value="5")
        self.path_type_var = tk.StringVar(value="Orienté") # Nouveau
        self.algo_var = tk.StringVar(value="Dijkstra")
        self.start_node_var = tk.StringVar()
        self.end_node_var = tk.StringVar()
        self.result_var = tk.StringVar(value="En attente...") # Label résultat
        
        self.current_graph = None
        self.selected_node = None 
        self.highlighted_path = [] 
        self.pos = {} 
        self.drag_start = None # Pour le drag and drop d'arête
        self.drag_current = None

        self._setup_ui()
        
    def _setup_ui(self):
        # Layout principal: Box horizontale
        self.grid_columnconfigure(0, weight=0, minsize=320) # Panneau gauche fixe
        self.grid_columnconfigure(1, weight=1) # Zone dessin
        self.grid_rowconfigure(0, weight=1)

        # === PANNEAU GAUCHE ===
        # Scrollable frame pour s'assurer que tout rentre
        left_scroll = ctk.CTkScrollableFrame(self, label_text="Configuration", width=300)
        left_scroll.grid(row=0, column=0, padx=10, pady=10, sticky="nsew")
        
        # 1. Type de données
        ctk.CTkLabel(left_scroll, text="Type de données:", anchor="w").pack(fill="x", padx=5, pady=(5,0))
        self.combo_dtype = ctk.CTkComboBox(left_scroll, variable=self.data_type_var, 
                                           values=["Entier", "Réel", "Caractère", "Chaîne"])
        self.combo_dtype.pack(fill="x", padx=5, pady=5)
        
        # 2. Type de chemin (Nouveau)
        ctk.CTkLabel(left_scroll, text="Type de chemin:", anchor="w").pack(fill="x", padx=5, pady=(5,0))
        self.combo_path_type = ctk.CTkComboBox(left_scroll, variable=self.path_type_var,
                                               values=["Orienté", "Non orienté"],
                                               command=self._on_setting_changed)
        self.combo_path_type.pack(fill="x", padx=5, pady=5)

        # 3. Nombre de sommets
        ctk.CTkLabel(left_scroll, text="Nombre de sommets:", anchor="w").pack(fill="x", padx=5, pady=(5,0))
        # Spinbox simulé avec Entry + Validation
        self.entry_nodes = ctk.CTkEntry(left_scroll, textvariable=self.num_nodes_var)
        self.entry_nodes.pack(fill="x", padx=5, pady=5)

        # 4. Bouton Dessiner (Bleu / Success)
        self.btn_draw = ctk.CTkButton(left_scroll, text="🎨 Dessiner Graphe", command=self._on_init_graph, 
                                      fg_color="#3498db", hover_color="#2980b9")
        self.btn_draw.pack(fill="x", padx=5, pady=10)

        # 5. Bouton Vider (Rouge / Destructive)
        self.btn_clear = ctk.CTkButton(left_scroll, text="🗑️ Vider Graphe", command=self._on_clear_graph,
                                       fg_color="#e74c3c", hover_color="#c0392b")
        self.btn_clear.pack(fill="x", padx=5, pady=5)

        # Separator 1
        ttk.Separator(left_scroll, orient='horizontal').pack(fill='x', pady=10)

        # 6. Sommet Départ
        ctk.CTkLabel(left_scroll, text="Sommet de départ:", anchor="w").pack(fill="x", padx=5, pady=(5,0))
        self.menu_start = ctk.CTkOptionMenu(left_scroll, variable=self.start_node_var, dynamic_resizing=False)
        self.menu_start.pack(fill="x", padx=5, pady=5)

        # 7. Sommet Arrivée
        ctk.CTkLabel(left_scroll, text="Sommet d'arrivée:", anchor="w").pack(fill="x", padx=5, pady=(5,0))
        self.menu_end = ctk.CTkOptionMenu(left_scroll, variable=self.end_node_var, dynamic_resizing=False)
        self.menu_end.pack(fill="x", padx=5, pady=5)

        # 8. Algorithme
        ctk.CTkLabel(left_scroll, text="Algorithme:", anchor="w").pack(fill="x", padx=5, pady=(5,0))
        self.combo_algo = ctk.CTkComboBox(left_scroll, variable=self.algo_var,
                                          values=["Dijkstra", "Bellman-Ford", "Floyd-Warshall"])
        self.combo_algo.pack(fill="x", padx=5, pady=5)

        # 9. Bouton Exécuter (Violet/Accent dans le C, ici on met Vert comme planifié ou Accent ?)
        # C code says: gtk_widget_add_css_class(data->execute_btn, "accent-btn"); which is likely purple or blue.
        # But let's stick to a distinct color. 
        self.btn_run = ctk.CTkButton(left_scroll, text="🚀 Exécuter Algorithme", command=self._on_run_algo,
                                     fg_color="#9b59b6", hover_color="#8e44ad") # Violet-ish like C accent
        self.btn_run.pack(fill="x", padx=5, pady=10)

        # Separator 2
        ttk.Separator(left_scroll, orient='horizontal').pack(fill='x', pady=10)

        # 10. Bouton Retour
        self.btn_back = ctk.CTkButton(left_scroll, text="🔙 Retour Menu", command=self.destroy,
                                      fg_color="gray", hover_color="#555555")
        self.btn_back.pack(fill="x", padx=5, pady=5)

        # Separator 3
        ttk.Separator(left_scroll, orient='horizontal').pack(fill='x', pady=10)

        # 11. Résultats
        ctk.CTkLabel(left_scroll, text="Résultat:", font=("Arial", 14, "bold"), anchor="w").pack(fill="x", padx=5)
        self.lbl_result = ctk.CTkLabel(left_scroll, textvariable=self.result_var, wraplength=280, justify="left", anchor="nw")
        self.lbl_result.pack(fill="x", padx=5, pady=5)


        # === PANNEAU DROIT (VISU) ===
        right_frame = ctk.CTkFrame(self, fg_color="transparent")
        right_frame.grid(row=0, column=1, padx=10, pady=10, sticky="nsew")
        right_frame.grid_rowconfigure(0, weight=1)
        right_frame.grid_columnconfigure(0, weight=1)
        
        self.fig = plt.Figure(figsize=(6,6), dpi=100)
        self.ax = self.fig.add_subplot(111)
        
        self.canvas = FigureCanvasTkAgg(self.fig, master=right_frame)
        self.canvas_widget = self.canvas.get_tk_widget()
        self.canvas_widget.grid(row=0, column=0, sticky="nsew")
        
        # Events Matplotlib pour interactivité (Click & Drag pour arêtes)
        self.fig.canvas.mpl_connect('button_press_event', self._on_press)
        self.fig.canvas.mpl_connect('motion_notify_event', self._on_motion)
        self.fig.canvas.mpl_connect('button_release_event', self._on_release)
        
    # --- Interactivité (Drag & Drop Arêtes) ---
    def _get_node_at(self, x, y):
        if not self.pos: return None
        # Rayon approx en coordonnées data. 
        # C'est un peu tricky avec mpl, on assume threshold simple distance euclidienne
        threshold = 0.15 
        best_node = None
        min_dist = float('inf')
        for node, (nx, ny) in self.pos.items():
            d = math.sqrt((nx-x)**2 + (ny-y)**2)
            if d < threshold and d < min_dist:
                min_dist = d
                best_node = node
        return best_node

    def _on_press(self, event):
        if event.inaxes != self.ax or not self.current_graph: return
        node = self._get_node_at(event.xdata, event.ydata)
        if node:
            self.drag_start = node
            self.drag_current = (event.xdata, event.ydata)
            self._redraw_canvas_only() # Juste refresh visuel pour le trait

    def _on_motion(self, event):
        if self.drag_start and event.inaxes == self.ax:
            self.drag_current = (event.xdata, event.ydata)
            self._redraw_canvas_only()

    def _on_release(self, event):
        if self.drag_start:
            target = None
            if event.inaxes == self.ax:
                target = self._get_node_at(event.xdata, event.ydata)
            
            if target and target != self.drag_start:
                # Création arête
                self._prompt_edge(self.drag_start, target)
            
            self.drag_start = None
            self.drag_current = None
            self._refresh_viz()

    def _prompt_edge(self, u, v):
        w = simpledialog.askfloat("Poids de l'arête", f"Entrez le poids pour {u} -> {v}:", parent=self, minvalue=-1000, maxvalue=1000)
        if w is not None:
            self.current_graph.add_edge(u, v, w)

    # --- Logique UI ---
    def _on_setting_changed(self, choice):
        if self.current_graph:
            is_directed = (self.path_type_var.get() == "Orienté")
            self.current_graph.is_directed = is_directed
            self._refresh_viz()

    def _on_clear_graph(self):
        self.current_graph = None
        self.pos = {}
        self.highlighted_path = []
        self.result_var.set("Graphe vidé.")
        self.menu_start.configure(values=[""])
        self.menu_end.configure(values=[""])
        self.start_node_var.set("")
        self.end_node_var.set("")
        self.ax.clear()
        self.canvas.draw()

    def _on_init_graph(self):
        try:
            n = int(self.num_nodes_var.get())
            if n < 2 or n > 26: raise ValueError
        except:
             messagebox.showerror("Erreur", "Nombre de sommets invalide (2-26)", parent=self)
             return

        is_directed = (self.path_type_var.get() == "Orienté")
        
        # MAPPING TYPES C -> Python
        dtype_str = self.data_type_var.get()
        mapping = {"Entier": "int", "Réel": "float", "Caractère": "char", "Chaîne": "string"}
        dtype = mapping.get(dtype_str, 'char')

        self.current_graph = GraphStructure(is_directed, dtype)
        names = self.current_graph.generate_node_names(n, dtype)
        for name in names:
            self.current_graph.add_node(name)
        
        self.highlighted_path = []
        self.pos = {} # Reset pos layout
        
        # Update Menus
        self.menu_start.configure(values=names)
        self.menu_end.configure(values=names)
        if names:
            self.start_node_var.set(names[0])
            self.end_node_var.set(names[-1])
            
        self.result_var.set("Cliquez et faites glisser entre deux nœuds pour créer une arête.")
        self._refresh_viz()

    def _on_run_algo(self):
        if not self.current_graph: return
        s = self.start_node_var.get()
        e = self.end_node_var.get()
        algo = self.algo_var.get()
        
        path = []
        res_txt = ""
        
        if algo == "Dijkstra":
            dist, pred, msg = self.current_graph.dijkstra(s)
            if msg == "Succès":
                d = dist.get(e, INFINITY_VAL)
                if d == INFINITY_VAL: res_txt = "Pas de chemin."
                else: 
                    path = self.current_graph.reconstruct_path(pred, s, e)
                    res_txt = f"Distance: {d}\nChemin: {' -> '.join(path)}"
            else: res_txt = msg

        elif algo == "Bellman-Ford":
            dist, pred, msg = self.current_graph.bellman_ford(s)
            if msg == "Succès":
                d = dist.get(e, INFINITY_VAL)
                if d == INFINITY_VAL: res_txt = "Pas de chemin."
                else:
                    path = self.current_graph.reconstruct_path(pred, s, e)
                    res_txt = f"Distance: {d}\nChemin: {' -> '.join(path)}"
            else: res_txt = msg

        elif algo == "Floyd-Warshall":
            dmat, pmat, msg = self.current_graph.floyd_warshall()
            if msg == "Succès":
                nodes = self.current_graph.get_nodes()
                try:
                    i, j = nodes.index(s), nodes.index(e)
                    d = dmat[i][j]
                    if d == INFINITY_VAL: res_txt = "Pas de chemin."
                    else:
                        # Reconstruct Floyd
                        c_temp = j
                        path = [e]
                        while c_temp != i:
                            prev = pmat[i][c_temp]
                            if prev is None: break
                            path.append(prev)
                            c_temp = nodes.index(prev)
                        path.append(s)
                        path = path[::-1] # Reverse
                        res_txt = f"Distance: {d}\nChemin: {' -> '.join(path)}"
                except: res_txt = "Erreur index FW"
            else: res_txt = msg

        self.highlighted_path = path
        self.result_var.set(res_txt)
        self._refresh_viz()

    def _refresh_viz(self):
        self.ax.clear()
        if not self.current_graph: 
            self.ax.axis('off')
            self.canvas.draw()
            return
            
        G = self.current_graph.to_networkx()
        
        if not self.pos or set(self.pos.keys()) != set(G.nodes()):
            self.pos = nx.circular_layout(G)
        
        # Couleurs
        node_colors = []
        start_n = self.start_node_var.get()
        end_n = self.end_node_var.get()
        
        for n in G.nodes():
            if n in self.highlighted_path:
                if n == start_n: node_colors.append('#2ecc71') # Vert
                elif n == end_n: node_colors.append('#e67e22') # Orange
                else: node_colors.append('#f1c40f') # Jaune
            else:
                node_colors.append('#9b59b6') # Violet (Comme C)
        
        # Arêtes
        edge_colors = []
        widths = []
        path_edges = []
        if len(self.highlighted_path) > 1:
            for k in range(len(self.highlighted_path)-1):
                u, v = self.highlighted_path[k], self.highlighted_path[k+1]
                path_edges.append((u, v))
                if not self.current_graph.is_directed:
                    path_edges.append((v, u))

        for u, v in G.edges():
            if (u,v) in path_edges:
                edge_colors.append('#e74c3c') # Rouge
                widths.append(2.5)
            else:
                edge_colors.append('gray')
                widths.append(1.0)

        # Dessin
        # draw_networkx is higher level, but allows less control if mixing arrows.
        # Use separating calls
        nx.draw_networkx_nodes(G, self.pos, ax=self.ax, node_color=node_colors, node_size=600, edgecolors='black')
        nx.draw_networkx_labels(G, self.pos, ax=self.ax, font_color='white', font_weight='bold')
        
        # Edges
        # arrows=True only works for Directed graphs in nx defaults
        nx.draw_networkx_edges(G, self.pos, ax=self.ax, edge_color=edge_colors, width=widths, 
                               arrows=self.current_graph.is_directed, arrowstyle='-|>', arrowsize=20)
        
        edge_labels = nx.get_edge_attributes(G, 'label')
        nx.draw_networkx_edge_labels(G, self.pos, edge_labels=edge_labels, ax=self.ax)
        
        # Temp draw drag line
        if self.drag_start and self.drag_current:
            sx, sy = self.pos[self.drag_start]
            ex, ey = self.drag_current
            self.ax.plot([sx, ex], [sy, ey], color='yellow', linestyle='--', linewidth=2)

        self.ax.axis('off')
        self.canvas.draw()
    
    def _redraw_canvas_only(self):
        # Optimisation pour le drag: ne redessine que si nécessaire ou pardessus
        # Matplotlib n'est pas performant pour du 60fps interactif via blit ici complexe
        # On refait refresh pour simplicité (sur petit graphe ça passe)
        self._refresh_viz()


# In[8]:




# --- CLASSE GRAPH STRUCTURE ---
INFINITY_VAL = float('inf')

class GraphStructure:
    def __init__(self, is_directed=True, data_type='string'):
        self.is_directed = is_directed
        self.data_type = data_type
        self.nodes = []
        self.adj = {}    # {source: {dest: weight}}
        self.positions = {}
    
    def add_node(self, node_name: str, position: Tuple[int, int] = None):
        if node_name not in self.nodes:
            self.nodes.append(node_name)
            self.adj[node_name] = {}
            if position:
                self.positions[node_name] = position

    def to_dict(self):
        """Convertit l'état du graphe en un dictionnaire pour la sérialisation JSON."""
        return {
            "is_directed": self.is_directed,
            "data_type": self.data_type,
            "nodes": self.nodes,
            "adj": self.adj,
            "positions": self.positions # Nécessaire pour conserver le layout si sauvegardé
        }



    def validate_type(self, value: str) -> bool:
        try:
            if self.data_type == 'int':
                int(value)
            elif self.data_type == 'float':
                float(value)
            elif self.data_type == 'char':
                if len(value) != 1: return False
            return True
        except ValueError:
            return False
    
    
    def delete_node(self, node_name: str) -> bool:
        if node_name not in self.nodes:
            return False
        self.nodes.remove(node_name)
        if node_name in self.positions:
            del self.positions[node_name]
        if node_name in self.adj:
            del self.adj[node_name]
        for source in self.nodes:
            if node_name in self.adj[source]:
                del self.adj[source][node_name]
        return True
    
    def add_edge(self, source: str, dest: str, weight: float):
        if source not in self.nodes: self.add_node(source)
        if dest not in self.nodes: self.add_node(dest)
        
        self.adj[source][dest] = weight
        if not self.is_directed:
            self.adj[dest][source] = weight
            
    def get_nodes(self) -> List[str]:
        return self.nodes.copy()
    
    def get_edges(self) -> List[Tuple[str, str, float]]:
        edges = []
        for source in self.nodes:
            for dest, weight in self.adj.get(source, {}).items():
                edges.append((source, dest, weight))
        return edges

    def set_position(self, node: str, x: int, y: int):
        self.positions[node] = (x, y)
    
    def get_position(self, node: str) -> Optional[Tuple[int, int]]:
        return self.positions.get(node)

    # --- Algorithmes ---
    def dijkstra(self, start_node: str):
        if start_node not in self.nodes: return {}, {}, "Erreur: Nœud invalide"
        for s in self.nodes:
            for d, w in self.adj.get(s, {}).items():
                if w < 0: return {}, {}, "Erreur: Poids négatifs interdits pour Dijkstra"

        distances = {node: INFINITY_VAL for node in self.nodes}
        predecessors = {node: None for node in self.nodes}
        distances[start_node] = 0
        pq = [(0, start_node)]
        
        while pq:
            current_dist, current_node = heapq.heappop(pq)
            if current_dist > distances[current_node]: continue
            
            for neighbor, weight in self.adj.get(current_node, {}).items():
                distance = current_dist + weight
                if distance < distances[neighbor]:
                    distances[neighbor] = distance
                    predecessors[neighbor] = current_node
                    heapq.heappush(pq, (distance, neighbor))
                    
        return distances, predecessors, "Succès"

    def bellman_ford(self, start_node: str):
        if start_node not in self.nodes: return {}, {}, "Erreur: Nœud invalide"
        distances = {node: INFINITY_VAL for node in self.nodes}
        predecessors = {node: None for node in self.nodes}
        distances[start_node] = 0
        
        for _ in range(len(self.nodes) - 1):
            for u in self.nodes:
                for v, w in self.adj.get(u, {}).items():
                    if distances[u] != INFINITY_VAL and distances[u] + w < distances[v]:
                        distances[v] = distances[u] + w
                        predecessors[v] = u
                        
        for u in self.nodes:
            for v, w in self.adj.get(u, {}).items():
                if distances[u] != INFINITY_VAL and distances[u] + w < distances[v]:
                    return {}, {}, "Erreur: Cycle négatif détecté"
                    
        return distances, predecessors, "Succès"

    def floyd_warshall(self):
        n = len(self.nodes)
        if n == 0: return [], [], "Erreur: Graphe vide"
        dist = [[INFINITY_VAL] * n for _ in range(n)]
        pred = [[None] * n for _ in range(n)]
        
        node_map = {node: i for i, node in enumerate(self.nodes)}
        
        for i in range(n): dist[i][i] = 0
        
        for u in self.nodes:
            for v, w in self.adj.get(u, {}).items():
                i, j = node_map[u], node_map[v]
                dist[i][j] = w
                pred[i][j] = u
                
        for k in range(n):
            for i in range(n):
                for j in range(n):
                    if dist[i][k] != INFINITY_VAL and dist[k][j] != INFINITY_VAL:
                        if dist[i][k] + dist[k][j] < dist[i][j]:
                            dist[i][j] = dist[i][k] + dist[k][j]
                            pred[i][j] = pred[k][j]
                            
        for i in range(n):
            if dist[i][i] < 0: return [], [], "Erreur: Cycle négatif"
            
        return dist, pred, "Succès"

    def reconstruct_path(self, predecessors, start, end):
        if start == end: return [start]
        if predecessors.get(end) is None: return []
        path = []
        curr = end
        while curr is not None and curr != start:
            path.append(curr)
            curr = predecessors.get(curr)
        if curr == start:
            path.append(start)
            return path[::-1]
        return []

    def to_networkx(self):
        # Utilisation de nx.Graph() (non orienté) si is_directed est False
        G = nx.DiGraph() if self.is_directed else nx.Graph()
        for n in self.nodes: G.add_node(n)
        for u, neighbors in self.adj.items():
            for v, w in neighbors.items():
                G.add_edge(u, v, weight=w, label=f"{w:.1f}")
        return G
        
    def generate_node_names(self, count, dtype):
        if dtype == 'int': return [str(i) for i in range(1, count+1)]
        if dtype == 'float': return [f"{i}.0" for i in range(1, count+1)]
        if dtype == 'char': return [chr(65+i) for i in range(count)]
        return [f"S{i}" for i in range(1, count+1)]

# --- INTERFACE GRAPHIQUE ---
class GraphWindow(ctk.CTkToplevel):
    def __init__(self, master=None):
        super().__init__(master)
        
        self.title("Visualisation de Graphes")
        self.geometry("1100x800")
        self.resizable(True, True)
        
        # Correction z-order (Nuclear Option with Topmost)
        self.attributes("-topmost", True)
        self.after(100, lambda: self.lift())
        self.after(100, lambda: self.focus_force())
        self.after(100, lambda: self.grab_set())
        # Enlever topmost après 500ms pour ne pas bloquer les autres apps utilisateur
        self.after(500, lambda: self.attributes("-topmost", False))

        self.fill_mode_var = tk.StringVar(value="Aléatoire")

        
        # --- Variables ---
        self.data_type_var = tk.StringVar(value="Caractère")
        self.num_nodes_var = tk.StringVar(value="5")
        self.path_type_var = tk.StringVar(value="Orienté") # Nouveau
        self.algo_var = tk.StringVar(value="Dijkstra")
        self.start_node_var = tk.StringVar()
        self.end_node_var = tk.StringVar()
        self.result_var = tk.StringVar(value="En attente...") # Label résultat
        
        self.current_graph = None
        self.selected_node = None 
        self.highlighted_path = [] 
        self.pos = {} 
        self.drag_start = None # Pour le drag and drop d'arête
        self.drag_current = None

        self._setup_ui()
        
    def _setup_ui(self):
        # Layout principal: Box horizontale
        self.grid_columnconfigure(0, weight=0, minsize=320) # Panneau gauche fixe
        self.grid_columnconfigure(1, weight=1) # Zone dessin
        self.grid_rowconfigure(0, weight=1)

        # === PANNEAU GAUCHE ===
        # Scrollable frame pour s'assurer que tout rentre
        left_scroll = ctk.CTkScrollableFrame(self, label_text="Configuration", width=300)
        left_scroll.grid(row=0, column=0, padx=10, pady=10, sticky="nsew")
        
        # 1. Type de données
        ctk.CTkLabel(left_scroll, text="Type de données:", anchor="w").pack(fill="x", padx=5, pady=(5,0))
        self.combo_dtype = ctk.CTkComboBox(left_scroll, variable=self.data_type_var, 
                                           values=["Entier", "Réel", "Caractère", "Chaîne"])
        self.combo_dtype.pack(fill="x", padx=5, pady=5)
        
        # 2. Type de chemin (Nouveau)
        ctk.CTkLabel(left_scroll, text="Type de chemin:", anchor="w").pack(fill="x", padx=5, pady=(5,0))
        self.combo_path_type = ctk.CTkComboBox(left_scroll, variable=self.path_type_var,
                                               values=["Orienté", "Non orienté"],
                                               command=self._on_setting_changed)
        self.combo_path_type.pack(fill="x", padx=5, pady=5)

        # --- MENU DE REMPLISSAGE ---
        ctk.CTkLabel(left_scroll, text="Mode de remplissage:", anchor="w").pack(fill="x", padx=5, pady=(10,0))
        self.seg_fill_mode = ctk.CTkSegmentedButton(left_scroll, values=["Aléatoire", "Manuel"], 
                                            variable=self.fill_mode_var)
        self.seg_fill_mode.pack(fill="x", padx=5, pady=5)

        # Champ dynamique (Nombre ou Valeurs)
        self.lbl_input_hint = ctk.CTkLabel(left_scroll, text="Nombre de sommets/saisie de valeurs", anchor="w")
        self.lbl_input_hint.pack(fill="x", padx=5, pady=(5,0))
        self.entry_nodes = ctk.CTkEntry(left_scroll, placeholder_text="Ex: 5 ou A,B,C")
        self.entry_nodes.pack(fill="x", padx=5, pady=5)

        # 4. Bouton Dessiner (Bleu / Success)
        self.btn_draw = ctk.CTkButton(left_scroll, text="🎨 Dessiner Graphe", command=self._on_init_graph, 
                                      fg_color="#3498db", hover_color="#2980b9")
        self.btn_draw.pack(fill="x", padx=5, pady=10)

        # 5. Bouton Vider (Rouge / Destructive)
        self.btn_clear = ctk.CTkButton(left_scroll, text="🗑️ Vider Graphe", command=self._on_clear_graph,
                                       fg_color="#e74c3c", hover_color="#c0392b")
        self.btn_clear.pack(fill="x", padx=5, pady=5)

        # Separator 1
        ttk.Separator(left_scroll, orient='horizontal').pack(fill='x', pady=10)


        # 6. Sommet Départ
        ctk.CTkLabel(left_scroll, text="Sommet de départ:", anchor="w").pack(fill="x", padx=5, pady=(5,0))
        self.menu_start = ctk.CTkOptionMenu(left_scroll, variable=self.start_node_var, dynamic_resizing=False)
        self.menu_start.pack(fill="x", padx=5, pady=5)

        # 7. Sommet Arrivée
        ctk.CTkLabel(left_scroll, text="Sommet d'arrivée:", anchor="w").pack(fill="x", padx=5, pady=(5,0))
        self.menu_end = ctk.CTkOptionMenu(left_scroll, variable=self.end_node_var, dynamic_resizing=False)
        self.menu_end.pack(fill="x", padx=5, pady=5)

        # 8. Algorithme
        ctk.CTkLabel(left_scroll, text="Algorithme:", anchor="w").pack(fill="x", padx=5, pady=(5,0))
        self.combo_algo = ctk.CTkComboBox(left_scroll, variable=self.algo_var,
                                          values=["Dijkstra", "Bellman-Ford", "Floyd-Warshall"])
        self.combo_algo.pack(fill="x", padx=5, pady=5)

        # 9. Bouton Exécuter (Violet/Accent dans le C, ici on met Vert comme planifié ou Accent ?)
        # C code says: gtk_widget_add_css_class(data->execute_btn, "accent-btn"); which is likely purple or blue.
        # But let's stick to a distinct color. 
        self.btn_run = ctk.CTkButton(left_scroll, text="🚀 Exécuter Algorithme", command=self._on_run_algo,
                                     fg_color="#9b59b6", hover_color="#8e44ad") # Violet-ish like C accent
        self.btn_run.pack(fill="x", padx=5, pady=10)

        # Separator 2
        ttk.Separator(left_scroll, orient='horizontal').pack(fill='x', pady=10)

        self.btn_save = ctk.CTkButton(left_scroll, text="💾 Enregistrer Graphe", command=self._on_save,
                                      fg_color="#3498db", hover_color="#2980b9") # Couleur Bleue
        self.btn_save.pack(fill="x", padx=5, pady=5)

        # 10. Bouton Retour
        self.btn_back = ctk.CTkButton(left_scroll, text="🔙 Retour Menu", command=self.destroy,
                                      hover_color="#555555")
        self.btn_back.pack(fill="x", padx=5, pady=5)

        # Separator 3
        ttk.Separator(left_scroll, orient='horizontal').pack(fill='x', pady=10)
        

        # 11. Résultats
        ctk.CTkLabel(left_scroll, text="Résultat:", font=("Arial", 14, "bold"), anchor="w").pack(fill="x", padx=5)
        self.lbl_result = ctk.CTkLabel(left_scroll, textvariable=self.result_var, wraplength=280, justify="left", anchor="nw")
        self.lbl_result.pack(fill="x", padx=5, pady=5)


        # === PANNEAU DROIT (VISU) ===
        right_frame = ctk.CTkFrame(self, fg_color="transparent")
        right_frame.grid(row=0, column=1, padx=10, pady=10, sticky="nsew")
        right_frame.grid_rowconfigure(0, weight=1)
        right_frame.grid_columnconfigure(0, weight=1)
        
        self.fig = plt.Figure(figsize=(6,6), dpi=100)
        self.ax = self.fig.add_subplot(111)
        
        self.canvas = FigureCanvasTkAgg(self.fig, master=right_frame)
        self.canvas_widget = self.canvas.get_tk_widget()
        self.canvas_widget.grid(row=0, column=0, sticky="nsew")
        
        # Events Matplotlib pour interactivité (Click & Drag pour arêtes)
        self.fig.canvas.mpl_connect('button_press_event', self._on_press)
        self.fig.canvas.mpl_connect('motion_notify_event', self._on_motion)
        self.fig.canvas.mpl_connect('button_release_event', self._on_release)
        self._toggle_save_button()
        
    # --- Interactivité (Drag & Drop Arêtes) ---
    def _get_node_at(self, x, y):
        if not self.pos: return None
        # Rayon approx en coordonnées data. 
        # C'est un peu tricky avec mpl, on assume threshold simple distance euclidienne
        threshold = 0.15 
        best_node = None
        min_dist = float('inf')
        for node, (nx, ny) in self.pos.items():
            d = math.sqrt((nx-x)**2 + (ny-y)**2)
            if d < threshold and d < min_dist:
                min_dist = d
                best_node = node
        return best_node

    def _on_press(self, event):
        if event.inaxes != self.ax or not self.current_graph: return
        node = self._get_node_at(event.xdata, event.ydata)
        if node:
            self.drag_start = node
            self.drag_current = (event.xdata, event.ydata)
            self._redraw_canvas_only() # Juste refresh visuel pour le trait

    def _on_motion(self, event):
        if self.drag_start and event.inaxes == self.ax:
            self.drag_current = (event.xdata, event.ydata)
            self._redraw_canvas_only()

    def _on_release(self, event):
        if self.drag_start:
            target = None
            if event.inaxes == self.ax:
                target = self._get_node_at(event.xdata, event.ydata)
            
            if target and target != self.drag_start:
                # Création arête
                self._prompt_edge(self.drag_start, target)
            
            self.drag_start = None
            self.drag_current = None
            self._refresh_viz()
            self._toggle_save_button()

    def _prompt_edge(self, u, v):
        w = simpledialog.askfloat("Poids de l'arête", f"Entrez le poids pour {u} -> {v}:", parent=self, minvalue=-1000, maxvalue=1000)
        if w is not None:
            self.current_graph.add_edge(u, v, w)

    # --- Logique UI ---
    
    def _toggle_save_button(self):
        """Active ou désactive le bouton Enregistrer en fonction de la présence de nœuds."""
        if self.current_graph and self.current_graph.nodes:
            self.btn_save.configure(state="normal")
        else:
            self.btn_save.configure(state="disabled")
    
    def _on_setting_changed(self, choice):
        if self.current_graph:
            is_directed = (self.path_type_var.get() == "Orienté")
            self.current_graph.is_directed = is_directed
            self._refresh_viz()

    def _on_clear_graph(self):
        self.current_graph = None
        self.pos = {}
        self.highlighted_path = []
        self.result_var.set("Graphe vidé.")
        self.menu_start.configure(values=[""])
        self.menu_end.configure(values=[""])
        self.start_node_var.set("")
        self.end_node_var.set("")
        self.ax.clear()
        self.canvas.draw()
        self._toggle_save_button()

    def _on_init_graph(self):
        mode = self.fill_mode_var.get()
        raw_input = self.entry_nodes.get().strip()
    
        dtype_str = self.data_type_var.get()
        mapping = {"Entier": "int", "Réel": "float", "Caractère": "char", "Chaîne": "string"}
        dtype = mapping.get(dtype_str, 'char')
    
        is_directed = (self.path_type_var.get() == "Orienté")
        nodes_to_add = []

        if mode == "Aléatoire":
            try:
                n = int(raw_input)
                if not (2 <= n <= 1000): raise ValueError
                # On utilise votre méthode existante pour générer les noms
                temp_graph = GraphStructure(is_directed, dtype)
                nodes_to_add = temp_graph.generate_node_names(n, dtype)
            except ValueError:
                messagebox.showerror("Erreur", "Veuillez saisir un nombre entier entre 2 et 1000 pour le mode aléatoire.")
                return
        else:
            # MODE MANUEL
            if not raw_input:
                messagebox.showerror("Erreur", "Veuillez saisir des noms de sommets séparés par des virgules.")
                return
        
            raw_nodes = [n.strip() for n in raw_input.split(',')]
        
            # Validation du type pour chaque nœud saisi
            checker = GraphStructure(is_directed, dtype)
            for val in raw_nodes:
                if not checker.validate_type(val):
                    messagebox.showerror("Erreur de Type", 
                        f"Le sommet '{val}' ne correspond pas au type choisi ({dtype_str}).\n"
                        f"Veuillez corriger votre saisie.")
                    return
            nodes_to_add = raw_nodes

        # Initialisation finale du graphe
        self.current_graph = GraphStructure(is_directed, dtype)
        for name in nodes_to_add:
            self.current_graph.add_node(name)
    
        # Mise à jour de la vue
        self.highlighted_path = []
        self.pos = {} 
        self.menu_start.configure(values=nodes_to_add)
        self.menu_end.configure(values=nodes_to_add)
        if nodes_to_add:
            self.start_node_var.set(nodes_to_add[0])
            self.end_node_var.set(nodes_to_add[-1])
        
        self.result_var.set("Graphe initialisé. Créez des arêtes avec la souris.")
        self._refresh_viz()
        self._toggle_save_button()


    def _on_run_algo(self):
        if not self.current_graph: return
        s = self.start_node_var.get()
        e = self.end_node_var.get()
        algo = self.algo_var.get()
        
        path = []
        res_txt = ""
        
        if algo == "Dijkstra":
            dist, pred, msg = self.current_graph.dijkstra(s)
            if msg == "Succès":
                d = dist.get(e, INFINITY_VAL)
                if d == INFINITY_VAL: res_txt = "Pas de chemin."
                else: 
                    path = self.current_graph.reconstruct_path(pred, s, e)
                    res_txt = f"Distance: {d}\nChemin: {' -> '.join(path)}"
            else: res_txt = msg

        elif algo == "Bellman-Ford":
            dist, pred, msg = self.current_graph.bellman_ford(s)
            if msg == "Succès":
                d = dist.get(e, INFINITY_VAL)
                if d == INFINITY_VAL: res_txt = "Pas de chemin."
                else:
                    path = self.current_graph.reconstruct_path(pred, s, e)
                    res_txt = f"Distance: {d}\nChemin: {' -> '.join(path)}"
            else: res_txt = msg

        elif algo == "Floyd-Warshall":
            dmat, pmat, msg = self.current_graph.floyd_warshall()
            if msg == "Succès":
                nodes = self.current_graph.get_nodes()
                try:
                    i, j = nodes.index(s), nodes.index(e)
                    d = dmat[i][j]
                    if d == INFINITY_VAL: res_txt = "Pas de chemin."
                    else:
                        # Reconstruct Floyd
                        c_temp = j
                        path = [e]
                        while c_temp != i:
                            prev = pmat[i][c_temp]
                            if prev is None: break
                            path.append(prev)
                            c_temp = nodes.index(prev)
                        path.append(s)
                        path = path[::-1] # Reverse
                        res_txt = f"Distance: {d}\nChemin: {' -> '.join(path)}"
                except: res_txt = "Erreur index FW"
            else: res_txt = msg

        self.highlighted_path = path
        self.result_var.set(res_txt)
        self._refresh_viz()


    def _on_save(self):
        """Ouvre une boîte de dialogue pour enregistrer le graphe sous forme de fichier JSON."""
        if not self.current_graph or not self.current_graph.nodes:
            messagebox.showwarning("Alerte", "Aucune donnée de graphe à enregistrer.", parent=self)
            return

        initial_name = f"Graph_{self.path_type_var.get()}_export.json"
        
        path = filedialog.asksaveasfilename(
            defaultextension=".json",
            initialfile=initial_name,
            filetypes=[("Fichiers JSON", "*.json"), ("Tous les fichiers", "*.*")],
            parent=self
        )
        
        if path:
            try:
                # Utilise la nouvelle méthode to_dict() du modèle
                graph_data = {
                    "type": self.path_type_var.get(),
                    "data_type": self.data_type_var.get(),
                    "structure": self.current_graph.to_dict() 
                }
                
                with open(path, "w", encoding="utf-8") as f:
                    json.dump(graph_data, f, indent=4)
                    
                messagebox.showinfo("Succès", f"Graphe enregistré dans :\n{path}", parent=self)
            except Exception as e:
                messagebox.showerror("Erreur d'enregistrement", f"Une erreur s'est produite lors de l'enregistrement : {e}", parent=self)

    def _refresh_viz(self):
        self.ax.clear()
        if not self.current_graph: 
            self.ax.axis('off')
            self.canvas.draw()
            return
            
        G = self.current_graph.to_networkx()
        
        if not self.pos or set(self.pos.keys()) != set(G.nodes()):
            self.pos = nx.circular_layout(G)
        
        # Couleurs
        node_colors = []
        start_n = self.start_node_var.get()
        end_n = self.end_node_var.get()
        
        for n in G.nodes():
            if n in self.highlighted_path:
                if n == start_n: node_colors.append('#2ecc71') # Vert
                elif n == end_n: node_colors.append('#e67e22') # Orange
                else: node_colors.append('#f1c40f') # Jaune
            else:
                node_colors.append('#9b59b6') # Violet (Comme C)
        
        # Arêtes
        edge_colors = []
        widths = []
        path_edges = []
        if len(self.highlighted_path) > 1:
            for k in range(len(self.highlighted_path)-1):
                u, v = self.highlighted_path[k], self.highlighted_path[k+1]
                path_edges.append((u, v))
                if not self.current_graph.is_directed:
                    path_edges.append((v, u))

        for u, v in G.edges():
            if (u,v) in path_edges:
                edge_colors.append('#e74c3c') # Rouge
                widths.append(2.5)
            else:
                edge_colors.append('gray')
                widths.append(1.0)

        # Dessin
        # draw_networkx is higher level, but allows less control if mixing arrows.
        # Use separating calls
        nx.draw_networkx_nodes(G, self.pos, ax=self.ax, node_color=node_colors, node_size=600, edgecolors='black')
        nx.draw_networkx_labels(G, self.pos, ax=self.ax, font_color='white', font_weight='bold')
        
        # Edges
        # arrows=True only works for Directed graphs in nx defaults
        nx.draw_networkx_edges(G, self.pos, ax=self.ax, edge_color=edge_colors, width=widths, 
                               arrows=self.current_graph.is_directed, arrowstyle='-|>', arrowsize=20)
        
        edge_labels = nx.get_edge_attributes(G, 'label')
        nx.draw_networkx_edge_labels(G, self.pos, edge_labels=edge_labels, ax=self.ax)
        
        # Temp draw drag line
        if self.drag_start and self.drag_current:
            sx, sy = self.pos[self.drag_start]
            ex, ey = self.drag_current
            self.ax.plot([sx, ex], [sy, ey], color='yellow', linestyle='--', linewidth=2)

        self.ax.axis('off')
        self.canvas.draw()
    
    def _redraw_canvas_only(self):
        # Optimisation pour le drag: ne redessine que si nécessaire ou pardessus
        # Matplotlib n'est pas performant pour du 60fps interactif via blit ici complexe
        # On refait refresh pour simplicité (sur petit graphe ça passe)
        self._refresh_viz()


# In[9]:


def bubble_sort(data_list):
    """Tri à bulle (In-place)."""
    n = len(data_list)
    for i in range(n - 1):
        for j in range(0, n - i - 1):
            if data_list[j] > data_list[j + 1]:
                data_list[j], data_list[j + 1] = data_list[j + 1], data_list[j]
    return data_list
def insertion_sort(data_list):
    """Tri par insertion (In-place)."""
    n = len(data_list)
    for i in range(1, n):
        key = data_list[i]
        j = i - 1
        while j >= 0 and data_list[j] > key:
            data_list[j + 1] = data_list[j]
            j -= 1
        data_list[j + 1] = key
    return data_list
def shell_sort(data_list):
    """Tri Shell (In-place)."""
    n = len(data_list)
    gap = n // 2
    while gap > 0:
        for i in range(gap, n):
            temp = data_list[i]
            j = i
            while j >= gap and data_list[j - gap] > temp:
                data_list[j] = data_list[j - gap]
                j -= gap
            data_list[j] = temp
        gap //= 2
    return data_list
def quick_sort(data_list):
    """Tri rapide (version récursive)."""
    if len(data_list) <= 1:
        return data_list
    else:
        pivot = data_list[len(data_list) // 2]
        left = [x for x in data_list if x < pivot]
        middle = [x for x in data_list if x == pivot]
        right = [x for x in data_list if x > pivot]
        return quick_sort(left) + middle + quick_sort(right)


class InputWindow(ctk.CTkToplevel):
    def __init__(self, master, array_size, data_type):
        super().__init__(master)
        self.title("Saisie Manuelle")
        self.geometry("300x150")
        self.grab_set()
        self.transient(master)

        self.array_size = array_size
        self.data_type = data_type
        self.current_index = 0
        self.input_elements = []

        self.label_info = ctk.CTkLabel(self, text=f"Type: {data_type}. Élément {self.current_index + 1}/{array_size}")
        self.label_info.pack(pady=(10,5))

        self.input_entry = ctk.CTkEntry(self)
        self.input_entry.pack(pady=5)
        self.input_entry.bind('<Return>', lambda event: self.submit_value())

        self.submit_button = ctk.CTkButton(self, text="OK", command=self.submit_value)
        self.submit_button.pack(pady=5)

        self.result = None
        self.protocol("WM_DELETE_WINDOW", self.on_close)

    def submit_value(self):
        value_str = self.input_entry.get().strip()
        if not value_str:
            self.label_info.configure(text="Saisie vide. Réessayez.", text_color="red")
            return
        try:
            if self.data_type == "Entier":
                value = int(value_str)
            elif self.data_type == "Réel":
                value = float(value_str)
            elif self.data_type == "Caractère":
                if value_str.isdigit():
                    raise ValueError("Ne peut pas être un chiffre")
                value = value_str[0]  # Prend juste le premier caractère
            elif self.data_type == "Chaîne de caractère":
                # On refuse si c'est uniquement un nombre
                if value_str.replace('.', '', 1).isdigit():
                    raise ValueError("Ne peut pas être un nombre seul")
                value = value_str
            else:
                value = value_str

            self.input_elements.append(value)
            self.current_index += 1
            self.input_entry.delete(0, "end")
            self.label_info.configure(text_color="white")

            if self.current_index >= self.array_size:
                self.result = self.input_elements
                self.destroy()
            else:
                self.label_info.configure(
                    text=f"Type: {self.data_type}. Élément {self.current_index + 1}/{self.array_size}"
                )

        except ValueError as e:
            self.label_info.configure(text=f"Erreur: {e}", text_color="red")

    def on_close(self):
        self.result = None
        self.destroy()


# =========================================================================
# 3. CLASSE POUR LA FENÊTRE DE COURBE (Légèrement modifiée pour être réutilisée)
# =========================================================================

class CurveWindow(ctk.CTkToplevel):
    def __init__(self, master, sizes_dict, execution_times_dict, array_type):
        super().__init__(master)
        self.title(f"Courbes de Performance - Type: {array_type}")
        self.geometry("800x600")
        self.grab_set() 
        
        # Création du graphique Matplotlib
        fig, ax = plt.subplots(figsize=(7, 5), dpi=100)
        
        # Définition des couleurs pour les 4 courbes
        colors = {
            'Tri à bulle': 'red',             
            'Tri par insertion': 'orange',    
            'Tri Shell': 'green',           
            'Tri rapide': 'blue'             
        }
        
        # Tracer toutes les courbes sur le même axe
        for method_name in sizes_dict:
            ax.plot(
                sizes_dict[method_name], 
                execution_times_dict[method_name], 
                marker='o',               # Affiche les points de mesure
                linestyle='-', 
                color=colors.get(method_name, 'gray'),
                label=method_name
            )
        
        # Configuration des axes
        ax.set_xlabel("Taille du tableau (N)")
        ax.set_ylabel("Temps d'exécution (secondes / s)")
        ax.set_title(f"Comparaison des Algorithmes de Tri (Type: {array_type})")
        ax.grid(True)
        ax.legend() # Affiche la légende (Tri à Bulle, Insertion, Shell, Rapide)
        
        # Intégrer le graphique dans la fenêtre CustomTkinter
        canvas = FigureCanvasTkAgg(fig, master=self)
        canvas_widget = canvas.get_tk_widget()
        canvas_widget.pack(side=ctk.TOP, fill=ctk.BOTH, expand=True, padx=10, pady=10)
        canvas.draw()
        
        close_button = ctk.CTkButton(self, text="Fermer", command=self.destroy)
        close_button.pack(pady=10)

# =========================================================================
# CLASSE POUR LA FENÊTRE DE TRI DES TABLEAUX
# =========================================================================



# Les fonctions de tri (bubble_sort, insertion_sort, shell_sort, quick_sort) doivent être définies ailleurs
# Pour la compilation, nous utiliserons des stubs
def bubble_sort(arr):
    n = len(arr)
    for i in range(n):
        for j in range(0, n-i-1):
            if arr[j] > arr[j+1]:
                arr[j], arr[j+1] = arr[j+1], arr[j]
    return arr

def insertion_sort(arr):
    for i in range(1, len(arr)):
        key = arr[i]
        j = i - 1
        while j >= 0 and key < arr[j]:
            arr[j + 1] = arr[j]
            j -= 1
        arr[j + 1] = key
    return arr

def shell_sort(arr):
    n = len(arr)
    gap = n // 2
    while gap > 0:
        for i in range(gap, n):
            temp = arr[i]
            j = i
            while j >= gap and arr[j - gap] > temp:
                arr[j] = arr[j - gap]
                j -= gap
            arr[j] = temp
        gap //= 2
    return arr

def quick_sort_impl(arr):
    if len(arr) <= 1:
        return arr
    pivot = arr[len(arr) // 2]
    left = [x for x in arr if x < pivot]
    middle = [x for x in arr if x == pivot]
    right = [x for x in arr if x > pivot]
    return quick_sort_impl(left) + middle + quick_sort_impl(right)

def quick_sort(arr):
    #quick_sort_impl trie et renvoie un nouveau tableau. On doit copier les résultats dans l'original.
    sorted_arr = quick_sort_impl(arr)
    arr[:] = sorted_arr # Mettre à jour le tableau original en place
    return arr

# Stub pour la fenêtre de saisie manuelle (non fournie dans le code initial)
class InputWindow(ctk.CTkToplevel):
    def __init__(self, master, size, type_choice):
        super().__init__(master)
        self.result = None
        self.title("Saisie Manuelle")
        self.geometry("300x150")
        self.label = ctk.CTkLabel(self, text=f"Entrez {size} éléments de type {type_choice}, séparés par des virgules:")
        self.label.pack(padx=10, pady=10)
        self.entry = ctk.CTkEntry(self)
        self.entry.pack(padx=10, pady=5)
        self.button = ctk.CTkButton(self, text="Valider", command=self.validate)
        self.button.pack(padx=10, pady=10)
        self.grab_set()

    def validate(self):
        # Ceci est un stub simple. La vraie validation devrait être plus robuste.
        try:
            input_values = self.entry.get().split(',')
            self.result = [int(val.strip()) for val in input_values] 
            self.destroy()
        except ValueError:
            # Afficher une erreur ou juste ignorer la saisie invalide pour cet exemple
            self.result = None
            self.destroy()

# Stub pour la fenêtre de la courbe
class CurveWindow(ctk.CTkToplevel):
    def __init__(self, master, all_sizes, all_execution_times, array_type):
        super().__init__(master)
        self.title(f"Courbe de performance des tris ({array_type})")
        self.geometry("800x600")
        
        fig, ax = plt.subplots(figsize=(8, 5))
        
        for method_name in all_sizes:
            ax.plot(all_sizes[method_name], all_execution_times[method_name], 
                    label=method_name, marker='o')

        ax.set_xlabel("Taille du tableau (N)")
        ax.set_ylabel("Temps d'exécution (ms)")
        ax.set_title(f"Performance des Algorithmes de Tri - Type: {array_type}")
        ax.legend()
        ax.grid(True)
        
        canvas = FigureCanvasTkAgg(fig, master=self)
        canvas_widget = canvas.get_tk_widget()
        canvas_widget.pack(fill=ctk.BOTH, expand=True)
        canvas.draw()
        self.grab_set()

class ArraySortWindow(ctk.CTkToplevel):
    def __init__(self, master=None):
        super().__init__(master)
        
        self.title("Gestion et Tri de Tableaux")
        self.geometry("800x700") 
        self.grab_set() 
        
        self.current_array = []
        self.last_sort_time = 0.0
        self.last_sort_method = ""
        self.data_history = [] 
        
        self.grid_columnconfigure((0, 1), weight=1)
        self.grid_rowconfigure((0, 1, 2, 3, 4, 5), weight=0)
        self.grid_rowconfigure((6, 7), weight=1)
        self.grid_rowconfigure(8, weight=0) 

        self.label_title = ctk.CTkLabel(
            master=self,
            text="Tri des Tableaux",
            font=ctk.CTkFont(size=24, weight="bold")
        )
        self.label_title.grid(row=0, column=0, columnspan=2, pady=(20, 15))

        control_frame = ctk.CTkFrame(self)
        control_frame.grid(row=1, column=0, rowspan=6, padx=20, pady=10, sticky="nsew")
        control_frame.grid_columnconfigure(0, weight=1)
        
        ctk.CTkLabel(control_frame, text="1. Type de tableau :").grid(row=0, column=0, padx=10, pady=(10, 0), sticky="w")
        self.type_menu = ctk.CTkComboBox(control_frame, values=["Entier", "Réel", "Caractère", "Chaîne de caractère"])
        self.type_menu.set("Entier")
        self.type_menu.grid(row=1, column=0, padx=10, pady=(0, 10), sticky="ew")

        ctk.CTkLabel(control_frame, text="2. Mode de remplissage :").grid(row=2, column=0, padx=10, pady=(10, 0), sticky="w")
        self.mode_menu = ctk.CTkComboBox(control_frame, values=["Manuel", "Aléatoire"], command=self.toggle_input_mode)
        self.mode_menu.set("Aléatoire")
        self.mode_menu.grid(row=3, column=0, padx=10, pady=(0, 10), sticky="ew")
        
        ctk.CTkLabel(control_frame, text="3. Taille du tableau :").grid(row=4, column=0, padx=10, pady=(10, 0), sticky="w")
        self.size_entry = ctk.CTkEntry(control_frame, placeholder_text="Ex: 1000")
        self.size_entry.insert(0, "0")
        self.size_entry.grid(row=5, column=0, padx=10, pady=(0, 10), sticky="ew")

        ctk.CTkLabel(control_frame, text="4. Méthode de tri :").grid(row=6, column=0, padx=10, pady=(10, 0), sticky="w")
        self.method_menu = ctk.CTkComboBox(control_frame, values=["Tri à bulle", "Tri par insertion", "Tri Shell", "Tri rapide"])
        self.method_menu.set("Tri rapide")
        self.method_menu.grid(row=7, column=0, padx=10, pady=(0, 10), sticky="ew")
        
        ctk.CTkLabel(self, text="5. Résultats des Comparaisons :").grid(row=1, column=1, padx=20, pady=(10, 5), sticky="w")
        self.comparison_textbox = ctk.CTkTextbox(self, width=350, height=80, wrap="word") 
        self.comparison_textbox.grid(row=2, column=1, padx=20, pady=5, sticky="ew")
        self.comparison_textbox.insert("0.0", "Utilisez 'Comparer Méthodes' pour afficher les temps d'exécution des 4 tris ici.")
        self.comparison_textbox.configure(state="disabled")
        
        self.time_label = ctk.CTkLabel(self, text="Temps d'exécution: N/A", text_color="orange")
        self.time_label.grid(row=3, column=1, padx=20, pady=(10, 5), sticky="w")
        
        ctk.CTkLabel(self, text="Tableau avant tri :").grid(row=4, column=1, padx=20, pady=(10, 5), sticky="w")
        self.before_sort_textbox = ctk.CTkTextbox(self, width=350, height=100, wrap="word") 
        self.before_sort_textbox.grid(row=5, column=1, padx=20, pady=0, sticky="nsew")
        
        ctk.CTkLabel(self, text="Tableau après tri :").grid(row=6, column=1, padx=20, pady=(10, 5), sticky="w")
        self.after_sort_textbox = ctk.CTkTextbox(self, width=350, height=100, wrap="word") 
        self.after_sort_textbox.grid(row=7, column=1, padx=20, pady=0, sticky="nsew")

        button_action_frame = ctk.CTkFrame(self)
        button_action_frame.grid(row=8, column=0, columnspan=2, padx=20, pady=(15, 20), sticky="ew")
        # Ajout d'une colonne pour le bouton Réinitialiser
        button_action_frame.grid_columnconfigure((0, 1, 2, 3, 4, 5, 6), weight=1)

        ctk.CTkButton(button_action_frame, text="Générer Tableau", command=self.generate_and_display_array).grid(row=0, column=0, padx=5, pady=10)
        ctk.CTkButton(button_action_frame, text="Trier", command=self.perform_sort_and_display).grid(row=0, column=1, padx=5, pady=10)
        ctk.CTkButton(button_action_frame, text="Comparer Méthodes", command=self.compare_methods).grid(row=0, column=2, padx=5, pady=10)
        ctk.CTkButton(button_action_frame, text="Enregistrer Données", command=self.save_results).grid(row=0, column=3, padx=5, pady=10)
        ctk.CTkButton(button_action_frame, text="Générer Courbe", command=self.generate_curve).grid(row=0, column=4, padx=5, pady=10)
        
        # Le bouton de réinitialisation est ajouté ici
        ctk.CTkButton(button_action_frame, text="🔄 Réinitialiser", command=self.reset_ui).grid(row=0, column=5, padx=5, pady=10)
        
        ctk.CTkButton(button_action_frame, text="🔙 Retour Menu", command=self.destroy).grid(row=0, column=6, padx=5, pady=10)
        
    def toggle_input_mode(self, choice):
        if choice == "Manuel":
            self.time_label.configure(text="Mode Manuel sélectionné. La saisie se fera via une fenêtre modale.", text_color="orange")
        else:
            self.time_label.configure(text="Mode Aléatoire sélectionné. La taille N sera utilisée pour la génération.", text_color="orange")

    def generate_array(self):
        type_choice = self.type_menu.get()
        mode_choice = self.mode_menu.get()

        try:
            array_size = int(self.size_entry.get())
            if array_size <= 0:
                self.time_label.configure(text="Taille invalide.", text_color="red")
                return None
        except ValueError:
            self.time_label.configure(text="Taille invalide.", text_color="red")
            return None

        if mode_choice == "Manuel":
            input_win = InputWindow(self, array_size, type_choice)
            self.wait_window(input_win)
            elements = input_win.result
            if elements is None:
                self.time_label.configure(text="Saisie manuelle annulée.", text_color="red")
                return None

            self.comparison_textbox.configure(state="normal")
            self.comparison_textbox.delete("0.0", "end")
            self.comparison_textbox.insert("0.0", f"Données manuelles : {elements}")
            self.comparison_textbox.configure(state="disabled")
            return elements
        else:
            # Simplifié pour Entier, la gestion des autres types nécessiterait une logique plus poussée
            if type_choice == "Entier":
                elements = [random.randint(1, array_size*10) for _ in range(array_size)]
            elif type_choice == "Réel":
                elements = [round(random.uniform(1.0, array_size * 10.0), 2) for _ in range(array_size)]
            elif type_choice == "Caractère":
                import string
                elements = [random.choice(string.ascii_letters) for _ in range(array_size)]
            else: # Chaîne de caractère
                 import string
                 elements = [''.join(random.choices(string.ascii_letters + string.digits, k=random.randint(3, 8))) for _ in range(array_size)]
                 
            self.comparison_textbox.configure(state="normal")
            self.comparison_textbox.delete("0.0", "end")
            self.comparison_textbox.insert("0.0", f"Données aléatoires générées.")
            self.comparison_textbox.configure(state="disabled")
            return elements

    def generate_and_display_array(self):
        """Génère le tableau et l'affiche dans la zone 'avant tri'."""
        
        raw_array = self.generate_array()
        if raw_array is None:
            return
        
        self.current_array = list(raw_array)
        
        self.before_sort_textbox.delete("0.0", "end")
        self.before_sort_textbox.insert("0.0", ", ".join(map(str, self.current_array)))
        
        self.after_sort_textbox.delete("0.0", "end")
        
        self.time_label.configure(
            text=f"Tableau généré avec succès. Taille: {len(self.current_array)}", 
            text_color="green"
        )

    def perform_sort_and_display(self):
        """Exécute le tri sélectionné sur le tableau DÉJÀ GÉNÉRÉ."""
        
        if not self.current_array:
            self.time_label.configure(
                text="Veuillez d'abord générer un tableau avec le bouton 'Générer Tableau'.", 
                text_color="red"
            )
            return
        
        array_before_sort = list(self.current_array)
        array_to_sort = list(self.current_array)
        
        method_choice = self.method_menu.get()
        sort_function = {
            "Tri à bulle": bubble_sort,
            "Tri par insertion": insertion_sort,
            "Tri Shell": shell_sort,
            "Tri rapide": quick_sort
        }.get(method_choice)

        if sort_function is None:
            self.time_label.configure(text="Erreur: Méthode de tri non reconnue.", text_color="red")
            return
        
        self.before_sort_textbox.delete("0.0", "end")
        self.after_sort_textbox.delete("0.0", "end")

        start_time = time.perf_counter()
        sorted_array = sort_function(array_to_sort)
        end_time = time.perf_counter()
        
        self.last_sort_time = (end_time - start_time) * 1000
        self.last_sort_method = method_choice
        
        self.time_label.configure(
            text=f"Temps d'exécution ({method_choice}): {self.last_sort_time:.4f} ms", 
            text_color="green"
        )
        
        self.before_sort_textbox.insert("0.0", ", ".join(map(str, array_before_sort)))
        self.after_sort_textbox.insert("0.0", ", ".join(map(str, sorted_array)))
        
        self.data_history.append({
            "Time": self.last_sort_time,
            "Method": method_choice,
            "Size": len(array_before_sort),
            "Type": self.type_menu.get(),
            "Array_Before": array_before_sort,
            "Array_After": sorted_array
        })

    def compare_methods(self):
        """Compare les quatre méthodes de tri sur le tableau DÉJÀ GÉNÉRÉ."""
        
        if not self.current_array:
            self.time_label.configure(
                text="Veuillez d'abord générer un tableau avec le bouton 'Générer Tableau'.", 
                text_color="red"
            )
            return
        
        if len(self.current_array) > 5000:
             self.time_label.configure(
                 text="Attention: Tableau trop grand (N>5000). La comparaison peut être lente.", 
                 text_color="orange"
             )
             
        methods = {
            "Tri à bulle": bubble_sort,
            "Tri par insertion": insertion_sort,
            "Tri Shell": shell_sort,
            "Tri rapide": quick_sort
        }
        
        results = {}
        for name, func in methods.items():
            temp_array = list(self.current_array) 
            start_time = time.perf_counter()
            func(temp_array)
            end_time = time.perf_counter()
            results[name] = (end_time - start_time) * 1000
        
        comparison_text = f"--- Temps de Comparaison (N={len(self.current_array)}) ---\n"
        best_time = float('inf')
        best_method = ""
        for name, t in results.items():
            comparison_text += f"{name}: {t:.4f} ms\n"
            if t < best_time:
                best_time = t
                best_method = name
        
        self.comparison_textbox.configure(state="normal")
        self.comparison_textbox.delete("0.0", "end")
        self.comparison_textbox.insert("0.0", comparison_text)
        self.comparison_textbox.configure(state="disabled")
        
        self.time_label.configure(
            text=f"Le plus rapide: {best_method} ({best_time:.4f} ms)", 
            text_color="blue"
        )
        
        plt.figure(figsize=(8, 5))
        plt.bar(results.keys(), results.values(), color=['red', 'orange', 'green', 'blue'])
        plt.ylabel("Temps d'exécution (millisecondes)")
        plt.title(f"Comparaison des Méthodes de Tri (Taille: {len(self.current_array)})")
        plt.show()

    def save_results(self):
        if not self.data_history:
            self.time_label.configure(text="Aucune donnée à enregistrer. Veuillez effectuer un tri d'abord.", text_color="red")
            return

        file_path = filedialog.asksaveasfilename(
            defaultextension=".txt",
            filetypes=[("Fichiers Texte", "*.txt"), ("Tous les fichiers", "*.*")],
            title="Enregistrer les résultats de tri"
        )

        if file_path:
            try:
                with open(file_path, "w") as f:
                    f.write("--- HISTORIQUE DES OPÉRATIONS DE TRI ---\n\n")
                    for idx, data in enumerate(self.data_history):
                        f.write(f"Opération n°{idx + 1}:\n")
                        f.write(f"  Méthode: {data['Method']}\n")
                        f.write(f"  Type: {data['Type']}\n")
                        f.write(f"  Taille: {data['Size']}\n")
                        f.write(f"  Temps d'exécution: {data['Time']:.4f} ms\n")
                        f.write(f"  Tableau avant tri: {str(data['Array_Before'])}\n") 
                        f.write(f"  Tableau après tri: {str(data['Array_After'])}\n")
                        f.write("-" * 30 + "\n")
                self.time_label.configure(text=f"Données enregistrées dans {file_path}", text_color="green")
            except Exception as e:
                self.time_label.configure(text=f"Erreur d'enregistrement: {e}", text_color="red")
    
    def generate_curve(self):
        """Génère des courbes Temps vs Taille pour les QUATRE méthodes de tri."""
        
        if not self.current_array:
             self.time_label.configure(text="Veuillez d'abord générer un tableau pour définir le type de données.", text_color="red")
             return

        array_type = self.type_menu.get()
        
        if array_type not in ["Entier", "Réel"]: 
             self.time_label.configure(text="Courbe limitée aux types Entier/Réel pour la performance.", text_color="red")
             return
        
        try:
            max_size = int(self.size_entry.get())
            if max_size <= 0:
                raise ValueError
        except:
            self.time_label.configure(text="Veuillez entrer une taille valide.", text_color="red")
            return
            
        methods = {
            "Tri à bulle": bubble_sort,
            "Tri par insertion": insertion_sort,
            "Tri Shell": shell_sort,
            "Tri rapide": quick_sort
        }
        
        all_sizes = {}
        all_execution_times = {}

        self.time_label.configure(text="Début de la génération de la courbe de performance pour les 4 méthodes...", text_color="blue")
        self.update_idletasks()
        
        num_points = min(10, max_size // 10 + 1)
        if max_size <= 100:
            sizes_to_test = list(range(10, max_size + 1, max(1, max_size // num_points)))
        else:
            sizes_to_test = list(range(max_size // num_points, max_size + 1, max_size // num_points))
        
        if sizes_to_test and sizes_to_test[-1] != max_size:
            sizes_to_test.append(max_size)
        
        for method_name, sort_function in methods.items():
            
            execution_times = []
            
            for size in sizes_to_test:

                if array_type == "Entier":
                    data = [random.randint(1, size * 10) for _ in range(size)]
                else: 
                    data = [round(random.uniform(1.0, size * 10.0), 2) for _ in range(size)]
                
                temp_array = list(data)
                start_time = time.perf_counter()
                sort_function(temp_array)
                end_time = time.perf_counter()
                
                execution_times.append((end_time - start_time) * 1000)
                self.time_label.configure(text=f"Calcul en cours: {method_name} taille {size}...", text_color="blue") 
                self.update_idletasks()

            all_sizes[method_name] = sizes_to_test
            all_execution_times[method_name] = execution_times

        CurveWindow(self, all_sizes, all_execution_times, array_type)
        self.time_label.configure(text="Courbe de comparaison Temps vs Taille générée pour les 4 méthodes.", text_color="green")
        
    def reset_ui(self):
        """Réinitialise toutes les zones de texte et l'état interne de l'application."""
        
        # 1. Réinitialiser les zones d'affichage de texte
        self.before_sort_textbox.delete("0.0", "end")
        self.after_sort_textbox.delete("0.0", "end")
        
        self.comparison_textbox.configure(state="normal")
        self.comparison_textbox.delete("0.0", "end")
        self.comparison_textbox.insert("0.0", "Utilisez 'Comparer Méthodes' pour afficher les temps d'exécution des 4 tris ici.")
        self.comparison_textbox.configure(state="disabled")
        
        # 2. Réinitialiser les variables d'état
        self.current_array = []
        self.last_sort_time = 0.0
        self.last_sort_method = ""
        self.data_history = [] 
        
        # 3. Mettre à jour les labels d'information
        self.time_label.configure(text="Temps d'exécution: N/A", text_color="orange")
        
        # 4. Afficher un message de confirmation (optionnel)
        self.time_label.configure(text="Application réinitialisée. Prête pour de nouvelles opérations.", text_color="green")
        
# =========================================================================
# 5. CLASSE POUR LA FENÊTRE PRINCIPALE
# =========================================================================

class MainApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        # --- Configuration de la Fenêtre ---
        self.title("Mini-Système : Structure de Données")
        self.geometry("700x550")
        self.minsize(500, 450)
        
        # Configuration de la grille principale
        self.grid_rowconfigure(0, weight=1) # Header
        self.grid_rowconfigure(1, weight=2) # Contenu (Boutons)
        self.grid_rowconfigure(2, weight=1) # Footer
        self.grid_columnconfigure(0, weight=1)
        self.current_font = ctk.CTkFont(family="Roboto", size=32, weight="bold")
        # --- 1. En-tête (Header) ---
        self.header_frame = ctk.CTkFrame(self, fg_color="transparent")
        self.header_frame.grid(row=0, column=0, sticky="nsew", pady=(20, 0))
        self.header_frame.grid_columnconfigure(0, weight=1)
        self.label_title = ctk.CTkLabel(
            self.header_frame, 
            text="Mini-Système\nStructures de Données", 
            font=self.current_font,
            text_color=("gray20", "gray90")
        )
        self.label_title.pack(pady=10)
        self.lbl_subtitle = ctk.CTkLabel(
            self.header_frame, 
            text="Visualisez et manipulez vos structures de données\nde manière intuitive et interactive.", 
            font=ctk.CTkFont(family="Roboto", size=14),
            text_color=("gray50", "gray70")
        )
        self.lbl_subtitle.pack(pady=(0, 20))
        # --- 2. Contenu (Boutons) ---
        self.button_frame = ctk.CTkFrame(self, fg_color=("gray90", "gray17"), corner_radius=15)
        self.button_frame.grid(row=1, column=0, sticky="nsew", padx=40, pady=20)
        
        # Grille 2x2 pour les boutons
        self.button_frame.grid_columnconfigure((0, 1), weight=1)
        self.button_frame.grid_rowconfigure((0, 1), weight=1)
        self.open_windows = {}
        # Définition des boutons
        buttons_data = [
            {"key": "Tableau", "text": "Tableau", "icon": "📊", "color": "#3498db", "hover": "#2980b9", "pos": (0, 0)},
            {"key": "Liste", "text": "Liste Chaînée", "icon": "🔗", "color": "#e67e22", "hover": "#d35400", "pos": (0, 1)},
            {"key": "Arbre", "text": "Arbre Binaire / N Aire", "icon": "🌳", "color": "#2ecc71", "hover": "#27ae60", "pos": (1, 0)},
            {"key": "Graphe", "text": "Graphe", "icon": "🕸️", "color": "#9b59b6", "hover": "#8e44ad", "pos": (1, 1)},
        ]
        for btn_info in buttons_data:
            self._create_menu_button(btn_info)
        # --- 3. Footer ---
        self.footer_frame = ctk.CTkFrame(self, fg_color="transparent")
        self.footer_frame.grid(row=2, column=0, sticky="s", pady=20)
        
        ctk.CTkLabel(
            self.footer_frame, 
            text="©Project - TP Structures de Données", 
            font=ctk.CTkFont(size=10), 
            text_color="gray50"
        ).pack()
        # Binding du redimensionnement
        self.bind("<Configure>", self._on_resize) 
        self.update_idletasks() 
        self._on_resize(None)
    def _create_menu_button(self, info):
        row, col = info["pos"]
        btn = ctk.CTkButton(
            self.button_frame,
            text=f"  {info['icon']}  {info['text']}",
            font=ctk.CTkFont(family="Roboto", size=18, weight="bold"),
            fg_color=info["color"],
            hover_color=info["hover"],
            height=60,
            corner_radius=12,
            command=lambda t=info["key"]: self.button_action(t)
        )
        btn.grid(row=row, column=col, sticky="nsew", padx=20, pady=20)
    def button_action(self, structure):
        if structure == "Tableau":
            self.open_array_window()
        elif structure == "Liste":
            ListWindow(self) 
            pass
        elif structure == "Arbre":
            TreeWindow(self) 
            pass
        elif structure == "Graphe":
            GraphWindow(self) 
            pass
    def open_array_window(self):
        if "array_sort" not in self.open_windows or not self.open_windows["array_sort"].winfo_exists():
            self.open_windows["array_sort"] = ArraySortWindow(self) 
        else:
            self.open_windows["array_sort"].focus()
    def _on_resize(self, event):
        new_width = self.winfo_width()
        # Ajustement dynamique de la taille de police du titre
        new_font_size = max(20, int(new_width / 25)) 
        self.current_font.configure(size=new_font_size)
        
        new_wraplength = new_width - 40 
        if new_wraplength > 100:
            self.label_title.configure(wraplength=new_wraplength)
        
        self.label_title.configure(font=self.current_font)


# =========================================================================
# 6. LANCEMENT DE L'APPLICATION
# =========================================================================

if __name__ == "__main__":
    app = MainApp()
    app.mainloop()


# In[ ]:





# In[ ]:





# In[ ]:





# In[ ]:




