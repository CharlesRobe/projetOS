#include "race.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Liste (statique) des numéros de voitures
static int carNumbers[MAX_CARS] = {
  1, 11, 44, 63, 16, 55, 4, 81, 14, 18,
  10, 31, 23, 2, 22, 3, 77, 24, 20, 27
};

// Initialisation de la "Race"
int init_race(Race *race, int nbCars, int isSprintWeekend)
{
    if (nbCars > MAX_CARS) {
        fprintf(stderr, "Trop de voitures (%d), max = %d\n", nbCars, MAX_CARS);
        return -1;
    }

    race->numberOfCars = nbCars;
    race->currentStage = STAGE_NONE;
    race->totalLaps    = 50;  // Ex : 50 tours pour la course
    race->currentLap   = 0;
    race->sprintLaps   = 20;  // Ex : 20 tours pour le sprint
    race->isSprintWeekend = isSprintWeekend;

    // Init des voitures
    for (int i = 0; i < nbCars; i++) {
        race->cars[i].pid = 0;
        race->cars[i].carNumber = carNumbers[i];
        race->cars[i].status = CAR_RUNNING;
        for (int p = 0; p < 3; p++) {
            race->cars[i].bestTimePractice[p] = 9999.0;
            race->cars[i].bestTimeQualif[p]   = 9999.0;
        }
        race->cars[i].bestTimeRace = 9999.0;
        race->cars[i].points = 0;
    }
    return 0;
}

/* 
   manage_weekend : enchaîne les différentes sessions d’un GP
   Format classique : P1 -> P2 -> P3 -> Q1 -> Q2 -> Q3 -> RACE
   Format sprint    : P1 -> Q1 -> Q2 -> Q3 -> SPRINT -> Q1 -> Q2 -> Q3 -> RACE
   (Ici on fait un exemple "simplifié" pour la démo.)
*/
void manage_weekend(Race *race)
{
    // === CAS WEEK-END CLASSIQUE ===
    if (!race->isSprintWeekend) {
        start_session(race, STAGE_P1);
        end_session(race,   STAGE_P1);

        start_session(race, STAGE_P2);
        end_session(race,   STAGE_P2);

        start_session(race, STAGE_P3);
        end_session(race,   STAGE_P3);

        // Qualifications
        start_session(race, STAGE_Q1);
        end_session(race,   STAGE_Q1);

        start_session(race, STAGE_Q2);
        end_session(race,   STAGE_Q2);

        start_session(race, STAGE_Q3);
        end_session(race,   STAGE_Q3);

        // Course
        start_session(race, STAGE_RACE);
        end_session(race,   STAGE_RACE);
    }
    // === CAS WEEK-END SPRINT === (exemple simplifié)
    else {
        start_session(race, STAGE_P1);
        end_session(race,   STAGE_P1);

        // Qualifications (pour sprint)
        start_session(race, STAGE_Q1);
        end_session(race,   STAGE_Q1);
        start_session(race, STAGE_Q2);
        end_session(race,   STAGE_Q2);
        start_session(race, STAGE_Q3);
        end_session(race,   STAGE_Q3);

        // Sprint
        start_session(race, STAGE_SPRINT);
        end_session(race,   STAGE_SPRINT);

        // Nouvelles qualifications (pour la course du dimanche),
        // on réutilise Q1,Q2,Q3 pour l’exemple :
        start_session(race, STAGE_Q1);
        end_session(race,   STAGE_Q1);
        start_session(race, STAGE_Q2);
        end_session(race,   STAGE_Q2);
        start_session(race, STAGE_Q3);
        end_session(race,   STAGE_Q3);

        // Course
        start_session(race, STAGE_RACE);
        end_session(race,   STAGE_RACE);
    }
}

// Lance une session (ex : P1, P2, …, Q1, …)
void start_session(Race *race, Stage sessionStage)
{
    race->currentStage = sessionStage;
    printf("\n=== Début de la session %d ===\n", sessionStage);

    // Forquer chaque voiture
    for (int i = 0; i < race->numberOfCars; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            error_exit("fork");
        } else if (pid == 0) {
            // Enfant : on lance run_car_process
            run_car_process(i);
            // Jamais revenir ici
        } else {
            // Parent : on stocke le PID
            race->cars[i].pid = pid;
        }
    }

    // Le parent "gère" la session :
    // (Dans un vrai code, on ferait un while() qui reçoit les temps
    //  régulièrement, met à jour le classement, etc.)

    // Dans l’exemple, on se contente de "patienter" 5 secondes,
    // puis on tue les enfants pour simuler la fin de session
    sleep(5);
}

// Termine la session : on tue les process enfants et on génère un classement
void end_session(Race *race, Stage sessionStage)
{
    printf("=== Fin de la session %d ===\n", sessionStage);

    // On tue tous les enfants (on envoie un signal)
    for (int i = 0; i < race->numberOfCars; i++) {
        if (race->cars[i].pid > 0) {
            kill(race->cars[i].pid, SIGTERM);
        }
    }

    // On attend leur fin
    for (int i = 0; i < race->numberOfCars; i++) {
        if (race->cars[i].pid > 0) {
            waitpid(race->cars[i].pid, NULL, 0);
            race->cars[i].pid = 0;
        }
    }

    // On met à jour le classement
    update_classification(race, sessionStage);

    // On l’affiche
    display_classification(race, sessionStage);

    // On sauvegarde
    char filename[64];
    sprintf(filename, "results_stage_%d.txt", sessionStage);
    save_results(race, sessionStage, filename);
}

// Tri / mise à jour (simplifiée)
void update_classification(Race *race, Stage sessionStage)
{
    // Pour l’exemple, on ne fait pas grand-chose de concret,
    // car on ne reçoit pas vraiment les meilleurs temps en direct.
    // On peut imaginer un tri par "bestTimePractice[sessionStage]" ou autre.

    // (Ici, on ne tri pas vraiment, on fait juste un swap bidon.)
    // A adapter si vous gérez réellement les temps.

    // Ex : tri par carNumber :
    for (int i = 0; i < race->numberOfCars - 1; i++) {
        for (int j = i + 1; j < race->numberOfCars; j++) {
            if (race->cars[j].carNumber < race->cars[i].carNumber) {
                Car tmp = race->cars[i];
                race->cars[i] = race->cars[j];
                race->cars[j] = tmp;
            }
        }
    }
}

void display_classification(const Race *race, Stage sessionStage)
{
    printf("=== Classement (session %d) ===\n", sessionStage);
    for (int i = 0; i < race->numberOfCars; i++) {
        printf("Pos %2d | Car #%2d | Status=%d\n",
               i+1,
               race->cars[i].carNumber,
               race->cars[i].status);
    }
}

void save_results(const Race *race, Stage sessionStage, const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return;
    }
    fprintf(f, "=== Résultats de la session %d ===\n", sessionStage);
    for (int i = 0; i < race->numberOfCars; i++) {
        fprintf(f, "Pos %2d | Car #%2d | Status=%d\n",
                i+1,
                race->cars[i].carNumber,
                race->cars[i].status);
    }
    fclose(f);
}
