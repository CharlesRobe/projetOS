#include "race.h"
#include "f1shared.h"
#include "rwcourtois.h"
#include "car.h"
#include "utils.h"
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Affichage scoreboard (lecteur)
static void scoreboard_live(F1Shared *f1, const char *title, int sortByLaps)
{
    lire_debut();
    Car localC[MAX_CARS];
    memcpy(localC, f1->cars, sizeof(localC));
    int fin= f1->carsFinished;
    double cLen= f1->circuitLength;
    lire_fin();
    if(sortByLaps==0){
        for(int i=0;i<MAX_CARS-1;i++){
            for(int j=i+1;j<MAX_CARS;j++){
                if(localC[j].bestLap < localC[i].bestLap){
                    Car tmp= localC[i];
                    localC[i]= localC[j];
                    localC[j]= tmp;
                }
            }
        }
    } else {
        for(int i=0;i<MAX_CARS-1;i++){
            for(int j=i+1;j<MAX_CARS;j++){
                if(localC[j].lapsDone > localC[i].lapsDone){
                    Car tmp= localC[i];
                    localC[i]= localC[j];
                    localC[j]= tmp;
                }
            }
        }
    }
    printf("\n=== [%s] (finished=%d) ===\n", title, fin);
    printf("| Voiture |   S1  |   S2  |   S3  | Meilleur Tour | Temps Total | Tour Actuel | Écart | Statut |\n");
    printf("|---------|-------|-------|-------|---------------|-------------|------------|-------|--------|\n");
    double leaderTime = -1.0;
    for(int i=0;i<MAX_CARS;i++){
        if(localC[i].status == CAR_OUT)
            continue;
        if(leaderTime < 0 || localC[i].bestLap < leaderTime){
            leaderTime = localC[i].bestLap;
        }
    }
    for(int i=0;i<MAX_CARS;i++){
        if(localC[i].status == CAR_OUT)
            continue;
        char st[8]="RUN";
        if(localC[i].status == CAR_PIT) strcpy(st,"PIT");
        else if(localC[i].status == CAR_OUT) strcpy(st,"OUT");
        double gap = 0.0;
        if(localC[i].bestLap > leaderTime){
            gap = localC[i].bestLap - leaderTime;
        }
        if(gap == 0.0){
            printf("| %7d | %5.1f | %5.1f | %5.1f | %13.3f | %-11s | %-10d | %-5s | %-6s |\n",
                   localC[i].carNumber,
                   localC[i].bestS1,
                   localC[i].bestS2,
                   localC[i].bestS3,
                   localC[i].bestLap,
                   "-", // Temps Total (non implémenté)
                   localC[i].lapsDone,
                   "-", // Écart (non implémenté)
                   st);
        } else {
            printf("| %7d | %5.1f | %5.1f | %5.1f | %13.3f | %-11s | %-10d | +%.0fs | %-6s |\n",
                   localC[i].carNumber,
                   localC[i].bestS1,
                   localC[i].bestS2,
                   localC[i].bestS3,
                   localC[i].bestLap,
                   "-", // Temps Total (non implémenté)
                   localC[i].lapsDone,
                   (int)gap,
                   st);
        }
    }
}

static void eliminate_5_worst(F1Shared *f1)
{
    ecrire_debut();
    Car copy[MAX_CARS];
    memcpy(copy, f1->cars, sizeof(copy));
    ecrire_fin();

    // Tri par bestLap asc
    for(int i=0;i<MAX_CARS-1;i++){
        for(int j=i+1;j<MAX_CARS;j++){
            if(copy[j].bestLap < copy[i].bestLap){
                Car tmp= copy[i];
                copy[i]= copy[j];
                copy[j]= tmp;
            }
        }
    }

    ecrire_debut();
    for(int i=MAX_CARS-5;i<MAX_CARS;i++){
        int cn= copy[i].carNumber;
        for(int k=0;k<MAX_CARS;k++){
            if(f1->cars[k].carNumber == cn && f1->cars[k].status != CAR_OUT){
                f1->cars[k].status = CAR_OUT;
                f1->carsFinished++;
            }
        }
    }
    ecrire_fin();
}

void load_circuit_length(const char *filename, int circuitNum, F1Shared *f1)
{
    FILE *fp= fopen(filename,"r");
    if(!fp) error_exit("fopen circuits.csv");
    char line[256];
    if(!fgets(line, sizeof(line), fp)){
        fclose(fp);
        error_exit("csv empty");
    }
    double length=5.412; // Default
    while(fgets(line, sizeof(line), fp)){
        char *tk= strtok(line,",");
        if(!tk) continue;
        int num= atoi(tk);
        if(num == circuitNum){
            tk= strtok(NULL,",");
            if(!tk) break;
            length= atof(tk);
            break;
        }
    }
    fclose(fp);
    ecrire_debut();
    f1->circuitLength = length;
    ecrire_fin();
}

void run_essai_1h(F1Shared *f1)
{
    ecrire_debut();
    for(int i=0;i<MAX_CARS;i++){
        f1->cars[i].remaining = 3600.0; // 1h
        f1->cars[i].status = CAR_RUNNING;
        f1->cars[i].bestLap = 9999.0;
        f1->cars[i].bestS1 = 9999.0;
        f1->cars[i].bestS2 = 9999.0;
        f1->cars[i].bestS3 = 9999.0;
        f1->cars[i].lapsDone = 0;
    }
    f1->carsFinished = 0;
    ecrire_fin();

    for(int i=0;i<MAX_CARS;i++){
        pid_t p = fork();
        if(p < 0){
            error_exit("fork essai");
        }
        if(p == 0){
            run_car_process(i);
            _exit(0);
        }
    }

    while(1){
        sleep(5);
        scoreboard_live(f1, "Essai 1H (live)", 0);
        lire_debut();
        int fin = f1->carsFinished;
        lire_fin();
        if(fin >= MAX_CARS){
            printf("Essai terminé!\n");
            break;
        }
    }

    for(int i=0;i<MAX_CARS;i++){
        wait(NULL);
    }

    scoreboard_live(f1, "Essai 1H (final)", 0);
}

void run_qualifQ1(F1Shared *f1)
{
    ecrire_debut();
    f1->carsFinished = 0;
    for(int i=0;i<MAX_CARS;i++){
        if(f1->cars[i].status != CAR_OUT){
            f1->cars[i].remaining = 18*60; // 18 min
            f1->cars[i].bestLap = 9999.0;
        }
    }
    ecrire_fin();

    for(int i=0;i<MAX_CARS;i++){
        if(f1->cars[i].status != CAR_OUT){
            pid_t p = fork();
            if(p < 0){
                error_exit("fork Q1");
            }
            if(p == 0){
                run_car_process(i);
                _exit(0);
            }
        }
    }

    while(1){
        sleep(5);
        scoreboard_live(f1, "Q1 (live)", 0);
        lire_debut();
        int fin = f1->carsFinished;
        lire_fin();
        int active = 0;
        for(int i=0;i<MAX_CARS;i++) if(f1->cars[i].status != CAR_OUT) active++;
        if(fin >= active){
            printf("Q1 terminé\n");
            break;
        }
    }

    for(int i=0;i<MAX_CARS;i++) wait(NULL);

    scoreboard_live(f1, "Q1 (final)", 0);
    eliminate_5_worst(f1);
}

void run_qualifQ2(F1Shared *f1)
{
    ecrire_debut();
    f1->carsFinished = 0;
    for(int i=0;i<MAX_CARS;i++){
        if(f1->cars[i].status != CAR_OUT){
            f1->cars[i].remaining = 15*60; // 15 min
            f1->cars[i].bestLap = 9999.0;
        }
    }
    ecrire_fin();

    for(int i=0;i<MAX_CARS;i++){
        if(f1->cars[i].status != CAR_OUT){
            pid_t p = fork();
            if(p < 0){
                error_exit("fork Q2");
            }
            if(p == 0){
                run_car_process(i);
                _exit(0);
            }
        }
    }

    while(1){
        sleep(5);
        scoreboard_live(f1, "Q2 (live)", 0);
        lire_debut();
        int fin = f1->carsFinished;
        lire_fin();
        int active = 0;
        for(int i=0;i<MAX_CARS;i++) if(f1->cars[i].status != CAR_OUT) active++;
        if(fin >= active){
            printf("Q2 terminé\n");
            break;
        }
    }

    for(int i=0;i<MAX_CARS;i++) wait(NULL);

    scoreboard_live(f1, "Q2 (final)", 0);
    eliminate_5_worst(f1);
}

void run_qualifQ3(F1Shared *f1)
{
    ecrire_debut();
    f1->carsFinished = 0;
    for(int i=0;i<MAX_CARS;i++){
        if(f1->cars[i].status != CAR_OUT){
            f1->cars[i].remaining = 12*60; // 12 min
            f1->cars[i].bestLap = 9999.0;
        }
    }
    ecrire_fin();

    for(int i=0;i<MAX_CARS;i++){
        if(f1->cars[i].status != CAR_OUT){
            pid_t p = fork();
            if(p < 0){
                error_exit("fork Q3");
            }
            if(p == 0){
                run_car_process(i);
                _exit(0);
            }
        }
    }

    while(1){
        sleep(5);
        scoreboard_live(f1, "Q3 (live)", 0);
        lire_debut();
        int fin = f1->carsFinished;
        lire_fin();
        int active = 0;
        for(int i=0;i<MAX_CARS;i++) if(f1->cars[i].status != CAR_OUT) active++;
        if(fin >= active){
            printf("Q3 terminé\n");
            break;
        }
    }

    for(int i=0;i<MAX_CARS;i++) wait(NULL);

    scoreboard_live(f1, "Q3 (final)", 0);
}

void run_course(F1Shared *f1)
{
    lire_debut();
    double cl = f1->circuitLength;
    lire_fin();
    int nbTours = (int) ceil(300.0 / cl);
    printf("=== Course de 300 km => %d tours ===\n", nbTours);
    
    ecrire_debut();
    f1->carsFinished = 0;
    for(int i=0;i<MAX_CARS;i++){
        if(f1->cars[i].status != CAR_OUT){
            f1->cars[i].remaining = nbTours;
            f1->cars[i].lapsDone = 0;
        }
    }
    ecrire_fin();

    for(int i=0;i<MAX_CARS;i++){
        if(f1->cars[i].status != CAR_OUT){
            pid_t p = fork();
            if(p < 0){
                error_exit("fork course");
            }
            if(p == 0){
                // Course-specific process
                while(1){
                    // Crash 3%
                    if((rand()%100)<3){
                        ecrire_debut();
                        f1->cars[i].status = CAR_OUT;
                        f1->carsFinished++;
                        ecrire_fin();
                        _exit(0);
                    }
                    // Pit 5%
                    int pit = (rand()%100 < 5) ? 1 : 0;
                    if(pit){
                        usleep(5000000); // 5s
                    }
                    // Un tour
                    ecrire_debut();
                    double rem = f1->cars[i].remaining;
                    if(rem <= 0.1){
                        ecrire_fin();
                        _exit(0);
                    }
                    rem -= 1.0;
                    f1->cars[i].remaining = rem;
                    f1->cars[i].lapsDone +=1;

                    // Meilleur tour
                    double s1=25.0 + (rand() % 21);
                    double s2=25.0 + (rand() % 21);
                    double s3=25.0 + (rand() % 21);
                    double lapTime = s1 + s2 + s3;
                    if(lapTime < f1->cars[i].bestLap){
                        f1->cars[i].bestLap = lapTime;
                    }
                    if(s1 < f1->cars[i].bestS1) f1->cars[i].bestS1 = s1;
                    if(s2 < f1->cars[i].bestS2) f1->cars[i].bestS2 = s2;
                    if(s3 < f1->cars[i].bestS3) f1->cars[i].bestS3 = s3;

                    if(rem <= 0.1){
                        f1->cars[i].status = CAR_OUT;
                        f1->carsFinished++;
                        ecrire_fin();
                        _exit(0);
                    } else {
                        f1->cars[i].status = (pit ? CAR_PIT : CAR_RUNNING);
                    }
                    ecrire_fin();

                    usleep(1000000); // 1s per tour
                }
            }
        }
    }

    while(1){
        sleep(5);
        scoreboard_live(f1, "Course (live)", 1);
        lire_debut();
        int fin = f1->carsFinished;
        lire_fin();
        if(fin >= MAX_CARS){
            printf("Course terminée!\n");
            break;
        }
    }

    for(int i=0;i<MAX_CARS;i++) wait(NULL);

    scoreboard_live(f1, "Course (final)", 1);
}
