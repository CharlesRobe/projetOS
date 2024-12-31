#ifndef RACE_H
#define RACE_H

#include "car.h"
#include "circuit.h"

typedef enum {
    SESS_NONE = 0,
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
    Circuit      circuit;
    char         gpName[64];
    int          isSprint;       // 0=classique, 1=special
    SessionType  currentSession;
    Car          cars[MAX_CARS];
} GPState;

// Initialise un nouveau GP en donnant le numéro du circuit, plus "isSprint"
void init_new_gp(GPState *gp,
                 int circuitNumber,
                 int isSprint,
                 Circuit circuits[],
                 int nbCirc);

// Lance la session courante
void run_session(GPState *gp);

// Avance la session
void next_session(GPState *gp);

// Fin de session -> attribuer points, etc.
void end_session(GPState *gp);

// Trie par bestLap
void sort_cars_by_bestlap(GPState *gp);

// Affiche un tableau live ou final
void display_classification(const GPState *gp, const char *title, int liveMode);

// Sauvegarde finale
void save_final_classification(const GPState *gp, const char *filename);

#endif /* RACE_H */
