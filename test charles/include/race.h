#ifndef RACE_H
#define RACE_H

#include "car.h"

// Différents « stades » du week-end
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
    int numberOfCars;
    Car cars[MAX_CARS];

    Stage currentStage;  // P1, P2, P3, Q1, Q2, Q3, SPRINT, RACE
    int totalLaps;       // Nombre de tours si besoin (course)
    int currentLap;      // Tour en cours (course)
    int sprintLaps;      // Nombre de tours pour la course sprint (si existante)

    // Autres champs selon vos besoins :
    int isSprintWeekend; // 0 = week-end classique, 1 = sprint
} Race;

// Fonctions de gestion du week-end
int  init_race(Race *race, int nbCars, int isSprintWeekend);
void manage_weekend(Race *race);

// Fonctions de session (une session = P1 ou P2, Q1, Q2, ...)
void start_session(Race *race, Stage sessionStage);
void end_session(Race *race, Stage sessionStage);

// Mise à jour, affichage et sauvegarde
void update_classification(Race *race, Stage sessionStage);
void display_classification(const Race *race, Stage sessionStage);
void save_results(const Race *race, Stage sessionStage, const char *filename);

#endif /* RACE_H */
