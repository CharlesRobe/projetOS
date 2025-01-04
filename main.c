#include <fcntl.h>
#include "main_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


#define FICHIER_CIRCUITS "circuits_inities.txt"

int main(int argc, char *argv[]) {
    const char *circuit = NULL;

    if (argc < 2) {   // si on a pas dde param on prend le dernier circuit
        circuit = dernier_circuit();
        if (!circuit) {
            printf("Aucun circuit \n");
            return 1;
        }
        printf("cicruit : %s\n", circuit);
    } else {
        circuit = argv[1];
        creer_dossier(circuit);
        ajouter_circuit(circuit);
    }

    int etape_actuelle = charger_etat(circuit);

    if (etape_actuelle < 3) {
        printf("essis n°%d sur %s ",etape_actuelle+1,circuit);
        essais(etape_actuelle);
        sauvegarder_etat(circuit, etape_actuelle+1);
    }
    else if (etape_actuelle < 5){
        printf("qualif n°%d sur %s ",etape_actuelle-2,circuit);
        qualifications(etape_actuelle-2);
        sauvegarder_etat(circuit, etape_actuelle+1);
    }
    else if (etape_actuelle == 6){
        printf("Course final %s \n", circuit);
        course();
        sauvegarder_etat(circuit, etape_actuelle+1);
    }

    else if (etape_actuelle==7){
        printf("le week end de %s est fini veuillez lancer un autre circuit",circuit);
    }

// Exemple de fonctions (vides pour l'instant)
void essais(int etape) {
    printf("Essais %d en cours...\n", etape);
    // Logique des essais
}

void qualifications() {
    printf("Qualifications en cours...\n");
    // Logique des qualifications
}

void course() {
    printf("Course en cours...\n");
    // Logique de la course
}
}
