#include "car.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// On envoie s1,s2,s3, + un "pit?" + un "out?"
void run_car_process(int indexCar, int writeFd)
{
    srand(time(NULL) ^ (getpid()<<16));

    while(1) {
        // ~3% out
        if((rand()%100)<3){
            // s1=-1 => out
            double outVal=-1.0;
            write(writeFd, &outVal, sizeof(double)); // s1
            write(writeFd, &outVal, sizeof(double)); // s2
            write(writeFd, &outVal, sizeof(double)); // s3
            // pit=0
            double pitVal=0.0;
            write(writeFd, &pitVal, sizeof(double));
            usleep(100000); // un mini-delai
            _exit(0);
        }

        double s1 = 25 + (rand()%21) + (rand()%1000)/1000.0;
        double s2 = 25 + (rand()%21) + (rand()%1000)/1000.0;
        double s3 = 25 + (rand()%21) + (rand()%1000)/1000.0;

        // pit 5%
        double pitVal=0.0;
        if((rand()%100)<5){
            pitVal=1.0; // pit
            // On simule +25 s => on n’envoie pas forcément, mais ...
            // On dort 2 s
            sleep(2);
        }

        // On envoie s1, s2, s3, pitVal
        write(writeFd, &s1, sizeof(double));
        write(writeFd, &s2, sizeof(double));
        write(writeFd, &s3, sizeof(double));
        write(writeFd, &pitVal, sizeof(double));

        // 0.5 s entre chaque "tour"
        usleep(500000);
    }
}
