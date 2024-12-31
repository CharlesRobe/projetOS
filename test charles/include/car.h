#ifndef CAR_H
#define CAR_H

#include <sys/types.h>

#define MAX_CARS 20

typedef enum {
    CAR_RUNNING,
    CAR_PIT,
    CAR_OUT
} CarStatus;

/*
  Structure d'une voiture (processus).
  - bestLap : meilleur temps au tour
  - bestS1, bestS2, bestS3
  - points
*/
typedef struct {
    pid_t pid;
    int   carNumber;
    CarStatus status;

    // Meilleur tour (somme s1+s2+s3)
    double bestLap;

    // Meilleurs secteurs
    double bestS1;
    double bestS2;
    double bestS3;

    // Points cumulés
    int    points;
} Car;

// L'enfant : "indexCar", "writeFd" => envoie s1,s2,s3, ...
void run_car_process(int indexCar, int writeFd);

#endif /* CAR_H */
