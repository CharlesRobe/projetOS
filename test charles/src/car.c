#include "car.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void run_car_process(int carIndex)
{
    // Dans un vrai projet, "carIndex" servira à indexer la mémoire partagée
    // ou le pipe vers le parent. Ici, on se contente d'un squelette.

    srand(time(NULL) ^ (getpid() << 16));

    // Exemple d’une boucle "infinie", en attendant un signal du parent
    // pour arrêter la session. On simule un passage en piste toutes les X ms.
    while (1) {
        // Probabilité d'abandon : 3 %
        if ((rand() % 100) < 3) {
            handle_abandon();
            _exit(0);
        }

        // On "roule" un tour (en réalité, un simple usleep + random)
        double s1 = generate_sector_time(1);
        double s2 = generate_sector_time(2);
        double s3 = generate_sector_time(3);
        double lapTime = s1 + s2 + s3;

        // Dans un code complet :
        //   - On mettrait lapTime dans un segment partagé
        //   - On signalerait au parent "voiture X vient de finir un tour"

        // Petit affichage pour démo
        printf("[Car #%d, PID=%d] LapTime = %.3f (S1=%.3f, S2=%.3f, S3=%.3f)\n",
               carIndex, getpid(), lapTime, s1, s2, s3);

        // On simule un éventuel pit stop (5% de chance)
        if ((rand() % 100) < 5) {
            handle_pit_stop();
        }

        // On fait une pause avant le prochain tour
        usleep(500000); // 0.5 s
    }

    // Process terminé
    _exit(0);
}

double generate_sector_time(int sector)
{
    // Ex : temps aléatoire entre 25s et 45s
    double base = 25.0 + (rand() % 21);
    double frac = (rand() % 1000) / 1000.0;
    // On peut moduler selon le secteur
    double adjustment = 0.1 * sector; 
    return base + frac + adjustment;
}

void handle_pit_stop()
{
    // Un pit stop ~ 25s, on simplifie à 1 ou 2s
    printf("[Car PID=%d] PIT STOP\n", getpid());
    sleep(2);
}

void handle_abandon()
{
    printf("[Car PID=%d] ABANDON !\n", getpid());
}
