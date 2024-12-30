#include "race.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // Exemple d’utilisation
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <nbCars> <totalLaps>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int nbCars = atoi(argv[1]);
    int totalLaps = atoi(argv[2]);

    if (nbCars > MAX_CARS || nbCars <= 0) {
        fprintf(stderr, "Nombre de voitures invalide (max %d)\n", MAX_CARS);
        exit(EXIT_FAILURE);
    }

    Race myRace;
    if (init_race(&myRace, nbCars, totalLaps) != 0) {
        error_exit("Erreur init_race");
    }

    // Lancement de la course (et forking des voitures)
    start_race(&myRace);

    // Mettre à jour le classement (par exemple, après la fin de la course)
    update_classification(&myRace);

    // Afficher le classement final
    display_classification(&myRace);

    // Sauvegarder les résultats
    save_results(&myRace, "results.txt");

    return 0;
}
