#include "race.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

// Les 20 numéros
static int carNumbers[MAX_CARS] = {
    1, 11, 44, 63, 16, 55, 4, 81, 14, 18,
    10, 31, 23, 2, 22, 3, 77, 24, 20, 27
};

// Convertit un SessionType en chaîne plus sympa
static const char* session_name(SessionType st)
{
    switch(st){
        case SESS_ESSAI1: return "Essai1";
        case SESS_ESSAI2: return "Essai2";
        case SESS_ESSAI3: return "Essai3";
        case SESS_Q1:     return "Q1";
        case SESS_Q2:     return "Q2";
        case SESS_Q3:     return "Q3";
        case SESS_SPRINT: return "Sprint";
        case SESS_COURSE: return "Course";
        default:          return "None/Finished";
    }
}

void init_new_gp(GPState *gp,
                 int circuitNumber,
                 int isSprint,
                 Circuit circuits[],
                 int nbCirc)
{
    memset(gp, 0, sizeof(GPState));
    gp->isSprint = isSprint;

    // On cherche dans circuits[] un qui a .numeroCourse == circuitNumber
    int found = -1;
    for(int i=0; i<nbCirc; i++){
        if(circuits[i].numeroCourse == circuitNumber){
            found = i;
            break;
        }
    }
    if(found<0){
        // pas trouvé => on prend le 1er
        found=0;
    }
    gp->circuit = circuits[found];

    // Nom du GP = "Circuit <numero>"
    snprintf(gp->gpName, sizeof(gp->gpName),
             "Circuit #%d", circuitNumber);

    // On commence par Essai1
    gp->currentSession = SESS_ESSAI1;

    // Init des voitures
    for(int i=0; i<MAX_CARS; i++){
        gp->cars[i].pid = 0;
        gp->cars[i].carNumber = carNumbers[i];
        gp->cars[i].status = CAR_RUNNING;
        gp->cars[i].bestLap = 9999.0;
        gp->cars[i].bestS1 = 9999.0;
        gp->cars[i].bestS2 = 9999.0;
        gp->cars[i].bestS3 = 9999.0;
        gp->cars[i].points = 0;
    }
}

void run_session(GPState *gp)
{
    if(gp->currentSession==SESS_NONE || gp->currentSession==SESS_FINISHED){
        printf("Aucune session à lancer.\n");
        return;
    }

    printf("\n=== [GP: %s | Session: %s] ===\n",
           gp->gpName, session_name(gp->currentSession));

    // 1) pipe par voiture
    int pipefd[MAX_CARS][2];
    for(int i=0; i<MAX_CARS; i++){
        if(pipe(pipefd[i])<0){
            error_exit("pipe");
        }
    }

    // 2) Fork
    for(int i=0; i<MAX_CARS; i++){
        pid_t p = fork();
        if(p<0){
            error_exit("fork");
        }
        else if(p==0){
            // enfant
            close(pipefd[i][0]); // on lit pas
            run_car_process(i, pipefd[i][1]);
        }
        else {
            gp->cars[i].pid = p;
            close(pipefd[i][1]); // on écrit pas
        }
    }

    // 3) Durée
    // ex. Essai1,2,3 => 8s, Q1->6, Q2->6, Q3->6, Sprint->5, Course->10
    int duration=8;
    switch(gp->currentSession){
        case SESS_Q1: case SESS_Q2: case SESS_Q3:
            duration=6; break;
        case SESS_SPRINT:
            duration=5; break;
        case SESS_COURSE:
            duration=10; break;
        default:
            duration=8;
    }

    // 4) refresh
    int steps = duration*(1000/2000); // ex. 8s -> 4 steps
    for(int s=0; s<steps; s++){
        // lecture non bloquante
        for(int i=0; i<MAX_CARS; i++){
            // on lit 3 doubles s1, s2, s3
            while(1){
                double s1, s2, s3;
                int r = read(pipefd[i][0], &s1, sizeof(double));
                if(r<=0) break; // plus de data
                // on lit s2, s3
                int r2 = read(pipefd[i][0], &s2, sizeof(double));
                int r3 = read(pipefd[i][0], &s3, sizeof(double));
                if(r2>0 && r3>0){
                    // on a un tour complet
                    double lapTime = s1 + s2 + s3;
                    if(lapTime < gp->cars[i].bestLap){
                        gp->cars[i].bestLap = lapTime;
                    }
                    if(s1 < gp->cars[i].bestS1) gp->cars[i].bestS1 = s1;
                    if(s2 < gp->cars[i].bestS2) gp->cars[i].bestS2 = s2;
                    if(s3 < gp->cars[i].bestS3) gp->cars[i].bestS3 = s3;
                }
                else {
                    // data incomplete
                    break;
                }
            }
        }

        // tri
        sort_cars_by_bestlap(gp);

        // affiche live
        printf("\n--- [Live scoreboard: %s, step=%d] ---\n",
               session_name(gp->currentSession), s);
        display_classification(gp, "Classement provisoire", 1);

        sleep(2);
    }

    // 5) fin => kill
    for(int i=0; i<MAX_CARS; i++){
        if(gp->cars[i].pid>0){
            kill(gp->cars[i].pid, SIGTERM);
        }
    }
    // wait
    for(int i=0; i<MAX_CARS; i++){
        if(gp->cars[i].pid>0){
            waitpid(gp->cars[i].pid, NULL, 0);
            gp->cars[i].pid=0;
        }
    }

    // 6) relire le buffer final
    for(int i=0; i<MAX_CARS; i++){
        while(1){
            double s1, s2, s3;
            int r = read(pipefd[i][0], &s1, sizeof(double));
            if(r<=0) break;
            int r2 = read(pipefd[i][0], &s2, sizeof(double));
            int r3 = read(pipefd[i][0], &s3, sizeof(double));
            if(r2>0 && r3>0){
                double lapTime = s1+s2+s3;
                if(lapTime < gp->cars[i].bestLap){
                    gp->cars[i].bestLap = lapTime;
                }
                if(s1<gp->cars[i].bestS1) gp->cars[i].bestS1=s1;
                if(s2<gp->cars[i].bestS2) gp->cars[i].bestS2=s2;
                if(s3<gp->cars[i].bestS3) gp->cars[i].bestS3=s3;
            } else {
                break;
            }
        }
        close(pipefd[i][0]);
    }

    // tri final
    sort_cars_by_bestlap(gp);

    // affiche final
    printf("\n=== Classement final de la session %s ===\n",
           session_name(gp->currentSession));
    display_classification(gp, "Classement final", 0);
}

void next_session(GPState *gp)
{
    switch(gp->currentSession){
        case SESS_NONE:
            gp->currentSession=SESS_ESSAI1; break;

        case SESS_ESSAI1:
            gp->currentSession=SESS_ESSAI2; break;
        case SESS_ESSAI2:
            gp->currentSession=SESS_ESSAI3; break;
        case SESS_ESSAI3:
            gp->currentSession=SESS_Q1; break;
        case SESS_Q1:
            gp->currentSession=SESS_Q2; break;
        case SESS_Q2:
            gp->currentSession=SESS_Q3; break;
        case SESS_Q3:
            if(gp->isSprint){
                gp->currentSession=SESS_SPRINT;
            } else {
                gp->currentSession=SESS_COURSE;
            }
            break;
        case SESS_SPRINT:
            gp->currentSession=SESS_COURSE; break;
        case SESS_COURSE:
        default:
            gp->currentSession=SESS_FINISHED; break;
    }
}

void end_session(GPState *gp)
{
    SessionType st= gp->currentSession;
    // si sprint => points 8..1
    if(st==SESS_SPRINT){
        int sprintPts[8] = {8,7,6,5,4,3,2,1};
        for(int i=0; i<8; i++){
            gp->cars[i].points += sprintPts[i];
        }
    }
    else if(st==SESS_COURSE){
        // top10 => 25..1
        int racePts[10] = {25,20,15,10,8,6,5,3,2,1};
        for(int i=0; i<10; i++){
            gp->cars[i].points += racePts[i];
        }
        // +1 point meilleur tour (voiture #0 du tri)
        gp->cars[0].points +=1;
    }
}

void sort_cars_by_bestlap(GPState *gp)
{
    for(int i=0; i<MAX_CARS-1; i++){
        for(int j=i+1; j<MAX_CARS; j++){
            if(gp->cars[j].bestLap < gp->cars[i].bestLap){
                Car tmp=gp->cars[i];
                gp->cars[i]=gp->cars[j];
                gp->cars[j]=tmp;
            }
        }
    }
}

void display_classification(const GPState *gp, const char *title, int liveMode)
{
    if(liveMode){
        printf("(LIVE) ");
    }
    print_ascii_table_header(title);
    for(int i=0; i<MAX_CARS; i++){
        print_ascii_table_row(i+1,
                              gp->cars[i].carNumber,
                              gp->cars[i].bestLap,
                              gp->cars[i].points);
    }
    print_ascii_table_footer();
}

void save_final_classification(const GPState *gp, const char *filename)
{
    // tri desc par points
    Car sorted[MAX_CARS];
    for(int i=0; i<MAX_CARS; i++){
        sorted[i] = gp->cars[i];
    }
    for(int i=0; i<MAX_CARS-1; i++){
        for(int j=i+1; j<MAX_CARS; j++){
            if(sorted[j].points>sorted[i].points){
                Car tmp=sorted[i];
                sorted[i]=sorted[j];
                sorted[j]=tmp;
            }
        }
    }

    FILE *f = fopen(filename, "w");
    if(!f){
        perror("fopen");
        return;
    }
    fprintf(f, "=== Classement final GP: %s ===\n", gp->gpName);
    fprintf(f, "POS | Car# | Points | BestLap\n");
    for(int i=0; i<MAX_CARS; i++){
        fprintf(f, "%2d  | %3d |  %3d   | %.3f\n",
                i+1,
                sorted[i].carNumber,
                sorted[i].points,
                sorted[i].bestLap);
    }
    fclose(f);
    printf("Classement final dans '%s'\n", filename);
}
