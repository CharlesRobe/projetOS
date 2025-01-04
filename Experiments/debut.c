#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

#define NUM_CARS 5 // Nombre total de voitures simulées
#define NUM_SECTORS 3 // Nombre de secteurs dans un tour
#define MIN_SECTOR_TIME 25 // Temps minimal pour un secteur (en secondes)
#define MAX_SECTOR_TIME 45 // Temps maximal pour un secteur (en secondes)

int display_flag = 0; // Indicateur pour déclencher l'affichage périodique des résultats

// Génère un temps aléatoire entre MIN_SECTOR_TIME et MAX_SECTOR_TIME
float generate_random_time(int car_id, int sector, int iteration) {
    // Obtenir l'heure actuelle en nanosecondes pour une seed unique
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned int seed = ts.tv_nsec ^ (car_id * 100 + sector * 10 + iteration); // Combiner l'identifiant, le secteur et l'itération
    srand(seed); // Initialiser la seed du générateur de nombres aléatoires

    // Générer un temps aléatoire dans l'intervalle défini
    return MIN_SECTOR_TIME + (rand() % (MAX_SECTOR_TIME - MIN_SECTOR_TIME + 1)) + ((float)rand() / RAND_MAX);
}

// Affiche un tableau des performances actuelles des voitures
void display_table(int car_ids[], float sector_times[][NUM_SECTORS], float best_lap_times[], int is_pitting[], int is_out[]) {
    printf("\n\nTableau des temps actuels:\n");
    printf("+------------+-------------+-------------+-------------+---------------+-------+\n");
    printf("| Voiture ID | Secteur 1   | Secteur 2   | Secteur 3   | Meilleur tour | P/O   |\n");
    printf("+------------+-------------+-------------+-------------+---------------+-------+\n");

    // Parcourir toutes les voitures pour afficher leurs données
    for (int i = 0; i < NUM_CARS; i++) {
        printf("| %10d | %11.3f | %11.3f | %11.3f | %13.3f | %s/%s |\n",
               car_ids[i],
               sector_times[i][0],
               sector_times[i][1],
               sector_times[i][2],
               best_lap_times[i],
               is_pitting[i] ? "P" : "-", // Indicateur pour les stands
               is_out[i] ? "O" : "-"); // Indicateur pour l'abandon
    }
    printf("+------------+-------------+-------------+-------------+---------------+-------+\n");
}

// Fonction exécutée par chaque voiture (processus enfant)
void simulate_car(int car_id, int pipe_fd[2]) {
    close(pipe_fd[0]); // Fermer la lecture du pipe (utilisée uniquement par le processus parent)

    float sector_times[NUM_SECTORS]; // Temps pour chaque secteur
    float best_lap_time = 0; // Meilleur temps pour un tour complet
    int is_pitting = 0; // Indique si la voiture est dans les stands
    int is_out = 0; // Indique si la voiture a abandonné

    for (int lap = 0; lap < 5; lap++) { // Simuler 5 tours pour chaque voiture
        float lap_time = 0; // Temps total pour le tour courant

        // Générer les temps pour les trois secteurs
        for (int sector = 0; sector < NUM_SECTORS; sector++) {
            sector_times[sector] = generate_random_time(car_id, sector, lap);
            lap_time += sector_times[sector]; // Ajouter le temps du secteur au total
        }

        // Mettre à jour le meilleur temps si le tour actuel est plus rapide
        if (best_lap_time == 0 || lap_time < best_lap_time) {
            best_lap_time = lap_time;
        }

        // Simuler un arrêt aux stands ou un abandon aléatoire
        is_pitting = rand() % 10 < 1; // 10% de chance d'aller aux stands
        if (rand() % 20 < 1) { // 5% de chance d'abandonner
            is_out = 1;
        }

        // Envoyer les données au processus parent
        write(pipe_fd[1], &sector_times, sizeof(sector_times));
        write(pipe_fd[1], &best_lap_time, sizeof(float));
        write(pipe_fd[1], &is_pitting, sizeof(int));
        write(pipe_fd[1], &is_out, sizeof(int));

        if (is_out) break; // Arrêter la simulation si la voiture abandonne
    }

    close(pipe_fd[1]); // Fermer l'écriture du pipe
    exit(0); // Terminer le processus enfant
}

// Gérer le signal pour déclencher l'affichage
void handle_display_signal(int sig) {
    display_flag = 1; // Activer le drapeau pour demander l'affichage
}

int main() {
    int pipe_fd[NUM_CARS][2]; // Tableau de pipes pour la communication entre le parent et les enfants
    pid_t pids[NUM_CARS]; // Identifiants des processus enfants

    int car_ids[NUM_CARS]; // Identifiants des voitures
    float sector_times[NUM_CARS][NUM_SECTORS]; // Temps pour chaque secteur par voiture
    float best_lap_times[NUM_CARS]; // Meilleurs temps pour chaque voiture
    int is_pitting[NUM_CARS]; // État de "stands" pour chaque voiture
    int is_out[NUM_CARS]; // État d'abandon pour chaque voiture

    // Configurer le signal pour afficher les données périodiquement
    signal(SIGALRM, handle_display_signal); // Associer le gestionnaire de signal
    alarm(10); // Déclencher un signal toutes les 10 secondes

    // Initialiser les pipes et créer les processus enfants
    for (int i = 0; i < NUM_CARS; i++) {
        pipe(pipe_fd[i]); // Créer un pipe pour chaque voiture
        car_ids[i] = i + 1; // Assigner un ID unique à chaque voiture
        best_lap_times[i] = 0; // Initialiser le meilleur temps à 0
        is_pitting[i] = 0; // Initialiser l'état "stands" à faux
        is_out[i] = 0; // Initialiser l'état "abandon" à faux

        if ((pids[i] = fork()) == 0) { // Créer un processus enfant
            simulate_car(car_ids[i], pipe_fd[i]); // Appeler la fonction de simulation
        }
    }

    // Lecture des données des processus enfants
    while (1) {
        for (int i = 0; i < NUM_CARS; i++) {
            close(pipe_fd[i][1]); // Fermer l'écriture du pipe dans le parent

            // Lire les données envoyées par chaque voiture
            read(pipe_fd[i][0], &sector_times[i], sizeof(sector_times[i]));
            read(pipe_fd[i][0], &best_lap_times[i], sizeof(float));
            read(pipe_fd[i][0], &is_pitting[i], sizeof(int));
            read(pipe_fd[i][0], &is_out[i], sizeof(int));
        }

        // Afficher les données si le signal a été reçu
        if (display_flag) {
            display_table(car_ids, sector_times, best_lap_times, is_pitting, is_out);
            display_flag = 0; // Réinitialiser le drapeau
            alarm(10); // Réarmer le signal
        }
    }

    return 0; // Fin du programme
}

