/* f1_essai.c - Programme principal pour la gestion des essais d'un week-end de F1 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#define N_VOITURES 20
#define DUREE_ESSAIS 60 // Simulation de 60 cycles pour représenter 1 heure fictive
#define ETAT_PIT_IN 1
#define ETAT_PIT_OUT 0

// Structure représentant une voiture de F1
typedef struct {
    int numero;
    char nom[20];
    float meilleur_temps;
    float secteurs[3];
    int etat; // ETAT_PIT_IN ou ETAT_PIT_OUT
} Voiture;

int numeros_voitures[N_VOITURES] = {1, 11, 44, 63, 16, 55, 4, 81, 14, 18, 10, 31, 23, 2, 22, 3, 77, 24, 20, 27};
Voiture voitures[N_VOITURES];

void initialiser_voitures() {
    for (int i = 0; i < N_VOITURES; i++) {
        snprintf(voitures[i].nom, sizeof(voitures[i].nom), "Voiture_%d", numeros_voitures[i]);
        voitures[i].numero = numeros_voitures[i];
        voitures[i].meilleur_temps = 0;
        voitures[i].etat = ETAT_PIT_OUT;
        for (int j = 0; j < 3; j++) {
            voitures[i].secteurs[j] = rand() % 21 + 25; // Temps entre 25 et 45 secondes par secteur
        }
    }
}

void simulation_essais() {
    pid_t pid;
    for (int i = 0; i < N_VOITURES; i++) {
        pid = fork();
        if (pid == 0) {
            // Processus enfant : simuler une voiture
            srand(time(NULL) ^ (getpid() << 16));
            for (int t = 0; t < DUREE_ESSAIS; t++) {
                int total_temps = 0;
                for (int j = 0; j < 3; j++) {
                    int secteur = rand() % 21 + 25;
                    voitures[i].secteurs[j] = secteur;
                    total_temps += secteur;
                    if (j == 2 && (rand() % 4 == 0)) { // Probabilité de rentrer au pit dans le 3ème secteur
                        voitures[i].etat = ETAT_PIT_IN;
                    } else {
                        voitures[i].etat = ETAT_PIT_OUT;
                    }
                }
                if (voitures[i].meilleur_temps == 0 || total_temps < voitures[i].meilleur_temps) {
                    voitures[i].meilleur_temps = total_temps;
                }
                sleep(1);
            }
            exit(0);
        } else if (pid < 0) {
            perror("Erreur fork");
            exit(1);
        }
    }
    // Attendre tous les enfants
    for (int i = 0; i < N_VOITURES; i++) {
        wait(NULL);
    }
}

void afficher_tableau() {
    printf("+---------+-----------+-----------+-----------+-----------+---------+\n");
    printf("| Numéro  | Temps     | Secteur1  | Secteur2  | Secteur3  | État    |\n");
    printf("+---------+-----------+-----------+-----------+-----------+---------+\n");
    for (int i = 0; i < N_VOITURES; i++) {
        printf("| %-7d | %-9.2f | %-9.2f | %-9.2f | %-9.2f | %-7s |\n",
               voitures[i].numero,
               (float)voitures[i].meilleur_temps,
               (float)voitures[i].secteurs[0],
               (float)voitures[i].secteurs[1],
               (float)voitures[i].secteurs[2],
               voitures[i].etat == ETAT_PIT_IN ? "PIT IN" : "PIT OUT");
    }
    printf("+---------+-----------+-----------+-----------+-----------+---------+\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <nom_de_la_course>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Début des essais pour la course: %s\n", argv[1]);
    srand(time(NULL));
    initialiser_voitures();
    simulation_essais();
    afficher_tableau();

    return EXIT_SUCCESS;
}
