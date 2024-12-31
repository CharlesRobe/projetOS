#include "f1shared.h"
#include "utils.h"
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>

int create_shm()
{
    int sid= shmget(SHM_KEY, sizeof(F1Shared),0666|IPC_CREAT);
    if(sid<0) error_exit("shmget create_shm");
    return sid;
}
F1Shared* attach_shm(int shmid)
{
    void* ptr= shmat(shmid,NULL,0);
    if(ptr==(void*)-1) error_exit("shmat attach");
    return (F1Shared*) ptr;
}
void detach_shm(F1Shared *f1)
{
    shmdt(f1);
}
void remove_shm(int shmid)
{
    shmctl(shmid,IPC_RMID,NULL);
}
