#include "car.h"
#include "f1shared.h"
#include "rwcourtois.h"
#include "utils.h"
#include <sys/shm.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void run_car_process(int idxCar)
{
    int sid= shmget(SHM_KEY, sizeof(F1Shared),0);
    if(sid<0) error_exit("shmget car");
    F1Shared* f1= (F1Shared*) shmat(sid,NULL,0);
    if((void*)f1==(void*)-1) error_exit("shmat car");
    srand(time(NULL) ^ (getpid()<<16));

    while(1){
        // Crash 3%
        if((rand()%100)<3){
            ecrire_debut();
            f1->cars[idxCar].status= CAR_OUT;
            f1->carsFinished++;
            ecrire_fin();
            _exit(0);
        }
        double s1=25.0 + (rand() % 21); // 25.0 to 45.0
        double s2=25.0 + (rand() % 21);
        double s3=25.0 + (rand() % 21);
        double lapTime= s1 + s2 + s3;

        // Pit 5%
        if((rand()%100)<5){
            // Essais/Qualifs => 5 min = 300s, Course => 5s
            lapTime += 300.0;
            ecrire_debut();
            f1->cars[idxCar].status= CAR_PIT;
            ecrire_fin();
        }

        ecrire_debut();
        double rem = f1->cars[idxCar].remaining;
        if(rem <= 0.1){
            ecrire_fin();
            _exit(0);
        }
        if(lapTime > rem) lapTime = rem;
        rem -= lapTime;
        f1->cars[idxCar].remaining = rem;

        if(lapTime < f1->cars[idxCar].bestLap){
            f1->cars[idxCar].bestLap = lapTime;
        }
        if(s1 < f1->cars[idxCar].bestS1) f1->cars[idxCar].bestS1 = s1;
        if(s2 < f1->cars[idxCar].bestS2) f1->cars[idxCar].bestS2 = s2;
        if(s3 < f1->cars[idxCar].bestS3) f1->cars[idxCar].bestS3 = s3;

        if(rem <= 0.1){
            f1->cars[idxCar].status=CAR_OUT;
            f1->carsFinished++;
            ecrire_fin();
            _exit(0);
        } else {
            f1->cars[idxCar].status= CAR_RUNNING;
        }
        ecrire_fin();

        usleep(500000); // 0.5s
    }
}
