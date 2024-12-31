#ifndef RACE_H
#define RACE_H

#include "car.h"
#include "circuit.h"

// Sessions
typedef enum {
    SESS_NONE=0,
    SESS_P1,
    SESS_P2,
    SESS_P3,
    SESS_Q1,
    SESS_Q2,
    SESS_Q3,
    SESS_SPRINT,
    SESS_RACE,
    SESS_FINISHED
} SessionType;

/*
  Structure principale d'un GP.
  On va stocker :
  - le circuit choisi
  - le nom du GP (ex: "Bahrain2025")
  - isSpecial = 1 si sprint
  - la session en cours
  - 20 voitures
*/
typedef struct {
    Circuit      circuit;
    char         gpName[64];
    int          isSprint;
    SessionType  currentSession;

    Car          cars[MAX_CARS];
} GPState;

// Initialise un nouveau GP, cherchant <gpName> dans circuits[]
void init_new_gp(GPState *gp,
                 const char *gpName,
                 int isSprint,
                 Circuit circuits[],
                 int nbCirc);

// Lance la session courante (fork 20 voitures, pipe, etc.)
void run_session(GPState *gp);

// Avance la session (P1->P2->P3->Q1->Q2->Q3->SPRINT->RACE->FINISHED)
void next_session(GPState *gp);

// Donne les points appropriés à la fin de la session
void end_session(GPState *gp);

// Tri par bestLap, + affichage “live”
void sort_cars_by_bestlap(GPState *gp);
void display_classification(const GPState *gp, const char *title, int liveMode);

// Sauvegarde finale dans "final_result.txt" quand SESS_FINISHED
void save_final_classification(const GPState *gp, const char *filename);

#endif /* RACE_H */
