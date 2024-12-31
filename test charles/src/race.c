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
        for(int i=0;i<19;i++){
            for(int j=i+1;j<20;j++){
                if(localC[j].bestLap< localC[i].bestLap){
                    Car tmp= localC[i]; localC[i]= localC[j]; localC[j]= tmp;
                }
            }
        }
    } else {
        for(int i=0;i<19;i++){
            for(int j=i+1;j<20;j++){
                if(localC[j].lapsDone> localC[i].lapsDone){
                    Car tmp= localC[i]; localC[i]= localC[j]; localC[j]= tmp;
                }
            }
        }
    }
    printf("\n[%s] (finished=%d)\n", title, fin);
    for(int i=0;i<20;i++){
        char st[8]="RUN";
        if(localC[i].status==CAR_PIT) strcpy(st,"PIT");
        else if(localC[i].status==CAR_OUT) strcpy(st,"OUT");
        printf(" Car#%2d: bestLap=%.3f remain=%.1f laps=%d status=%s\n",
               localC[i].carNumber,
               localC[i].bestLap,
               localC[i].remaining,
               localC[i].lapsDone,
               st);
    }
}

// Elimine 5 pires bestLap
static void eliminate_5_worst(F1Shared *f1)
{
    ecrire_debut();
    Car copy[20];
    memcpy(copy, f1->cars,sizeof(copy));
    ecrire_fin();

    // tri local
    for(int i=0;i<19;i++){
        for(int j=i+1;j<20;j++){
            if(copy[j].bestLap< copy[i].bestLap){
                Car tmp= copy[i];
                copy[i]= copy[j];
                copy[j]= tmp;
            }
        }
    }
    ecrire_debut();
    for(int i=15; i<20; i++){
        int cn= copy[i].carNumber;
        for(int k=0;k<20;k++){
            if(f1->cars[k].carNumber== cn && f1->cars[k].status!=CAR_OUT){
                f1->cars[k].status=CAR_OUT;
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
    if(!fgets(line,sizeof(line),fp)){
        fclose(fp);
        error_exit("csv empty");
    }
    double length=5.412;
    while(fgets(line,sizeof(line),fp)){
        char *tk= strtok(line,",");
        if(!tk) continue;
        int num= atoi(tk);
        if(num== circuitNum){
            tk= strtok(NULL,",");
            if(!tk) break;
            length= atof(tk);
            break;
        }
    }
    fclose(fp);
    ecrire_debut();
    f1->circuitLength= length;
    ecrire_fin();
}

void run_essai_1h(F1Shared *f1)
{
    ecrire_debut();
    for(int i=0;i<MAX_CARS;i++){
        f1->cars[i].remaining= 3600.0;
        f1->cars[i].status= CAR_RUNNING;
        f1->cars[i].bestLap=9999.0;
        f1->cars[i].bestS1=9999.0;
        f1->cars[i].bestS2=9999.0;
        f1->cars[i].bestS3=9999.0;
        f1->cars[i].lapsDone=0;
    }
    f1->carsFinished=0;
    ecrire_fin();

    for(int i=0;i<20;i++){
        pid_t p=fork();
        if(p==0){
            run_car_process(i);
            _exit(0);
        }
    }
    while(1){
        sleep(5);
        scoreboard_live(f1,"ESSAI live",0);
        lire_debut();
        int fin= f1->carsFinished;
        lire_fin();
        if(fin>=20){
            printf("Essai terminé!\n");
            break;
        }
    }
    for(int i=0;i<20;i++){
        wait(NULL);
    }
    scoreboard_live(f1,"ESSAI final",0);
}

void run_qualifQ1(F1Shared *f1)
{
    ecrire_debut();
    f1->carsFinished=0;
    for(int i=0;i<20;i++){
        if(f1->cars[i].status!=CAR_OUT){
            f1->cars[i].remaining= 18*60;
            f1->cars[i].bestLap=9999.0;
        }
    }
    ecrire_fin();
    for(int i=0;i<20;i++){
        if(f1->cars[i].status!=CAR_OUT){
            pid_t p=fork();
            if(p==0){
                run_car_process(i);
                _exit(0);
            }
        }
    }
    while(1){
        sleep(5);
        scoreboard_live(f1,"Q1 live",0);
        lire_debut();
        int fin= f1->carsFinished;
        lire_fin();
        if(fin>=20){
            printf("Q1 terminé\n");
            break;
        }
    }
    for(int i=0;i<20;i++){
        wait(NULL);
    }
    scoreboard_live(f1,"Q1 final",0);
    eliminate_5_worst(f1);
}

void run_qualifQ2(F1Shared *f1)
{
    ecrire_debut();
    f1->carsFinished=0;
    for(int i=0;i<20;i++){
        if(f1->cars[i].status!=CAR_OUT){
            f1->cars[i].remaining= 15*60;
            f1->cars[i].bestLap=9999.0;
        }
    }
    ecrire_fin();
    for(int i=0;i<20;i++){
        if(f1->cars[i].status!=CAR_OUT){
            pid_t p=fork();
            if(p==0){
                run_car_process(i);
                _exit(0);
            }
        }
    }
    while(1){
        sleep(5);
        scoreboard_live(f1,"Q2 live",0);
        lire_debut();
        int fin= f1->carsFinished;
        lire_fin();
        int active=0;
        for(int i=0;i<20;i++){
            if(f1->cars[i].status!=CAR_OUT) active++;
        }
        if(fin>=active){
            printf("Q2 terminé\n");
            break;
        }
    }
    for(int i=0;i<20;i++){
        wait(NULL);
    }
    scoreboard_live(f1,"Q2 final",0);
    eliminate_5_worst(f1);
}

void run_qualifQ3(F1Shared *f1)
{
    ecrire_debut();
    f1->carsFinished=0;
    for(int i=0;i<20;i++){
        if(f1->cars[i].status!=CAR_OUT){
            f1->cars[i].remaining= 12*60;
            f1->cars[i].bestLap=9999.0;
        }
    }
    ecrire_fin();
    for(int i=0;i<20;i++){
        if(f1->cars[i].status!=CAR_OUT){
            pid_t p=fork();
            if(p==0){
                run_car_process(i);
                _exit(0);
            }
        }
    }
    while(1){
        sleep(5);
        scoreboard_live(f1,"Q3 live",0);
        lire_debut();
        int fin= f1->carsFinished;
        lire_fin();
        int act=0;
        for(int i=0;i<20;i++){
            if(f1->cars[i].status!=CAR_OUT) act++;
        }
        if(fin>=act){
            printf("Q3 terminé\n");
            break;
        }
    }
    for(int i=0;i<20;i++){
        wait(NULL);
    }
    scoreboard_live(f1,"Q3 final",0);
}

void run_course(F1Shared* f1)
{
    lire_debut();
    double cl= f1->circuitLength;
    lire_fin();
    int nbTours= (int) ceil(300.0 / cl);
    printf("Course => %d tours\n", nbTours);
    ecrire_debut();
    f1->carsFinished=0;
    for(int i=0;i<20;i++){
        if(f1->cars[i].status!=CAR_OUT){
            f1->cars[i].remaining= nbTours;
            f1->cars[i].lapsDone=0;
        }
    }
    ecrire_fin();
    for(int i=0;i<20;i++){
        if(f1->cars[i].status!=CAR_OUT){
            pid_t p=fork();
            if(p==0){
                int cidx=i;
                int sid= shmget(SHM_KEY,sizeof(F1Shared),0);
                F1Shared *f2= (F1Shared*) shmat(sid,NULL,0);
                srand(time(NULL) ^ (getpid()<<16));
                while(1){
                    if((rand()%100)<3){
                        ecrire_debut();
                        f2->cars[cidx].status=CAR_OUT;
                        f2->carsFinished++;
                        ecrire_fin();
                        _exit(0);
                    }
                    int pit=(rand()%100<5)?1:0;
                    if(pit){
                        usleep(5000000);
                    }
                    ecrire_debut();
                    double rr= f2->cars[cidx].remaining;
                    if(rr<=0.1){
                        ecrire_fin();
                        _exit(0);
                    }
                    rr-=1.0;
                    f2->cars[cidx].remaining= rr;
                    f2->cars[cidx].lapsDone++;
                    double s1=25+(rand()%21);
                    double s2=25+(rand()%21);
                    double s3=25+(rand()%21);
                    double lapt= s1+s2+s3;
                    if(lapt< f2->cars[cidx].bestLap) f2->cars[cidx].bestLap=lapt;
                    if(s1< f2->cars[cidx].bestS1) f2->cars[cidx].bestS1=s1;
                    if(s2< f2->cars[cidx].bestS2) f2->cars[cidx].bestS2=s2;
                    if(s3< f2->cars[cidx].bestS3) f2->cars[cidx].bestS3=s3;
                    if(rr<=0.1){
                        f2->cars[cidx].status=CAR_OUT;
                        f2->carsFinished++;
                        ecrire_fin();
                        _exit(0);
                    } else {
                        f2->cars[cidx].status= (pit? CAR_PIT: CAR_RUNNING);
                    }
                    ecrire_fin();
                    usleep(1000000);
                }
            }
        }
    }
    while(1){
        sleep(5);
        scoreboard_live(f1,"Course live",1);
        lire_debut();
        int fin= f1->carsFinished;
        lire_fin();
        if(fin>=20){
            printf("Course terminée\n");
            break;
        }
    }
    for(int i=0;i<20;i++){
        wait(NULL);
    }
    scoreboard_live(f1,"Course final",1);
}
