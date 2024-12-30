#include "race.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // Usage: ./f1_sim <nbVoitures> <0|1 pour sprint>
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <nbCars> <isSprintWeekend>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int nbCars = atoi(argv[1]);
    int sprint = atoi(argv[2]);

    Race race;
    if (init_race(&race, nbCars, sprint) != 0) {
        error_exit("Erreur init_race");
    }

    // Enchaînement du week-end
    manage_weekend(&race);

    printf("\n>>> Week-end terminé !\n");

    return 0;
}
