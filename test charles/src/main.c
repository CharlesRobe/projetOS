#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "f1shared.h"
#include "race.h"
#include "rwcourtois.h"
#include "utils.h"

// Prototypes
void load_circuit_length(const char *filename, int circuitNum, F1Shared *f1);

int main(int argc, char *argv[])
{
    // Vérifier les arguments
    if(argc < 2){
        printf("Usage: %s <NomGP>\n", argv[0]);
        return 1;
    }

    // Nom du GP
    char gpName[128];
    strncpy(gpName, argv[1], sizeof(gpName)-1);
    gpName[sizeof(gpName)-1] = '\0';

    // Initialiser les sémaphores (Courtois)
    rw_init();

    // Créer et attacher la mémoire partagée
    int shmid = create_shm();
    F1Shared *f1 = attach_shm(shmid);

    // Charger la longueur du circuit (ex: circuit #1)
    load_circuit_length("circuits.csv", 1, f1);

    // Lancer les Essais Libres 1h
    run_essai_1h(f1);

    // Lancer les Qualifications Q1, Q2, Q3
    run_qualifQ1(f1);
    run_qualifQ2(f1);
    run_qualifQ3(f1);

    // Lancer la Course de 300 km
    run_course(f1);

    // Détacher et supprimer la mémoire partagée
    detach_shm(f1);
    remove_shm(shmid);

    // Nettoyer les sémaphores
    rw_cleanup();

    return 0;
}
