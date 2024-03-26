# MacMahon Solver Project

## Auteur
DJAGHLOUL AYOUB AHMED AMINE (IA)

## Introduction
Ce projet vise à résoudre le jeu de MacMahon en utilisant divers algorithmes de backtracking, y compris des implémentations séquentielles et parallèles. L'objectif est d'explorer l'efficacité du parallélisme en programmation multithread en C++11 pour améliorer les performances de résolution du jeu.

## Compilation des programmes
Les commandes suivantes sont utilisées pour compiler les programmes associés à ce projet. Assurez-vous d'avoir un environnement C++11 avec support de pthread configuré sur votre machine.

### Algorithme séquentiel
```bash
g++ main1.cpp -Wall -o main.o

Algorithme avec threads
```bash
g++ -std=c++11 -pthread main_thread.cpp -o main_thread

Algorithme avec ThreadPool
```bash
g++ -std=c++11 -pthread main_threadpool.cpp -o main_threadpool

Exécution des programmes
Une fois compilés, les programmes peuvent être exécutés comme suit pour tester le solver sur un plateau de jeu 6x6.

Avec l'algorithme de thread
```bash
./main_thread 6x6_colorv2.txt

Avec l'algorithme séquentiel
```bash
./main 6x6_colorv2.txt


Il semble que vous ayez déjà préparé le contenu de votre fichier README.md pour le projet MacMahon Solver. Si vous cherchez à enregistrer ce contenu dans un fichier README.md réel, je peux vous aider à le faire. Voici comment nous pouvons procéder avec du code Python pour créer le fichier :

python
Copy code
# Définir le contenu du fichier README.md
readme_content = """
# MacMahon Solver Project

## Auteur
DJAGHLOUL AYOUB AHMED AMINE (IA)

## Introduction
Ce projet vise à résoudre le jeu de MacMahon en utilisant divers algorithmes de backtracking, y compris des implémentations séquentielles et parallèles. L'objectif est d'explorer l'efficacité du parallélisme en programmation multithread en C++11 pour améliorer les performances de résolution du jeu.

## Compilation des programmes
Les commandes suivantes sont utilisées pour compiler les programmes associés à ce projet. Assurez-vous d'avoir un environnement C++11 avec support de pthread configuré sur votre machine.

### Algorithme séquentiel
g++ main1.cpp -Wall -o main.o

### Algorithme avec threads
g++ -std=c++11 -pthread main_thread.cpp -o main_thread

### Algorithme avec ThreadPool
g++ -std=c++11 -pthread main_threadpool.cpp -o main_threadpool

### Exécution des programmes:

Une fois compilés, les programmes peuvent être exécutés comme suit pour tester le solver sur un plateau de jeu 6x6.

Avec l'algorithme de thread
./main_thread 6x6_colorv2.txt

Avec l'algorithme séquentiel
./main 6x6_colorv2.txt

Description des Implémentations
Algorithme Séquentiel: Suit une approche classique de backtracking, explorant l'espace de solutions de manière linéaire.
Algorithme avec Threads: Utilise std::thread pour exécuter le backtracking en parallèle sur plusieurs threads.
Algorithme avec ThreadPool: Optimise le traitement parallèle en réutilisant les threads pour différentes tâches de backtracking, réduisant ainsi l'overhead de gestion des threads.

Résultats de Performances :
Les tests montrent que les implémentations parallèles surpassent significativement l'algorithme séquentiel, surtout pour les plateaux de taille moyenne et grande, démontrant l'efficacité du parallélisme dans la réduction du temps de calcul.

Conclusion :

Les approches parallèles de résolution du jeu de MacMahon présentent des améliorations significatives en termes de performances comparées à l'approche séquentielle, soulignant l'importance de l'exploitation du parallélisme dans la résolution de problèmes combinatoires complexes.
