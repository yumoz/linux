#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define N 64

typedef struct
{
    pid_t pid;
    char buf[N];
} SHM;

void handler(int sig_no)
{
    return;
}

int main()
{
    key_t key;
    int shmid;
    SHM *p;
    pid_t pid;

    if ((key = ftok(".", 'm')) < 0)
    {
        perror("fail to ftok");
        exit(-1);
    }

    signal(SIGUSR1, handler);
    if ((shmid = shmget(key, sizeof(SHM), 0666|IPC_CREAT|IPC_EXCL)) < 0)
    {
        if (EEXIST == errno)
        {
            shmid = shmget(key, sizeof(SHM), 0666);
            p = (SHM *)shmat(shmid, NULL, 0);
            pid = p->pid;
            p->pid = getpid();//把自己的pid写到共享内存
            kill(pid, SIGUSR1);
        }
        else//fail
        {
            perror("fail to shmget");
            exit(-1);
        }
    }
    else//success
    {
        p = (SHM *)shmat(shmid, NULL, 0);
        p->pid = getpid();
        pause();
        pid = p->pid;//得到写端进程的pid
    }

    while ( 1 )
    {
        pause();//堵塞，等待信号
        if (strcmp(p->buf, "quit\n") == 0) exit(0);//输入"quit结束"
        printf("read from shm : %s", p->buf);
        kill(pid, SIGUSR1);//向写进程发SIGUSR1信号
    }
    shmdt(p);
    shmctl(shmid,IPC_RMID,NULL);
    return 0;
}
