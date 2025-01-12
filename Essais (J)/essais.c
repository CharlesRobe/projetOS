#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define NUM_CARS 20

typedef struct {
    int car_number;        // Numéro de la voiture.
    float best_time_sec1;   // Meilleur temps pour le secteur 1.
    float best_time_sec2;   // Meilleur temps pour le secteur 2.
    float best_time_sec3;   // Meilleur temps pour le secteur 3.
    float best_time_total;    // Meilleur temps total de la voiture.
} CarResult;

// Génère un nombre aléatoire représentant un temps compris entre num_min et num_max.
float genRanNum(int seed_incrementer, int num_min, int num_max) {
    srand(time(NULL) + seed_incrementer);
    float rand_num = ((rand() % (((num_max - num_min)*1000) + 1) ) + (num_min*1000)) / 1000.0;
    return rand_num;
}

// Simule la performance d'une voiture pendant la durée donnée.
void simulateCar(int car_number, int seed_incrementer, int duration_minutes, int pipe_fd) {
    float best_time_sec1 = INFINITY; // Meilleur temps secteur 1 initialisé à l'infini.
    float best_time_sec2 = INFINITY; // Meilleur temps secteur 2 initialisé à l'infini.
    float best_time_sec3 = INFINITY; // Meilleur temps secteur 3 initialisé à l'infini.
    float best_time_total = INFINITY;  // Meilleur temps total initialisé à l'infini.

    float session_duration = 0; // Durée de la session écoulée.
    float time_sec1, time_sec2, time_sec3, time_total;

    // Boucle pour simuler les tours tant que la session n'est pas terminée.
    while (session_duration < duration_minutes * 60) {
        time_sec1 = genRanNum(seed_incrementer++, 25, 45); // Temps aléatoire pour le secteur 1.
        usleep(10000);
        time_sec2 = genRanNum(seed_incrementer++, 25, 45); // Temps aléatoire pour le secteur 2.
        usleep(10000);
        time_sec3 = genRanNum(seed_incrementer++, 25, 45); // Temps aléatoire pour le secteur 3.
        usleep(10000);
        time_total = time_sec1 + time_sec2 + time_sec3; // Temps total pour ce tour.
        session_duration += time_total; // Ajout du temps total à la durée de la session.

        // Mise à jour des meilleurs temps si nécessaires.
        if (time_sec1 < best_time_sec1) best_time_sec1 = time_sec1;
        if (time_sec2 < best_time_sec2) best_time_sec2 = time_sec2;
        if (time_sec3 < best_time_sec3) best_time_sec3 = time_sec3;
        if (time_total < best_time_total) best_time_total = time_total;
    }

    // Stocke les résultats dans une structure et les envoie via le pipe.
    CarResult result = {car_number, best_time_sec1, best_time_sec2, best_time_sec3, best_time_total};
    write(pipe_fd, &result, sizeof(CarResult)); // Écriture des résultats dans le pipe.
    close(pipe_fd); // Fermeture du pipe.
    exit(0); // Fin du processus enfant.
}

// Fonction de comparaison pour trier les résultats selon le meilleur temps total.
int compareResults(const void *a, const void *b) {
    CarResult *carA = (CarResult *)a;
    CarResult *carB = (CarResult *)b;
    if (carA->best_time_total < carB->best_time_total) return -1;
    if (carA->best_time_total > carB->best_time_total) return 1;
    return 0;
}

// Lance une session d'essais.
void runTrialsSession(const char *session_name, int duration_minutes, int amount_cars, int *car_numbers, CarResult *results) {
    printf("Starting %s\n", session_name); // Début de la session.
    int pipes[NUM_CARS][2]; // Tableau de pipes pour la communication interprocessus.
    pid_t pids[NUM_CARS];   // Tableau pour stocker les PID des processus enfants.

    // Création des processus pour chaque voiture.
    for (int i = 0; i < amount_cars; i++) {
        if (pipe(pipes[i]) == -1) { // Création du pipe pour chaque voiture.
            perror("pipe"); // Gestion d'erreur en cas d'échec.
            exit(EXIT_FAILURE);
        }
        pids[i] = fork(); // Création d'un processus enfant.
        if (pids[i] == 0) { // Code exécuté par le processus enfant.
            close(pipes[i][0]); // Fermeture du côté lecture du pipe.
            srand(getpid()); // Initialisation de la graine aléatoire avec le PID.
            simulateCar(car_numbers[i], rand(), duration_minutes, pipes[i][1]); // Simulation de la voiture.
        } else if (pids[i] > 0) { // Code exécuté par le processus parent.
            close(pipes[i][1]); // Fermeture du côté écriture du pipe.
        } else { // Gestion d'erreur en cas d'échec de `fork`.
            perror("erreur du fork");
            exit(EXIT_FAILURE);
        }
    }

    // Lecture des résultats depuis les pipes après la fin des processus enfants.
    for (int i = 0; i < amount_cars; i++) {
        waitpid(pids[i], NULL, 0); // Attente de la fin du processus enfant.
        read(pipes[i][0], &results[i], sizeof(CarResult)); // Lecture des résultats.
        close(pipes[i][0]); // Fermeture du côté lecture du pipe.
    }

    // Tri des résultats par ordre croissant de temps total.
    qsort(results, amount_cars, sizeof(CarResult), compareResults);

    // Affichage des résultats de la session.
    printf("Results for %s:\n", session_name);
    for (int i = 0; i < amount_cars; i++) {
        printf("Car %d: %.3f\n", results[i].car_number, results[i].best_time_total);
    }
    printf("End of %s\n", session_name); // Fin de la session.


    printf("    n°    | sector 1 | sector 2 | sector 3 |  total   \n", session_name);
    printf("----------+----------+----------+----------+----------\n");
    for (int i = 0; i < amount_cars; i++) {
        // printf("Car %d: %.3f\n", results[i].car_number, results[i].best_time_total);
        printf("    %d    |  %.3f  |  %.3f  |  %.3f  |  %.3f  \n",
            results[i].car_number, results[i].best_time_sec1, results[i].best_time_sec2,
            results[i].best_time_sec3, results[i].best_time_total);
    }
}

int main() {
    int car_numbers[NUM_CARS] = {1, 11, 44, 63, 16, 55, 4, 81, 14, 18, 10, 31, 23,
        2, 22, 3, 77, 24, 20, 27};

    CarResult results_P1[NUM_CARS]; // Résultats de P1.
    CarResult results_P2[NUM_CARS]; // Résultats de P2.
    CarResult results_P3[NUM_CARS]; // Résultats de P3.

    // Lancement des trois sessions d'essais.
    runTrialsSession("P1", 60, NUM_CARS, car_numbers, results_P1);
    runTrialsSession("Q2", 60, NUM_CARS, car_numbers, results_P2);
    runTrialsSession("Q3", 60, NUM_CARS, car_numbers, results_P3);

    // Pilote fictif sur tableau de scores pour meilleurs résultats.
    printf("----------+----------+----------+----------+----------\n");
    printf("   BEST   |          |          |          |          \n");

    return 0; // Fin du programme.
}