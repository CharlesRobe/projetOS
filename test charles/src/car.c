#include "car.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// Process enfant : envoie régulièrement ses temps (lapTime) via pipe
void run_car_process(int indexCar, int writeFd)
{
    srand(time(NULL) ^ (getpid()<<16));

    while(1) {
        // ~3% d'abandon
        if((rand()%100)<3) {
            handle_abandon();
            _exit(0);
        }
        // Un tour
        double s1 = generate_sector_time(1);
        double s2 = generate_sector_time(2);
        double s3 = generate_sector_time(3);
        double lapTime = s1 + s2 + s3;

        // Affichage "enfant"
        printf("[Child-CarIndex=%d, PID=%d] Lap=%.3f (S1=%.2f,S2=%.2f,S3=%.2f)\n",
               indexCar, getpid(), lapTime, s1, s2, s3);

        // On envoie le lapTime au parent via le pipe
        if(write(writeFd, &lapTime, sizeof(double))<0) {
            perror("write pipe");
        }

        // ~5% PIT
        if((rand()%100)<5) {
            handle_pit_stop();
        }
        usleep(500000); // 0.5s
    }
    _exit(0);
}

double generate_sector_time(int sector)
{
    double base = 25.0 + (rand()%21); // 25..45
    double frac = (rand()%1000)/1000.0;
    return base+frac+0.1*sector;
}

void handle_pit_stop()
{
    printf("[Child PID=%d] PIT STOP\n", getpid());
    sleep(2);
}

void handle_abandon()
{
    printf("[Child PID=%d] ABANDON\n", getpid());
}
