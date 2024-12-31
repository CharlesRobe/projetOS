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
  Chaque voiture stocke :
  - un PID (process fork)
  - son numéro (1, 44, etc.)
  - son état (running, pit, out)
  - son meilleur tour (bestLap)
  - ses points
*/
typedef struct {
    pid_t pid;
    int   carNumber;
    CarStatus status;
    double bestLap;
    int    points;
} Car;

// Process enfant : reçoit un pipe pour communiquer ses temps au parent
void run_car_process(int indexCar, int writeFd);

double generate_sector_time(int sector);
void handle_pit_stop();
void handle_abandon();

#endif /* CAR_H */
