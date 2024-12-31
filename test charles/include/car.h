#ifndef CAR_H
#define CAR_H

#include <sys/types.h>

#define MAX_CARS 20

// État possible d’une voiture
typedef enum {
    CAR_RUNNING,
    CAR_PIT,
    CAR_OUT
} CarStatus;

// Structure d’une voiture
typedef struct {
    pid_t       pid;         // PID du process
    int         carNumber;   // Numéro F1 (ex: 1, 44, 16, etc.)
    CarStatus   status;      // CAR_RUNNING, CAR_PIT, CAR_OUT
    double      bestLap;     // Meilleur tour (une simple variable)
    double      totalTime;   // Temps total en course (simplifié)

    int         points;      // Points au championnat
} Car;

// Process voiture
void run_car_process(int carIndex);

// Simulations
double generate_sector_time(int sector);
void handle_pit_stop();
void handle_abandon();

#endif /* CAR_H */
