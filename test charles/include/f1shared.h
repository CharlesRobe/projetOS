#ifndef F1SHARED_H
#define F1SHARED_H

#include "car.h"
#include "circuit.h"

#define SHM_KEY 0xABCD1234

typedef struct {
    Car cars[MAX_CARS];
    int carsFinished;
    int nbCarsActive;
    double circuitLength;
} F1Shared;

int create_shm();
F1Shared* attach_shm(int shmid);
void detach_shm(F1Shared *f1);
void remove_shm(int shmid);

#endif /* F1SHARED_H */
