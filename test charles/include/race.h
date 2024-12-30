#ifndef RACE_H
#define RACE_H

#include "car.h"

// Différents « stades » d’un week-end
typedef enum {
    STAGE_P1,
    STAGE_P2,
    STAGE_P3,
    STAGE_Q1,
    STAGE_Q2,
    STAGE_Q3,
    STAGE_SPRINT,
    STAGE_RACE,
    STAGE_NONE
} Stage;

// Structure principale : gère l’état global du week-end
typedef struct {
    int  numberOfCars;        // Toujours 20
    Car  cars[MAX_CARS];

    Stage currentStage;       // P1, P2, P3, Q1, Q2, Q3, SPRINT, RACE…
    int   totalLaps;          // Nombre de tours pour la course
    int   currentLap;         // Tour en cours (course)
    int   sprintLaps;         // Nombre de tours pour la course sprint

    int   isSprintWeekend;    // 0 = classique, 1 = sprint
} Race;

// Fonctions de gestion du week-end
int  init_race(Race *race, int isSprintWeekend);
void manage_weekend(Race *race);

// Fonctions de session
void start_session(Race *race, Stage sessionStage);
void end_session(Race *race, Stage sessionStage);

// Mise à jour, affichage et sauvegarde
void update_classification(Race *race, Stage sessionStage);
void display_classification(const Race *race, Stage sessionStage);
void save_results(const Race *race, Stage sessionStage, const char *filename);

#endif /* RACE_H */
