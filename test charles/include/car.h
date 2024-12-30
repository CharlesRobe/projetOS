#ifndef CAR_H
#define CAR_H

#include <sys/types.h>

#define MAX_CARS 20

// État d’une voiture
typedef enum {
    CAR_RUNNING,
    CAR_PIT,
    CAR_OUT
} CarStatus;

// Structure contenant les données d’une voiture
typedef struct {
    pid_t pid;            // PID du process associé
    int   carNumber;      // Numéro de la voiture (ex: 1, 11, 44, ...)
    CarStatus status;     // CAR_RUNNING, CAR_PIT ou CAR_OUT

    // Meilleurs temps par session (P1, P2, P3, Q1, Q2, Q3, Course…)
    double bestTimePractice[3];   // P1,P2,P3
    double bestTimeQualif[3];     // Q1,Q2,Q3
    double bestTimeRace;          // Pour la course (simplifié)

    // Points cumulés pour le championnat
    int points;
} Car;

// Fonctions associées à la voiture (gérées dans car.c)
void run_car_process(int carIndex);
double generate_sector_time(int sector);
void handle_pit_stop();
void handle_abandon();

#endif /* CAR_H */
