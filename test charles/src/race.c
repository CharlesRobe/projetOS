#include "race.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Liste (statique) des numéros de voiture (toujours 20)
static int carNumbers[MAX_CARS] = {
    1, 11, 44, 63, 16, 55, 4, 81, 14, 18,
    10, 31, 23, 2, 22, 3, 77, 24, 20, 27
};

int init_race(Race *race, int isSprintWeekend)
{
    // On force 20 voitures
    race->numberOfCars = MAX_CARS;
    race->currentStage = STAGE_NONE;
    race->totalLaps    = 50;   // Exemple : 50 tours
    race->currentLap   = 0;
    race->sprintLaps   = 20;   // Exemple : 20 tours pour la course sprint
    race->isSprintWeekend = isSprintWeekend;

    // Init des voitures
    for (int i = 0; i < MAX_CARS; i++) {
        race->cars[i].pid       = 0;
        race->cars[i].carNumber = carNumbers[i];
        race->cars[i].status    = CAR_RUNNING;
        for (int p = 0; p < 3; p++) {
            race->cars[i].bestTimePractice[p] = 9999.0;
            race->cars[i].bestTimeQualif[p]   = 9999.0;
        }
        race->cars[i].bestTimeRace = 9999.0;
        race->cars[i].points       = 0;
    }
    return 0;
}

/*
   manage_weekend : enchaîne les sessions
   - Classique : P1 -> P2 -> P3 -> Q1 -> Q2 -> Q3 -> RACE
   - Sprint : P1 -> Q1 -> Q2 -> Q3 -> SPRINT -> Q1 -> Q2 -> Q3 -> RACE
   (code simplifié pour illustration)
*/
void manage_weekend(Race *race)
{
    if (!race->isSprintWeekend) {
        // Week-end classique
        start_session(race, STAGE_P1);
        end_session(race,   STAGE_P1);

        start_session(race, STAGE_P2);
        end_session(race,   STAGE_P2);

        start_session(race, STAGE_P3);
        end_session(race,   STAGE_P3);

        start_session(race, STAGE_Q1);
        end_session(race,   STAGE_Q1);

        start_session(race, STAGE_Q2);
        end_session(race,   STAGE_Q2);

        start_session(race, STAGE_Q3);
        end_session(race,   STAGE_Q3);

        start_session(race, STAGE_RACE);
        end_session(race,   STAGE_RACE);
    } else {
        // Week-end sprint
        start_session(race, STAGE_P1);
        end_session(race,   STAGE_P1);

        // Qualifications pour sprint
        start_session(race, STAGE_Q1);
        end_session(race,   STAGE_Q1);

        start_session(race, STAGE_Q2);
        end_session(race,   STAGE_Q2);

        start_session(race, STAGE_Q3);
        end_session(race,   STAGE_Q3);

        // Course sprint
        start_session(race, STAGE_SPRINT);
        end_session(race,   STAGE_SPRINT);

        // Re-qualifications pour la course du dimanche
        start_session(race, STAGE_Q1);
        end_session(race,   STAGE_Q1);

        start_session(race, STAGE_Q2);
        end_session(race,   STAGE_Q2);

        start_session(race, STAGE_Q3);
        end_session(race,   STAGE_Q3);

        // Course principale
        start_session(race, STAGE_RACE);
        end_session(race,   STAGE_RACE);
    }
}

// Lance une session
void start_session(Race *race, Stage sessionStage)
{
    race->currentStage = sessionStage;
    printf("\n=== Début de la session %d ===\n", sessionStage);

    // Forker un process par voiture
    for (int i = 0; i < race->numberOfCars; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            error_exit("fork");
        } else if (pid == 0) {
            // Enfant
            run_car_process(i);
            // Jamais revenir
        } else {
            // Parent
            race->cars[i].pid = pid;
        }
    }

    // Dans un vrai code : on récupère régulièrement les temps, on met à jour…
    // Ici, simplification : on attend 5s pour la session
    sleep(5);
}

// Termine la session : on tue les process enfants et on génère un classement
void end_session(Race *race, Stage sessionStage)
{
    printf("=== Fin de la session %d ===\n", sessionStage);

    // On tue les enfants
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

    // Mise à jour du classement
    update_classification(race, sessionStage);

    // Affichage
    display_classification(race, sessionStage);

    // Sauvegarde
    char filename[64];
    sprintf(filename, "results_stage_%d.txt", sessionStage);
    save_results(race, sessionStage, filename);
}

void update_classification(Race *race, Stage sessionStage)
{
    // Dans un vrai code, on trierait par meilleur temps obtenu
    // ou distance parcourue. Ici, on fait un tri simple par carNumber.
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
