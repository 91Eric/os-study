#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static void sig_hand(int sig)
{
    printf("sig \n");
}

int main(int argc, char const *argv[])
{
    sigset_t set, tset;
    sigemptyset(&set);
    sigemptyset(&tset);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);
    signal(SIGINT, sig_hand);
    sigsuspend(&tset);    // 调用完成后会恢复到set对应的掩码，
    while(1)
    {
        printf("loop\n");
        sleep(3);
    }
    return 0;
}
