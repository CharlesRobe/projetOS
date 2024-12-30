#include "race.h"
#include "utils.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

// Initialiser la course
int init_race(Race *race, int nbCars, int totalLaps)
{
    race->numberOfCars = nbCars;
    race->totalLaps = totalLaps;
    race->currentLap = 0;

    // Initialiser les structures Car
    // Normalement, vous avez la liste des numéros (1, 11, 44, etc.)
    int carNumbers[20] = {1, 11, 44, 63, 16, 55, 4, 81, 14, 18, 10, 31, 23, 2, 22, 3, 77, 24, 20, 27};

    for (int i = 0; i < nbCars; i++) {
        race->cars[i].pid = 0;
        race->cars[i].carNumber = carNumbers[i];
        race->cars[i].status = CAR_RUNNING;
        race->cars[i].bestLapTime = 9999.0;  // init "grand"
        race->cars[i].bestS1 = 9999.0;
        race->cars[i].bestS2 = 9999.0;
        race->cars[i].bestS3 = 9999.0;
    }

    return 0; // succès
}

void start_race(Race *race)
{
    // Forker un process par voiture
    for (int i = 0; i < race->numberOfCars; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            error_exit("Erreur fork");
        }
        else if (pid == 0) {
            // Processus enfant
            run_car_process(i);
            // Jamais revenir ici, le run_car_process() se termine avec _exit()
        } else {
            // Processus parent
            race->cars[i].pid = pid;
        }
    }

    // Le parent (race manager) doit maintenant "gérer" la course :
    // - Recevoir les infos des enfants (via pipe ou mémoire partagée)
    // - Mettre à jour la classification
    // - Afficher régulièrement

    // Ex. simple : on attend la fin de tous les enfants (dans la vraie version,
    // on ferait un while(1) avec une mise à jour régulière)
    for (int i = 0; i < race->numberOfCars; i++) {
        wait(NULL);
    }

    // La course est terminée
}

void update_classification(Race *race)
{
    // Ex. : trier le tableau race->cars par bestLapTime
    // ou par un autre critère (distance parcourue, etc.)
    // Ceci est un tri basique par insertion : à adapter !

    for (int i = 0; i < race->numberOfCars - 1; i++) {
        for (int j = i+1; j < race->numberOfCars; j++) {
            if (race->cars[j].bestLapTime < race->cars[i].bestLapTime) {
                // swap
                Car temp = race->cars[i];
                race->cars[i] = race->cars[j];
                race->cars[j] = temp;
            }
        }
    }
}

void display_classification(const Race *race)
{
    // Affiche le classement actuel
    printf("Classement provisoire:\n");
    for (int i = 0; i < race->numberOfCars; i++) {
        printf("%2d) Car #%d - BestLap: %.3f - Status: %d\n", 
               i+1,
               race->cars[i].carNumber, 
               race->cars[i].bestLapTime,
               race->cars[i].status);
    }
}

void save_results(const Race *race, const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return;
    }

    fprintf(f, "Classement final:\n");
    for (int i = 0; i < race->numberOfCars; i++) {
        fprintf(f, "%2d) Car #%d - BestLap: %.3f\n",
                i+1,
                race->cars[i].carNumber,
                race->cars[i].bestLapTime);
    }

    fclose(f);
}
