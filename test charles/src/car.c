#include "car.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Fonction exécutée par chaque processus "voiture"
void run_car_process(int carIndex)
{
    srand(time(NULL) ^ (getpid() << 16));

    // Exemples de boucle "course" simplifiée
    // Idéalement, la synchronisation (via signaux, mémoire partagée, etc.)
    // se fera ici pour que le parent et l'enfant communiquent.
    int lap = 0;
    while (1) {
        // Simulation d'un abandon en cours de route, par ex. 5% de chance
        if ((rand() % 100) < 5) {
            handle_abandon();
            _exit(0);
        }

        // Sinon, on génère un temps de tour
        double s1 = generate_sector_time(1);
        double s2 = generate_sector_time(2);
        double s3 = generate_sector_time(3);
        double lapTime = s1 + s2 + s3;

        // Ici, on ferait un envoi d'informations au parent
        // (via pipe, mémoire partagée, ou autre).

        // On fait "dormir" le processus pour simuler la durée du tour
        // ou la durée nécessaire à générer un nouveau passage
        usleep(500000);  // 0.5 s (valeur arbitraire pour la démo)

        lap++;
        // On peut décider d'une condition de fin, par ex. un signal du parent
        // ou un nombre de tours maxi. On sort alors de la boucle.
    }

    // Le process enfant se termine
    _exit(0);
}

double generate_sector_time(int sector)
{
    // Exemple : entre 25 et 45 secondes
    double timeSec = 25 + rand() % 21 + (rand() % 1000) / 1000.0;
    return timeSec;
}

void handle_pit_stop()
{
    // Gérer le temps dans les stands, ~25s
    sleep(1); // Simulation - adapter à vos besoins
}

void handle_abandon()
{
    // Gérer l'abandon
    // Dans un vrai code, on enverrait un message au parent pour indiquer l'OUT
    fprintf(stderr, "Voiture %d : abandon...\n", getpid());
}
