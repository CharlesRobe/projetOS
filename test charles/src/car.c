#include "car.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Process enfant : simulation d’une voiture
void run_car_process(int carIndex)
{
    srand(time(NULL) ^ (getpid()<<16));

    while(1) {
        // Petite probabilité d’abandon
        if ((rand() % 100) < 3) {
            handle_abandon();
            _exit(0);
        }

        // Simule un tour
        double s1 = generate_sector_time(1);
        double s2 = generate_sector_time(2);
        double s3 = generate_sector_time(3);
        double lapTime = s1 + s2 + s3;

        // Affiche en direct
        printf("[CarIndex=%d, PID=%d] Lap=%.3f (S1=%.2f,S2=%.2f,S3=%.2f)\n",
               carIndex, getpid(), lapTime, s1, s2, s3);

        // Pit Stop ?
        if ((rand() % 100) < 5) {
            handle_pit_stop();
        }

        // Pause
        usleep(500000); // 0.5 s
    }

    _exit(0);
}

// Génère le temps d’un secteur (25..45s)
double generate_sector_time(int sector)
{
    double base = 25 + (rand() % 21);
    double frac = (rand() % 1000) / 1000.0;
    // On peut ajuster selon le secteur
    return base + frac + 0.1 * sector;
}

void handle_pit_stop()
{
    printf("[Car PID=%d] PIT STOP!\n", getpid());
    // Simulation d’un arrêt = 2s
    sleep(2);
}

void handle_abandon()
{
    printf("[Car PID=%d] ABANDON!\n", getpid());
}
