#ifndef CAR_H
#define CAR_H

#include <sys/types.h>

// Nombre maximum de voitures (peut être 20)
#define MAX_CARS 20

// État possible de la voiture
typedef enum {
    CAR_RUNNING,
    CAR_PIT,
    CAR_OUT
} CarStatus;

// Structure de la voiture
typedef struct {
    pid_t pid;          // PID du processus associé à la voiture
    int carNumber;      // Numéro de la voiture
    CarStatus status;   // État actuel (en piste, pit, out)
    double bestLapTime; // Meilleur temps au tour
    double bestS1;      // Meilleur temps secteur 1
    double bestS2;      // Meilleur temps secteur 2
    double bestS3;      // Meilleur temps secteur 3
} Car;

// Prototypes
void run_car_process(int carIndex);
double generate_sector_time(int sector);
void handle_pit_stop();
void handle_abandon();

#endif /* CAR_H */
