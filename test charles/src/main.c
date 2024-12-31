#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "f1shared.h"
#include "car.h"
#include "race.h"
#include "rwcourtois.h"
#include "utils.h"

// Nouveau main.c pour le projet F1 + Courtois lecteurs/écrivains.
// Pas de référence à GPState ou SESS_FINISHED ici : on utilise F1Shared et les fonctions du "race" actuel.

int main(void)
{
    // 1) Initialiser l'algorithme de Courtois (lecteurs/écrivains)
    rw_init();

    // 2) Créer la mémoire partagée (F1Shared)
    int shmid = create_shm();
    F1Shared *f1 = attach_shm(shmid);

    // 3) Charger la longueur d'un circuit (ex: circuit #1) depuis circuits.csv
    //    pour la course de 300 km
    load_circuit_length("circuits.csv", 1, f1);

    // 4) Lancer la séance d'essais (1h)
    run_essai_1h(f1);

    // 5) Qualifications : Q1, Q2, Q3
    run_qualifQ1(f1);
    run_qualifQ2(f1);
    run_qualifQ3(f1);

    // 6) Course de 300 km (calcul du nb de tours en fonction de la longueur)
    run_course(f1);

    // 7) Détacher et supprimer la mémoire partagée
    detach_shm(f1);
    remove_shm(shmid);

    // 8) Nettoyer l'algo lecteurs/écrivains
    rw_cleanup();

    return 0;
}
