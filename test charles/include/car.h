#ifndef CAR_H
#define CAR_H

#include <sys/types.h>

#define MAX_CARS 20

typedef enum {
    CAR_RUNNING,
    CAR_PIT,
    CAR_OUT
} CarStatus;

typedef struct {
    pid_t pid;
    int   carNumber;    // 1, 11, 44, ...
    CarStatus status;

    // Meilleur tour (cumul s1+s2+s3)
    double bestLap;

    // Meilleurs S1, S2, S3
    double bestS1;
    double bestS2;
    double bestS3;

    // Points totaux
    int    points;
} Car;

// L'enfant reçoit "indexCar" et un "writeFd" pour envoyer s1,s2,s3
void run_car_process(int indexCar, int writeFd);

#endif /* CAR_H */
