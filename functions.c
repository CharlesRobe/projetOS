#include <fcntl.h>
#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FICHIER_CIRCUITS "circuits_inities.txt"

void creer_dossier(const char *circuit) {
    if (access(circuit, F_OK) == -1) {  // check si exisite 
        if (mkdir(circuit, 0700) == 0) {
            printf("Dossier %s créé.\n", circuit);
        } else {
            printf("Erreur : Dossier %s pas créé.\n",circuit );
        }
    } else {
        printf("Dossier %s existe déjà.\n", circuit);
    }
}

void ajouter_circuit(const char *circuit) {
    int fichier = open(FICHIER_CIRCUITS, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fichier == -1) {
        perror("Fichier pas ouvert");
        return;
    }

    size_t circuit_length = strlen(circuit);
    if (write(fichier, circuit, circuit_length) == -1 || write(fichier, "\n", 1) == -1) {
        perror("Erreur lors de l'écriture dans le fichier");
        close(fichier);
        return;
    }

    close(fichier);
    printf("Circuit %s ajouté à la liste des circuits initiés.\n", circuit);
}

void sauvegarder_etat(const char *circuit, int etape) {
    char nomFichier[64];
    snprintf(nomFichier, sizeof(nomFichier), "%s/etat.txt", circuit);

    int fichier = open(nomFichier, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fichier == -1) {
        perror("Sauvegarde pas réussi");
        return;
    }

    if (write(fichier, &etape, sizeof(etape)) == -1) {
        perror("Ecriture pas réussi");
    }

    close(fichier);
}

int charger_etat(const char *circuit) {
    char nomFichier[64];
    snprintf(nomFichier, sizeof(nomFichier), "%s/etat.txt", circuit);

    int fichier = open(nomFichier, O_RDONLY);
    int etape;
    if (fichier != -1) {
        if (read(fichier, &etape, sizeof(etape)) != sizeof(etape)) {
            etape = 0; 
        }
        close(fichier);
    }
    return etape;
}

const char *dernier_circuit() {
    static char dernier[64];
    int fichier = open(FICHIER_CIRCUITS, O_RDONLY);
    if (fichier == -1) {
        perror("Fichier introuvable ");
        return NULL; 
    }
    
    
    lseek(fichier, -1, SEEK_END); // On va a la fin du fichier pour lire la dernière ligne 

    char c;
    while (read(fichier, &c, 1) == 1 && c != '\n') {
        lseek(fichier, -2, SEEK_CUR); // Comme on lit le char du fichier on a decalé le curseur vers la droite et donc il faut remonter celuila + celui qu'on veux vérifier
    }

    ssize_t bytes_read = read(fichier, dernier, sizeof(dernier) - 1);
    if (bytes_read > 0) {
        dernier[bytes_read] = '\0';  
    } else {
        close(fichier);
        return NULL;
    }

    close(fichier);
    return dernier;
}