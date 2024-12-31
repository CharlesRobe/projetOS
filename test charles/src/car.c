#include "car.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static double generate_sector_time(int sector)
{
    // 25..45
    double base = 25.0 + (rand()%21);
    double frac = (rand()%1000)/1000.0;
    return base + frac;
}

void run_car_process(int indexCar, int writeFd)
{
    srand(time(NULL) ^ (getpid()<<16));

    while(1) {
        // ~3% d'abandon
        if((rand()%100)<3) {
            printf("[Child-%d, PID=%d] ABANDON\n", indexCar, getpid());
            _exit(0);
        }

        // Calcule s1, s2, s3
        double s1 = generate_sector_time(1);
        double s2 = generate_sector_time(2);
        double s3 = generate_sector_time(3);

        // 5% PIT
        if((rand()%100)<5) {
            printf("[Child-%d, PID=%d] PIT STOP\n", indexCar, getpid());
            sleep(2);
        }

        // On envoie s1, s2, s3
        if(write(writeFd, &s1, sizeof(double))<0) {}
        if(write(writeFd, &s2, sizeof(double))<0) {}
        if(write(writeFd, &s3, sizeof(double))<0) {}

        usleep(500000); // 0.5s
    }
}

