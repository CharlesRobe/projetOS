#include "race.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // Usage: ./f1_sim <isSprintWeekend>
    // 0 = week-end classique
    // 1 = week-end sprint
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <0|1>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sprint = atoi(argv[1]);

    Race race;
    if (init_race(&race, sprint) != 0) {
        error_exit("Erreur init_race");
    }

    // Enchaînement du week-end (sessions)
    manage_weekend(&race);

    printf("\n>>> Week-end terminé !\n");
    return 0;
}
