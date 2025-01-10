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

#define NUM_CARS 20

typedef struct {
    int carNumber;
    float bestTimeSec1;
    float bestTimeSec2;
    float bestTimeSec3;
    float bestTimeTot;
} CarResult;

float GenRanNum(int seedIncrementer, int secMin, int secMax) {
	float randomTime = ((rand() % (((secMax - secMin)*1000) + 1) ) + (secMin*1000)) / 1000.0; //Donner un temps compris entre 25 et 45 secondes pour un secteur.
	return randomTime;
}

void simulateCar(int carNumber, int seed, int durationMinutes, int pipeFd) {
    float bestTimeSec1 = INFINITY;
    float bestTimeSec2 = INFINITY;
    float bestTimeSec3 = INFINITY;
    float bestTimeTot = INFINITY;

    float sessionDuration = 0;
    float tempsSec1, tempsSec2, tempsSec3, tempsTotal;

    while (sessionDuration < durationMinutes * 60) {
        tempsSec1 = GenRanNum(seed++, 25, 45);
        tempsSec2 = GenRanNum(seed++, 25, 45);
        tempsSec3 = GenRanNum(seed++, 25, 45);
        tempsTotal = tempsSec1 + tempsSec2 + tempsSec3;
        sessionDuration += tempsTotal;

        if (tempsSec1 < bestTimeSec1) bestTimeSec1 = tempsSec1;
        if (tempsSec2 < bestTimeSec2) bestTimeSec2 = tempsSec2;
        if (tempsSec3 < bestTimeSec3) bestTimeSec3 = tempsSec3;
        if (tempsTotal < bestTimeTot) bestTimeTot = tempsTotal;
    }

    CarResult result = {carNumber, bestTimeSec1, bestTimeSec2, bestTimeSec3, bestTimeTot};
    write(pipeFd, &result, sizeof(CarResult));
    close(pipeFd);
    exit(0);
}

int compareResults(const void *a, const void *b) {
    CarResult *carA = (CarResult *)a;
    CarResult *carB = (CarResult *)b;
    if (carA->bestTimeTot < carB->bestTimeTot) return -1;
    if (carA->bestTimeTot > carB->bestTimeTot) return 1;
    return 0;
}

void runQualificationSession(const char *sessionName, int durationMinutes, int numCars, int *carNumbers, CarResult *results) {
    printf("Starting %s\n", sessionName);
    int pipes[NUM_CARS][2];
    pid_t pids[NUM_CARS];

    for (int i = 0; i < numCars; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        pids[i] = fork();
        if (pids[i] == 0) {
            close(pipes[i][0]);
            srand(getpid());
            simulateCar(carNumbers[i], rand(), durationMinutes, pipes[i][1]);
        } else if (pids[i] > 0) {
            close(pipes[i][1]);
        } else {
            perror("erreur du fork");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < numCars; i++) {
        waitpid(pids[i], NULL, 0);
        read(pipes[i][0], &results[i], sizeof(CarResult));
        close(pipes[i][0]);
    }

    qsort(results, numCars, sizeof(CarResult), compareResults);
    printf("Results for %s:\n", sessionName);
    for (int i = 0; i < numCars; i++) {
        printf("Car %d: %.3f\n", results[i].carNumber, results[i].bestTimeTot);
    }
    printf("End of %s\n", sessionName);
}


int main() {
    int carNumbers1[NUM_CARS] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    int carNumbers2[NUM_CARS - 5];
    int carNumbers3[NUM_CARS - 10];

    CarResult results1[NUM_CARS];
    CarResult results2[NUM_CARS - 5];
    CarResult results3[NUM_CARS - 10];

    runQualificationSession("Q1", 18, NUM_CARS, carNumbers1, results1);

    for (int i = 0; i < NUM_CARS - 5; i++) {
        carNumbers2[i] = results1[i].carNumber;
    }
    runQualificationSession("Q2", 15, NUM_CARS - 5, carNumbers2, results2);

    for (int i = 0; i < NUM_CARS - 10; i++) {
        carNumbers3[i] = results2[i].carNumber;
    }
    runQualificationSession("Q3", 12, NUM_CARS - 10, carNumbers3, results3);

    printf("Final Grid:\n");
    for (int i = 0; i < NUM_CARS - 10; i++) {
        printf("P%d: Car %d\n", i + 1, results3[i].carNumber);
    }
    for (int i = 0; i < 5; i++) {
        printf("P%d: Car %d\n", NUM_CARS - 10 + i + 1, results2[NUM_CARS - 10 + i].carNumber);
    }
    for (int i = 0; i < 5; i++) {
        printf("P%d: Car %d\n", NUM_CARS - 5 + i + 1, results1[NUM_CARS - 5 + i].carNumber);
    }

    return 0;
}
