#include "race.h"
#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>

// Numéros de voiture (statiques)
static int carNumbers[MAX_CARS] = {
  1, 11, 44, 63, 16, 55, 4, 81, 14, 18,
  10, 31, 23, 2, 22, 3, 77, 24, 20, 27
};

// Initialise le championnat avec la liste de circuits
void init_championship(Championship *champ, Circuit circuits[], int nbCircuits)
{
    // On suppose qu’on veut faire nbCircuits GPs, ou 22 si vous préférez
    champ->nbGP = nbCircuits;
    if (champ->nbGP > MAX_CIRCUITS) {
        champ->nbGP = MAX_CIRCUITS;
    }

    // On recopie les circuits dans champ->gpList
    for (int i = 0; i < champ->nbGP; i++) {
        champ->gpList[i].circuit = circuits[i];
        // Exemple : si le numéro de course est multiple de 3, on dit que c’est « special »
        // A adapter selon vos besoins
        champ->gpList[i].isSpecial = ((i+1) % 3 == 0) ? 1 : 0;

        // On calcule un nombre de tours en fonction de la longueur
        // ex: environ 300 km => tours = ceil(300 / longueurCircuit)
        int toursDimanche = (int)ceil(300.0 / champ->gpList[i].circuit.longueur);
        int toursSprint   = (int)ceil(100.0 / champ->gpList[i].circuit.longueur);

        champ->gpList[i].nbToursCourse = toursDimanche;
        champ->gpList[i].nbToursSprint = toursSprint;
    }

    champ->currentGP = 0;
    champ->currentStage = STAGE_NONE;

    // Init des voitures
    for (int i=0; i<MAX_CARS; i++) {
        champ->cars[i].pid       = 0;
        champ->cars[i].carNumber = carNumbers[i];
        champ->cars[i].status    = CAR_RUNNING;
        champ->cars[i].bestLap   = 9999.0;
        champ->cars[i].totalTime = 0.0;
        champ->cars[i].points    = 0;
    }
}

// Lance la session courante (fork 20 voitures, etc.)
void run_current_session(Championship *champ)
{
    Stage st = champ->currentStage;
    if (st == STAGE_NONE) {
        return;
    }

    // Forker 20 voitures
    for (int i=0; i<MAX_CARS; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            error_exit("fork");
        } else if (pid == 0) {
            // Enfant
            run_car_process(champ->cars[i].carNumber);
        } else {
            // Parent
            champ->cars[i].pid = pid;
        }
    }

    // On "laisse rouler" la session pendant X secondes
    // (ex: 10 secondes pour P1, P2, P3, Q1, Q2, Q3 -> sprint/course un peu plus ?)
    // Pour simplifier, on met 8s pour P1..P3, 6s pour Q1..Q3, 5s pour sprint, 10s pour race ...
    int duration = 8;
    switch(st) {
        case STAGE_Q1:
        case STAGE_Q2:
        case STAGE_Q3:
            duration = 6;
            break;
        case STAGE_SPRINT:
            duration = 5;
            break;
        case STAGE_RACE:
            duration = 10;
            break;
        default:
            duration = 8;
    }

    // On fait un refresh console toutes les 2s
    int steps = duration * (1000/2000); // 2s
    for (int s=0; s<steps; s++) {
        printf("===[ Parent refresh session %d ]===\n", st);
        sleep(2);
    }

    // Fin de la session => kill les enfants
    for (int i=0; i<MAX_CARS; i++) {
        if (champ->cars[i].pid > 0) {
            kill(champ->cars[i].pid, SIGTERM);
        }
    }

    // Wait
    for (int i=0; i<MAX_CARS; i++) {
        if (champ->cars[i].pid > 0) {
            waitpid(champ->cars[i].pid, NULL, 0);
            champ->cars[i].pid = 0;
        }
    }

    // On met à jour le classement (trier par bestLap par ex.)
    update_classification(champ);

    // On affiche
    display_classification(champ);

    // Sauvegarde
    char filename[64];
    sprintf(filename, "results_GP%d_%d.txt", (champ->currentGP+1), st);
    save_results_session(champ, filename);
}

// Passe à l’étape suivante : P1->P2->P3->Q1->Q2->Q3->(SPRINT)->RACE->(fin)
void next_stage(Championship *champ)
{
    if (champ->currentStage == STAGE_NONE) {
        champ->currentStage = STAGE_P1;
        return;
    }

    switch(champ->currentStage) {
        case STAGE_P1:
            champ->currentStage = STAGE_P2;
            break;
        case STAGE_P2:
            champ->currentStage = STAGE_P3;
            break;
        case STAGE_P3:
            champ->currentStage = STAGE_Q1;
            break;
        case STAGE_Q1:
            champ->currentStage = STAGE_Q2;
            break;
        case STAGE_Q2:
            champ->currentStage = STAGE_Q3;
            break;
        case STAGE_Q3:
            if (champ->gpList[champ->currentGP].isSpecial == 1) {
                champ->currentStage = STAGE_SPRINT;
            } else {
                champ->currentStage = STAGE_RACE;
            }
            break;
        case STAGE_SPRINT:
            champ->currentStage = STAGE_RACE;
            break;
        case STAGE_RACE:
            champ->currentStage = STAGE_NONE; // GP terminé
            break;
        default:
            champ->currentStage = STAGE_NONE;
            break;
    }
}

// Lance un stage
void start_stage(Championship *champ, Stage stage)
{
    champ->currentStage = stage;
}

// Termine un stage
void end_stage(Championship *champ, Stage stage)
{
    // On peut y gérer l’attribution de points, si c’est la course sprint, etc.
    // Barème de points (simplifié) :
    // - Sprint : top 8 (8,7,6,5,4,3,2,1)
    // - Course : top 10 (25,20,15,10,8,6,5,3,2,1)
    // - Meilleur tour : +1 point si top 10
    if (stage == STAGE_SPRINT) {
        // Donner points aux 8 premiers
        int sprintPoints[8] = {8,7,6,5,4,3,2,1};
        for (int i=0; i<8; i++) {
            champ->cars[i].points += sprintPoints[i];
        }
    }
    else if (stage == STAGE_RACE) {
        int racePoints[10] = {25,20,15,10,8,6,5,3,2,1};
        for (int i=0; i<10; i++) {
            champ->cars[i].points += racePoints[i];
        }
        // +1 point au meilleur tour dans le top 10
        // Ici, c’est simplifié, on considère que le plus bas "bestLap" dans le top 10 => meilleur tour
        // En vrai, on distinguerait "meilleur tour" d’un seul passage. On va faire un mini-simpli
        // => Car [0] c’est celui qui a le bestLap le plus petit
        champ->cars[0].points += 1; 
    }
}

// Mise à jour du classement "local" (après Q3, Sprint, Race, etc.)
void update_classification(Championship *champ)
{
    // On trie par bestLap (plus petit = meilleur)
    for (int i=0; i<MAX_CARS-1; i++) {
        for (int j=i+1; j<MAX_CARS; j++) {
            if (champ->cars[j].bestLap < champ->cars[i].bestLap) {
                Car tmp = champ->cars[i];
                champ->cars[i] = champ->cars[j];
                champ->cars[j] = tmp;
            }
        }
    }
}

// Affiche la liste des voitures dans l’ordre actuel
void display_classification(const Championship *champ)
{
    printf("\n=== Classement provisoire (GP %d, stage=%d) ===\n",
           champ->currentGP+1,
           champ->currentStage);
    for (int i=0; i<MAX_CARS; i++) {
        printf("%2d) Car #%2d | bestLap=%.3f | Points=%d\n",
               i+1,
               champ->cars[i].carNumber,
               champ->cars[i].bestLap,
               champ->cars[i].points);
    }
}

// Sauvegarde le résultat de la session dans un fichier
void save_results_session(const Championship *champ, const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return;
    }

    fprintf(f, "GP #%d - Stage=%d\n", champ->currentGP+1, champ->currentStage);
    for (int i=0; i<MAX_CARS; i++) {
        fprintf(f, "%2d) Car #%2d | bestLap=%.3f | Points=%d\n",
               i+1,
               champ->cars[i].carNumber,
               champ->cars[i].bestLap,
               champ->cars[i].points);
    }

    fclose(f);
}

// Sauvegarde le championnat (classement général) dans un fichier
void save_championship(const Championship *champ, const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return;
    }

    // Trier par points pour un vrai classement final
    Car sorted[MAX_CARS];
    for (int i=0; i<MAX_CARS; i++) {
        sorted[i] = champ->cars[i];
    }
    // Tri descendant par points
    for (int i=0; i<MAX_CARS-1; i++) {
        for (int j=i+1; j<MAX_CARS; j++) {
            if (sorted[j].points > sorted[i].points) {
                Car tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }

    fprintf(f, "=== Classement final du championnat ===\n");
    for (int i=0; i<MAX_CARS; i++) {
        fprintf(f, "%2d) Car #%2d | Points=%d | bestLap=%.3f\n",
               i+1,
               sorted[i].carNumber,
               sorted[i].points,
               sorted[i].bestLap);
    }

    fclose(f);
}
