#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Mettre à 1 pour activer les printf() de debug.
int debug_prints = 1;

// Conversion #secondes en mm:ss,ms (Utilisée pour affichage classement)
char* timeFormat(float seconds) {
    int time_minutes = seconds / 60.0, time_seconds = seconds - (time_minutes * 60),
        time_milliseconds = (seconds - (time_minutes * 60) - time_seconds) * 1000;
    char formatted_time[100];
    sprintf(formatted_time, "%d:%d,%d", time_minutes, time_seconds, time_milliseconds);
    return formatted_time;
}

// Génération aléatoire d'un nombre entre num_min et num_max.
float genRanNum(int seed_incrementer, int num_min, int num_max) {
    srand(time(NULL) + seed_incrementer);
    float rand_num = ((rand() % (((num_max - num_min)*1000) + 1) ) + (num_min*1000)) / 1000.0;
    return rand_num;
}

// Génération d'un scénario de tour effectué par une voiture.
float doingLap(int *seed, int car_number, float *trials_clock) {
    if (debug_prints == 1) { printf("[doingLap()] Car n°%d started a lap.\n", car_number); }
    int seed_incrementer = *seed,
        lap_outcome = genRanNum(seed_incrementer++, 25, 45);
    float time_sec1, time_sec2, time_sec3, time_total;
    if (lap_outcome == 100) { // La voiture se crash.
        int sector_crash = genRanNum(seed_incrementer++, 1, 3); // Dans quel secteur ?
        if (sector_crash == 1) { // Secteur 1 → Aucun secteur n'enregistre un temps.
            time_sec1 = INFINITY;
            time_sec2 = INFINITY;
            time_sec3 = INFINITY;
        }
        else if (sector_crash == 2) { // Secteur 2 → Seul secteur 1 reçoit un temps.
            time_sec1 = genRanNum(seed_incrementer++, 25, 45);
            time_sec2 = INFINITY;
            time_sec3 = INFINITY;
        }
        else if (sector_crash == 3) { // Secteur 3 → Seul secteur 3 ne reçoit pas de temps.
            time_sec1 = genRanNum(seed_incrementer++, 25, 45);
            time_sec2 = genRanNum(seed_incrementer++, 25, 45);
            time_sec3 = INFINITY;
        }
        else {
            printf("[doingLap()] ERROR: sector_crash.\n");
        }
        if (debug_prints == 1) { printf("[doingLap()] Car n°%d crashed in sector %d.\n", car_number, sector_crash); }
        if (debug_prints == 1) { printf("[doingLap()] Generated times: %f, %f, %f, %f.\n", time_sec1, time_sec2, time_sec3, time_total); }
    }
    else if (lap_outcome > 84) { // La voiture s'arrête aux stands.
        time_sec1 = genRanNum(seed_incrementer++, 25, 45);
        time_sec2 = genRanNum(seed_incrementer++, 25, 45);
        float remaining_time = 3600 - *trials_clock; // Combien de temps avant la fin des essais ?
        time_sec3 = genRanNum(seed++, 300, remaining_time);
        time_total = time_sec1 + time_sec2 + time_sec3;
        if (debug_prints == 1) { printf("[doingLap()] Car n°%d stopped in stands for %f seconds.\n", car_number, time_sec3); }
        if (debug_prints == 1) { printf("[doingLap()] Generated times are: %f, %f, %f, %f.\n", time_sec1, time_sec2, time_sec3, time_total); }
    }
    else { // Tour classique.
        time_sec1 = genRanNum(seed_incrementer++, 25, 45);
        time_sec2 = genRanNum(seed_incrementer++, 25, 45);
        time_sec3 = genRanNum(seed_incrementer++, 25, 45);
        time_total = time_sec1 + time_sec2 + time_sec3;
        if (debug_prints == 1) { printf("[doingLap()] Car n°%d reached the final line.\n", car_number); }
        if (debug_prints == 1) { printf("[doingLap()] Generated times are: %f, %f, %f, %f.\n", time_sec1, time_sec2, time_sec3, time_total); }
    }
    sleep(time_total);
    if (debug_prints == 1) { printf("[doingLap()] Car n°%d completed his lap.\n", car_number); }    
    // Retourner les temps effectués → tsec1, tsec2 et tsec3. pour que essais puissent les comparer aux meilleurs.
    return 0;
}

// Fonction continuellement exécutée par une voiture = Donne "vie" à une voiture.
void car_process(float *trials_clock, int *seed, int car_number) {
    if (debug_prints == 1) { printf("[car_process()] car_process function initiated.\n"); }
    doingLap(&seed, &car_number, &trials_clock);
    if (debug_prints == 1) { printf("[car_process()] car_process function ended.\n"); }
}

void main() {
    int seed = 0;
    float *trials_clock;
    float best_time_sec1 = INFINITY, best_time_sec2 = INFINITY, best_time_sec3 = INFINITY,
        best_time_total = INFINITY;
    
    int shmid = shmget(1009, sizeof(tabVoitures), IPC_CREAT | 0666);
    trials_clock = shmat(shmid, NULL, 0);


    pid_t main_clock, car_1, car_11, car_44, car_63, car_16, car_55, car_4, car_81, car_14, car_18,
        car_10, car_31, car_23, car_2, car_22, car_3, car_77, car_24, car_20, car_27;
    int pipefd_car_1[2], pipefd_car_11[2], pipefd_car_44[2], pipefd_car_63[2],
        pipefd_car_16[2], pipefd_car_55[2], pipefd_car_4[2], pipefd_car_81[2],
        pipefd_car_14[2], pipefd_car_18[2],	pipefd_car_10[2], pipefd_car_31[2],
        pipefd_car_23[2], pipefd_car_2[2], pipefd_car_22[2], pipefd_car_3[2],
        pipefd_car_77[2], pipefd_car_24[2], pipefd_car_20[2], pipefd_car_27[2];
    pipe(pipefd_car_1);

    if (!(main_clock = fork())) {
        if (debug_prints == 1) { printf("[main_clock] Started child process for main_clock.\n"); }
        while (trials_clock < 3600) {
            printf("[main_clock] Clock: %f\n", *trials_clock);
            sleep(1);
        }
        exit(0);
    }
    else if (!(car_1 = fork())) {
        if (debug_prints == 1) { printf("[car_1] Started child process for car n°1.\n"); }
        close(pipefd_car_1[0]); // Fermeture canal écriture pour le fils.
        dup2(pipefd_car_1[1], 1);
        printf("[car_1] Trials_clock initial value: %f\n", trials_clock);
        while (trials_clock < 3600) {
            printf("[car_1] trials_clock current value is: %f\n", *trials_clock);
            sleep(1);
        }
        //car_process(&trials_clock, &seed, 1);
        /*while (trials_clock < 3600) {
            sleep(1);
            printf("doingLap initiated.\n");
            //doingLap(&seed, 1, &trials_clock);
            printf("doingLap terminated.\n");
        }*/
        
    }
    /*
    else if (!(car_11 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°11.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_44 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°44.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_63 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°63.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_16 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°16.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_55 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°55.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_4 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°4.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_81 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°81.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_14 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°14.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_18 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°18.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_10 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°10.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_31 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°31.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_23 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°23.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_2 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°2.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_22 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°22.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_3 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°3.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_77 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°77.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_24 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°24.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_20 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°20.\n"); }
        sleep(5);
        exit(0);
    } else if (!(car_27 = fork())) {
        if (debug_prints == 1) { printf("Started child process for car n°27.\n"); }
        sleep(5);
        exit(0);
    } */
    else {
        // parent
        printf("[main] Back into parent process.\n");
        /*while (trials_temporal_progression < 3600) {
            trials_temporal_progression++;
            sleep(1);
        }*/
        // read(pipefd_car1[0]);
        char output_pipe[256];
        read(pipefd_car_1[0], output_pipe, sizeof(output_pipe) - 1);
        printf("%s", output_pipe); // Donne la parole à un processus fils.

        wait(&main_clock);
            printf("[main] main_clock process exited with code 0.\n");
        wait(&car_1);
            printf("[main] car_1 process exited with code 0.\n");
        //doingLap(&seed, 1, &trials_clock);
    }
}