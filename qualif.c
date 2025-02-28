#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <math.h>

#define NUM_CARS 20 // Nombre total de voitures participant à la session.

typedef struct {
    int carNumber;        // Numéro de la voiture.
    float bestTimeSec1;   // Meilleur temps pour le secteur 1.
    float bestTimeSec2;   // Meilleur temps pour le secteur 2.
    float bestTimeSec3;   // Meilleur temps pour le secteur 3.
    float bestTimeTot;    // Meilleur temps total de la voiture.
} CarResult;

int carNumbers1[NUM_CARS] = {1, 11, 44, 63, 16, 55, 4, 81, 14, 18, 10, 31, 23, 2, 22, 3, 77, 24, 20, 27};
int carNumbers2[NUM_CARS - 5];    // Tableau pour Q2.
int carNumbers3[NUM_CARS - 10];   // Tableau pour Q3.

CarResult results1[NUM_CARS];       // Résultats de Q1.
CarResult results2[NUM_CARS - 5];   // Résultats de Q2.
CarResult results3[NUM_CARS - 10];  // Résultats de Q3;

/**
 * Génère un nombre aléatoire représentant un temps compris entre secMin et secMax secondes.
 */
float GenRanNum(int seedIncrementer, int secMin, int secMax) {
    float randomTime = ((rand() % (((secMax - secMin) * 1000) + 1)) + (secMin * 1000)) / 1000.0;
    return randomTime;
}

/**
 * Simule la performance d'une voiture pendant la durée donnée.
 */
void simulateCar(int carNumber, int seed, int durationMinutes, int pipeFd) {
    float bestTimeSec1 = INFINITY; // Meilleur temps secteur 1 initialisé à l'infini.
    float bestTimeSec2 = INFINITY; // Meilleur temps secteur 2 initialisé à l'infini.
    float bestTimeSec3 = INFINITY; // Meilleur temps secteur 3 initialisé à l'infini.
    float bestTimeTot = INFINITY;  // Meilleur temps total initialisé à l'infini.

    float sessionDuration = 0; // Durée de la session écoulée.
    float tempsSec1, tempsSec2, tempsSec3, tempsTotal;

    // Boucle pour simuler les tours tant que la session n'est pas terminée.
    while (sessionDuration < durationMinutes * 60) {
        tempsSec1 = GenRanNum(seed++, 25, 45); // Temps aléatoire pour le secteur 1.
        tempsSec2 = GenRanNum(seed++, 25, 45); // Temps aléatoire pour le secteur 2.
        tempsSec3 = GenRanNum(seed++, 25, 45); // Temps aléatoire pour le secteur 3.
        tempsTotal = tempsSec1 + tempsSec2 + tempsSec3; // Temps total pour ce tour.
        sessionDuration += tempsTotal; // Ajout du temps total à la durée de la session.

        // Mise à jour des meilleurs temps si nécessaires.
        if (tempsSec1 < bestTimeSec1) bestTimeSec1 = tempsSec1;
        if (tempsSec2 < bestTimeSec2) bestTimeSec2 = tempsSec2;
        if (tempsSec3 < bestTimeSec3) bestTimeSec3 = tempsSec3;
        if (tempsTotal < bestTimeTot) bestTimeTot = tempsTotal;
    }

    // Stocke les résultats dans une structure et les envoie via le pipe.
    CarResult result = {carNumber, bestTimeSec1, bestTimeSec2, bestTimeSec3, bestTimeTot};
    write(pipeFd, &result, sizeof(CarResult)); // Écriture des résultats dans le pipe.
    close(pipeFd); // Fermeture du pipe.
    exit(0); // Fin du processus enfant.
}

/**
 * Fonction de comparaison pour trier les résultats selon le meilleur temps total.
 */
int compareResults(const void *a, const void *b) {
    CarResult *carA = (CarResult *)a;
    CarResult *carB = (CarResult *)b;
    if (carA->bestTimeTot < carB->bestTimeTot) return -1;
    if (carA->bestTimeTot > carB->bestTimeTot) return 1;
    return 0;
}

/**
 * Lance une session de qualification.
 */
void runQualificationSession(int numSession, int durationMinutes, int numCars, int *carNumbers, CarResult *results) {
    printf("Starting Q%d\n", numSession); // Début de la session.
    int pipes[NUM_CARS][2]; // Tableau de pipes pour la communication interprocessus.
    pid_t pids[NUM_CARS];   // Tableau pour stocker les PID des processus enfants.

    // Création des processus pour chaque voiture.
    for (int i = 0; i < numCars; i++) {
        if (pipe(pipes[i]) == -1) { // Création du pipe pour chaque voiture.
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        pids[i] = fork(); // Création d'un processus enfant.
        if (pids[i] == 0) { // Code exécuté par le processus enfant.
            close(pipes[i][0]); // Fermeture du côté lecture du pipe.
            srand(getpid()); // Initialisation de la graine aléatoire avec le PID.
            simulateCar(carNumbers[i], rand(), durationMinutes, pipes[i][1]); // Simulation de la voiture.
        } else if (pids[i] > 0) { // Code exécuté par le processus parent.
            close(pipes[i][1]); // Fermeture du côté écriture du pipe.
        } else {
            perror("erreur du fork");
            exit(EXIT_FAILURE);
        }
    }

    // Lecture des résultats depuis les pipes après la fin des processus enfants.
    for (int i = 0; i < numCars; i++) {
        waitpid(pids[i], NULL, 0); // Attente de la fin du processus enfant.
        read(pipes[i][0], &results[i], sizeof(CarResult)); // Lecture des résultats.
        close(pipes[i][0]); // Fermeture du côté lecture du pipe.
    }

    // Tri des résultats par ordre croissant de temps total.
    qsort(results, numCars, sizeof(CarResult), compareResults);

    // Affichage des résultats de la session.
    printf("Results for Q%d:\n", numSession);
    for (int i = 0; i < numCars; i++) {
        printf("Car %d: %.3f\n", results[i].carNumber, results[i].bestTimeTot);
    }
    printf("End of Q%d\n", numSession);
}

/**
 * Fonction principale de qualification.
 */
int qualif(const char* nomCircuit, int numSession, int weType) {
    
    int durationSession = 0;

    // Définition de la durée de session selon le type de week-end et le numéro de session
    if (weType == 0){
        switch (numSession) {
            case 1:
                durationSession = 18;
                break;
            case 2:
                durationSession = 15;
                break;
            case 3:
                durationSession = 12;
                break;
        }
    }
    else if (weType == 1){
        switch (numSession) {
            case 1:
                durationSession = 12;
                break;
            case 2:
                durationSession = 10;
                break;
            case 3:
                durationSession = 8;
                break;
        }
    }

    // Lancement des sessions et récupération des résultats
    if (numSession == 1) {
        // Q1
        runQualificationSession(1, durationSession, NUM_CARS, carNumbers1, results1);

        // Prépare les 15 meilleures voitures pour Q2
        for (int i = 0; i < NUM_CARS - 5; i++) {
            carNumbers2[i] = results1[i].carNumber;
        }
    }
    else if (numSession == 2) {
        // Q2
        runQualificationSession(2, durationSession, NUM_CARS - 5, carNumbers2, results2);

        // Prépare les 10 meilleures voitures pour Q3
        for (int i = 0; i < NUM_CARS - 10; i++) {
            carNumbers3[i] = results2[i].carNumber;
        }
    }
    else if (numSession == 3) {
        // Q3
        runQualificationSession(3, durationSession, NUM_CARS - 10, carNumbers3, results3);

        // Construction d'un tableau final pour l'affichage 
        CarResult finalGrid[NUM_CARS];
        int index = 0;

        // Les 10 premiers (issus de Q3)
        for (int i = 0; i < NUM_CARS - 10; i++) {
            finalGrid[index++] = results3[i];
        }

        // Les 5 suivants (issus de Q2)
        for (int i = 0; i < 5; i++) {
            finalGrid[index++] = results2[NUM_CARS - 10 + i];
        }

        // Les 5 derniers (issus de Q1)
        for (int i = 0; i < 5; i++) {
            finalGrid[index++] = results1[NUM_CARS - 5 + i];
        }

        // *** Affichage en forme de tableau dans la console ***
        printf("\n");
        printf("*********************************************\n");
        printf("*             QUALIFICATIONS                *\n");
        printf("*********************************************\n");
        printf("| %-3s | %-5s | %-9s | %-9s | %-9s | %-9s |\n",
               "Pos", "Car#", "Sect1", "Sect2", "Sect3", "Total");
        printf("|-----|-------|-----------|-----------|-----------|-----------|\n");

        for (int i = 0; i < NUM_CARS; i++) {
            printf("| %-3d | %-5d | %-9.3f | %-9.3f | %-9.3f | %-9.3f |\n",
                   i + 1,
                   finalGrid[i].carNumber,
                   finalGrid[i].bestTimeSec1,
                   finalGrid[i].bestTimeSec2,
                   finalGrid[i].bestTimeSec3,
                   finalGrid[i].bestTimeTot);
        }
        printf("---------------------------------------------------------------\n\n");

        // Écriture du classement dans un fichier "ranking.txt" dans le dossier nomCircuit fourni en paramètre
        // On construit le chemin vers "ranking.txt"
        char rankingFilePath[1024];
        snprintf(rankingFilePath, sizeof(rankingFilePath), "%s/ranking.txt", nomCircuit);

        FILE *fpRanking = fopen(rankingFilePath, "w");
        if (!fpRanking) {
            perror("Erreur lors de la création de ranking.txt");
            exit(EXIT_FAILURE);
        }

        // On écrit le même tableau que celui affiché en console
        fprintf(fpRanking, "*********************************************\n");
        fprintf(fpRanking, "*             QUALIFICATIONS                *\n");
        fprintf(fpRanking, "*********************************************\n");
        fprintf(fpRanking, "| %-3s | %-5s | %-9s | %-9s | %-9s | %-9s |\n",
                "Pos", "Car#", "Sect1", "Sect2", "Sect3", "Total");
        fprintf(fpRanking, "|-----|-------|-----------|-----------|-----------|-----------|\n");

        for (int i = 0; i < NUM_CARS; i++) {
            fprintf(fpRanking, "| %-3d | %-5d | %-9.3f | %-9.3f | %-9.3f | %-9.3f |\n",
                    i + 1,
                    finalGrid[i].carNumber,
                    finalGrid[i].bestTimeSec1,
                    finalGrid[i].bestTimeSec2,
                    finalGrid[i].bestTimeSec3,
                    finalGrid[i].bestTimeTot);
        }
        fprintf(fpRanking, "---------------------------------------------------------------\n\n");

        fclose(fpRanking);

        // Écriture dans le fichier qualifies.txt
        FILE *fp = fopen("qualifies.txt", "w");
        if (!fp) {
            perror("Erreur lors de l'ouverture du fichier qualifies.txt");
            exit(EXIT_FAILURE);
        }

        // Les 10 premiers (issus de Q3)
        for (int i = 0; i < NUM_CARS - 10; i++) {
            fprintf(fp, "%d\n", results3[i].carNumber);
        }

        // Les 5 suivants (issus de Q2)
        for (int i = 0; i < 5; i++) {
            fprintf(fp, "%d\n", results2[NUM_CARS - 10 + i].carNumber);
        }

        // Les 5 derniers (issus de Q1)
        for (int i = 0; i < 5; i++) {
            fprintf(fp, "%d\n", results1[NUM_CARS - 5 + i].carNumber);
        }

        fclose(fp);
    }

    return 0;
}
