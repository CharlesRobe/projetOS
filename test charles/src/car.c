#include "car.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void run_car_process(int carIndex)
{
    // "carIndex" peut servir d’index pour la mémoire partagée ou un pipe,
    // ici on se limite à un squelette.
    srand(time(NULL) ^ (getpid() << 16));

    while (1) {
        // Probabilité d'abandon : ~3%
        if ((rand() % 100) < 3) {
            handle_abandon();
            _exit(0);
        }

        // On simule un tour
        double s1 = generate_sector_time(1);
        double s2 = generate_sector_time(2);
        double s3 = generate_sector_time(3);
        double lapTime = s1 + s2 + s3;

        // Petit affichage “debug”
        printf("[Car index=%d, PID=%d] LapTime = %.3f (S1=%.3f, S2=%.3f, S3=%.3f)\n",
               carIndex, getpid(), lapTime, s1, s2, s3);

        // 5% de chance d’aller au stand
        if ((rand() % 100) < 5) {
            handle_pit_stop();
        }

        // Pause avant le prochain tour
        usleep(500000); // 0.5 s
    }
}

double generate_sector_time(int sector)
{
    // Entre 25 et 45s, on ajoute un petit offset selon le secteur
    double base = 25.0 + (rand() % 21);
    double frac = (rand() % 1000) / 1000.0;
    double adjustment = 0.1 * sector; 
    return base + frac + adjustment;
}

void handle_pit_stop()
{
    printf("[Car PID=%d] PIT STOP!\n", getpid());
    // 2s de stand, simplifié
    sleep(2);
}

void handle_abandon()
{
    printf("[Car PID=%d] ABANDON!\n", getpid());
}
