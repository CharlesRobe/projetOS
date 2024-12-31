#ifndef RACE_H
#define RACE_H

#include "car.h"
#include "circuit.h"

typedef enum {
    SESS_NONE=0,
    SESS_ESSAI1,
    SESS_ESSAI2,
    SESS_ESSAI3,
    SESS_Q1,
    SESS_Q2,
    SESS_Q3,
    SESS_SPRINT,
    SESS_COURSE,
    SESS_FINISHED
} SessionType;

typedef struct {
    Circuit     circuit;
    char        gpName[64];
    int         isSprint;       // 0=classique, 1=special
    SessionType currentSession;

    // 20 voitures
    Car         cars[MAX_CARS];

    // Liste des "voitures éliminées" après Q1, Q2, etc.
    // ex. elimQ1[5], elimQ2[5], pour la grille
    int         elimQ1[5];
    int         elimQ2[5];

    // Nombre de tours prévus pour la sprint/course
    int         nbToursSprint;
    int         nbToursCourse;
} GPState;

// Initialise un GP (on donne un "numero de circuit" + isSprint)
void init_new_gp(GPState *gp, int circuitNumber, int isSprint,
                 Circuit circuits[], int nbCirc);

// Lance la session courante
void run_session(GPState *gp);

// Passe à la session suivante
void next_session(GPState *gp);

// Fin de session => points + gestion éliminations Q1/Q2
void end_session(GPState *gp);

// Tri par bestLap
void sort_cars_by_bestlap(GPState *gp);

// Affichage (live ou final)
void display_classification(const GPState *gp, const char *title, int liveMode);

// Sauvegarde finale
void save_final_classification(const GPState *gp, const char *filename);

#endif /* RACE_H */
