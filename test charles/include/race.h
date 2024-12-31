#ifndef RACE_H
#define RACE_H

#include "car.h"
#include "circuit.h"

// Les différentes sessions
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

// Structure globale pour un seul GP
typedef struct {
    Circuit   circuit;       // Infos circuit
    char      gpName[64];    // Nom (ex: "Bahrain2025")
    int       isSprint;      // 0=classique, 1=sprint
    SessionType currentSession;
    Car       cars[MAX_CARS];
} GrandPrixWeekend;

// Initialise un nouveau GP
// Cherche si gpName existe dans circuits[], sinon prend circuit[0]
void init_new_gp(GrandPrixWeekend *gpw,
                 const char *gpName,
                 int isSprint,
                 Circuit circuits[],
                 int nbC);

// Lance la session courante => fork 20 voitures, etc.
void run_session(GrandPrixWeekend *gpw);

// Passe à la session suivante
void next_session(GrandPrixWeekend *gpw);

// Affecte les points (sprint, course)
void end_session(GrandPrixWeekend *gpw);

// Affiche un "beau tableau" en ASCII
void display_classification(const GrandPrixWeekend *gpw, const char *title);

// Tri local par bestLap
void sort_cars_by_bestlap(GrandPrixWeekend *gpw);

// Sauvegarde le classement final du week-end (quand SESS_FINISHED) dans un fichier
void save_final_classification(const GrandPrixWeekend *gpw, const char *filename);

#endif /* RACE_H */
