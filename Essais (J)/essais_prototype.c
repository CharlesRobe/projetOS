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

    /* 1) Création de la clé pour la mémoire partagée et le(s) sémaphore(s) */
    key_t key = ftok(".", 'R');
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    /* 2) Création de la mémoire partagée */
    int shmid = shmget(key, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    /* 3) Attachement au segment de mémoire partagée */
    SharedMemory *shm_ptr = (SharedMemory *) shmat(shmid, NULL, 0);
    if (shm_ptr == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    /* 4) Initialisation de la structure partagée */
    memset(shm_ptr, 0, sizeof(SharedMemory));
    shm_ptr->last_displayed_lap = -1; 
    shm_ptr->race_finished      = 0;  

    /* Lecture des données (longueur circuit, nb tours, voitures qualifiées...) */
    read_initial_data(shm_ptr,circuit_name);

    /* Allocation dynamique pour l'historique des temps et leader_times */
    allocate_dynamic_arrays(shm_ptr);

    /* On initialise la seed partagée : 
       Tous les fils vont l'incrémenter et s'en servir pour un srand(local_seed). */
    shm_ptr->random_seed = (unsigned int) time(NULL);

    /* Initialisation du CarData : on suppose qu’on a lu EXACTEMENT MAX_CARS = 10 qualifiés */
    for (int i = 0; i < MAX_CARS; i++) {
        shm_ptr->cars[i].current_lap    = 0;
        shm_ptr->cars[i].current_sector = 0;
        shm_ptr->cars[i].total_time     = 0.0;
        shm_ptr->cars[i].status         = 0;  /* 0=en course */

        /* Historique : allocation [total_laps][SECTORS_PER_LAP] */
        shm_ptr->cars[i].history = (double**) malloc(sizeof(double*) * shm_ptr->total_laps);
        for (int lap = 0; lap < shm_ptr->total_laps; lap++) {
            shm_ptr->cars[i].history[lap] = (double*) calloc(SECTORS_PER_LAP, sizeof(double));
        }
    }

    /* 5) Création + initialisation du sémaphore */
    int semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    init_semaphore(semid, SEM_MUTEX, 1);

    /* 6) Fork de processus fils (un par voiture) */
    pid_t pids[MAX_CARS];
    for (int i = 0; i < MAX_CARS; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            /* --- PROCESSUS FILS --- */
            car_process(i, shm_ptr, semid);

            /* Libération mémoire (côté fils) */
            for (int lap = 0; lap < shm_ptr->total_laps; lap++) {
                free(shm_ptr->cars[i].history[lap]);
            }
            free(shm_ptr->cars[i].history);

            /* Détachement */
            shmdt(shm_ptr);
            exit(EXIT_SUCCESS);
        } else {
            /* --- PROCESSUS PERE --- */
            pids[i] = pid;
        }
    }

    /* 7) Le père attend la fin de tous les fils */
    for (int i = 0; i < MAX_CARS; i++) {
        waitpid(pids[i], NULL, 0);
    }

    /* On signale la fin de la course (si nécessaire) */
    semaphore_wait(semid, SEM_MUTEX);
    shm_ptr->race_finished = 1;
    semaphore_signal(semid, SEM_MUTEX);

    /* 8) Affichage et sauvegarde des résultats finaux */
    printf("\n----- Course terminée -----\n\n");
    /* Exemple d'affichage simple du résultat final */
    for (int i = 0; i < MAX_CARS; i++) {
        CarData *car = &shm_ptr->cars[i];
        printf("Voiture %2d | Temps total: %6.2f s | Statut: %s\n",
               car->car_id,
               car->total_time,
               (car->status == 2) ? "Abandonnée" :
               (car->status == 1) ? "Aux stands"  : "Terminé/En course");
    }

    /* Sauvegarde des résultats dans un fichier (simplifié) */
    write_results_to_file(circuit_name,shm_ptr);

    /* 9) Nettoyage de la mémoire partagée et des sémaphores */
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl");
    }

    return 0;
}