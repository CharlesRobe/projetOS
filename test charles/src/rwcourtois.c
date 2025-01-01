#include "rwcourtois.h"
#include "utils.h"
#include <sys/sem.h>
#include <sys/types.h>

int nblect=0;
int mutex=-1;
int mutlect=-1;

static void semP(int sid)
{
    struct sembuf op;
    op.sem_num=0; 
    op.sem_op=-1; 
    op.sem_flg=0;
    if(semop(sid,&op,1)<0) error_exit("semop P");
}

static void semV(int sid)
{
    struct sembuf op;
    op.sem_num=0; 
    op.sem_op=1; 
    op.sem_flg=0;
    if(semop(sid,&op,1)<0) error_exit("semop V");
}

static int create_sem(key_t k, int initVal)
{
    int sid= semget(k,1,0666|IPC_CREAT);
    if(sid<0) error_exit("semget create_sem");
    union semun {
        int val;
        struct semid_ds* buf;
        unsigned short* array;
    } arg;
    arg.val= initVal;
    if(semctl(sid,0,SETVAL,arg)<0){
        error_exit("semctl setval");
    }
    return sid;
}

void rw_init()
{
    key_t k1= ftok(".", 'R');
    key_t k2= ftok(".", 'W');
    if(k1<0||k2<0) error_exit("ftok rw_init");
    mutex= create_sem(k1,1);
    mutlect= create_sem(k2,1);
    nblect=0;
}

void rw_cleanup()
{
    semctl(mutex,0,IPC_RMID,0);
    semctl(mutlect,0,IPC_RMID,0);
}

void lire_debut()
{
    semP(mutlect);
    nblect++;
    if(nblect==1){
        semP(mutex);
    }
    semV(mutlect);
}

void lire_fin()
{
    semP(mutlect);
    nblect--;
    if(nblect==0){
        semV(mutex);
    }
    semV(mutlect);
}

void ecrire_debut()
{
    semP(mutex);
}

void ecrire_fin()
{
    semV(mutex);
}
