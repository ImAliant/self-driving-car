# Projet de Programmation synchrone

Ce répertoire contient le projet du cours de Programmation synchrone du M2
Informatique de l'Université de Paris et de l'École d'Ingénieur Denis Diderot.

Le [sujet](sujet/sujet-projet.pdf) contient tous les détails.

# Environnement et Dependances
## Environnement
- Un système d'exploitation de type Unix (Linux, MacOS)
- Chaîne de développement C standard avec le compilateur GCC et GNU Make 4.0+
## Dependances
### Opam
`sudo apt install opam`   
### Heptagon
```
opam init
eval $(opam env)
opam install lablgtk heptagon
```
### SDL2
`sudo apt install libsdl2-dev`

# Compilation
`make` et `make test` pour lancer le programme de test.

# Execution
`./scontest assets/<n>.map`

# Auteurs
## Contrôle-commande
- **[Dilara DEDEOGLU](https://github.com/dedeoglu)**
- **[Alexandre DIAMANT](https://github.com/imaliant)**
## Code pour la simulation du véhicule
- **[Adrien GUATTO](https://www.irif.fr/~guatto/)** (Professeur encadrant)
