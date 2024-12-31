#ifndef RACE_H
#define RACE_H

#include "car.h"
#include "circuit.h"

typedef enum {
    STAGE_NONE = 0,
    STAGE_P1,
    STAGE_P2,
    STAGE_P3,
    STAGE_Q1,
    STAGE_Q2,
    STAGE_Q3,
    STAGE_SPRINT,
    STAGE_RACE
} Stage;

// Informations sur un Grand Prix donné
typedef struct {
    Circuit circuit;   // Infos sur le circuit
    int     isSpecial; // 0 = normal, 1 = sprint
    int     nbToursCourse; // Nombre de tours (dimanche)
    int     nbToursSprint; // Nombre de tours (samedi sprint)
} GrandPrix;

typedef struct {
    Car cars[MAX_CARS];

    // Infos sur la session en cours
    Stage currentStage;

    // Nombre total de GP
    int   nbGP;
    // Index du GP en cours (0..nbGP-1)
    int   currentGP;
    GrandPrix gpList[MAX_CIRCUITS]; // On stocke jusqu’à 30 GP max

    // Classement général (points cumulés, etc.)
    // -> On peut juste stocker dans Car.points
} Championship;

// Fonctions principales
void init_championship(Championship *champ, Circuit circuits[], int nbCircuits);
void run_current_session(Championship *champ);

// Pour enchaîner les étapes d’un GP
void next_stage(Championship *champ);
void start_stage(Championship *champ, Stage stage);
void end_stage(Championship *champ, Stage stage);

// Affichage & classement
void display_classification(const Championship *champ);
void update_classification(Championship *champ);
void save_results_session(const Championship *champ, const char *filename);
void save_championship(const Championship *champ, const char *filename);

#endif /* RACE_H */
