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
    pid_t pid;          // PID du process
    int   carNumber;    // Ex: 1, 44, etc.
    CarStatus status;
    double bestLap;     // Le meilleur tour
    int    points;      // Points accumulés
} Car;

// Process enfant
void run_car_process(int carIndex);

double generate_sector_time(int sector);
void handle_pit_stop();
void handle_abandon();

#endif /* CAR_H */
