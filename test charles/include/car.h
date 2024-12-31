#ifndef CAR_H
#define CAR_H

#define MAX_CARS 20

typedef enum {
    CAR_RUNNING,
    CAR_PIT,
    CAR_OUT
} CarStatus;

typedef struct {
    int   carNumber;
    CarStatus status;
    double bestLap;
    double bestS1, bestS2, bestS3;
    double remaining; 
    int lapsDone;
} Car;

// Process voiture (écrivain)
void run_car_process(int idxCar);

#endif /* CAR_H */
