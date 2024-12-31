#ifndef RACE_H
#define RACE_H

#include "car.h"
#include "circuit.h"

/* Les différentes sessions */
typedef enum {
    SESS_NONE = 0,
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
  Structure principale pour un seul GP.
  - circuit : infos du circuit (longueur, nom, pays...)
  - gpName : ex. "Monaco2025"
  - isSprint : 1 si GP au format sprint
  - currentSession : P1..P3, Q1..Q3, SPRINT ou RACE
  - cars : tableau de 20 voitures (pid, bestLap, points...)
*/
typedef struct {
    Circuit      circuit;
    char         gpName[64];
    int          isSprint;
    SessionType  currentSession;

    Car          cars[MAX_CARS];
} GPState;

/* Initialise un nouveau GP en cherchant <gpName> dans circuits[] */
void init_new_gp(GPState *gp,
                 const char *gpName,
                 int isSprint,
                 Circuit circuits[],
                 int nbCirc);

/* Lance la session courante (fork 20 voitures, pipes, etc.) */
void run_session(GPState *gp);

/* Passe à la session suivante (P1->P2->P3->Q1->Q2->Q3->(Sprint)->Race->Finished) */
void next_session(GPState *gp);

/* Attribue des points (sprint ou race) */
void end_session(GPState *gp);

/* Tri par bestLap */
void sort_cars_by_bestlap(GPState *gp);

/* 
   Affiche le classement dans un tableau ASCII.
   - title : un texte pour l’entête
   - liveMode : 1 = affichage en direct (pendant la session), 0 = final
*/
void display_classification(const GPState *gp, const char *title, int liveMode);

/* Sauvegarde finale quand tout est fini (classement global) */
void save_final_classification(const GPState *gp, const char *filename);

#endif /* RACE_H */
