#ifndef RACE_H
#define RACE_H

#include "car.h"

// Structure pour gérer l'information globale de la course
typedef struct {
    int numberOfCars;
    Car cars[MAX_CARS];    
    // Ajoutez ici d'autres infos sur la course, ex. nombre de tours
    int totalLaps;       // Nombre de tours
    int currentLap;      // Tour en cours
    // ...
} Race;

// Prototypes
int init_race(Race *race, int nbCars, int totalLaps);
void start_race(Race *race);
void update_classification(Race *race);
void display_classification(const Race *race);
void save_results(const Race *race, const char *filename);

#endif /* RACE_H */
