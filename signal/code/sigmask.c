#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);
    while(1)
    {
        sleep(3);
        sigset_t bkset;
        if (sigpending(&bkset) == 0)
        {
            if (sigismember(&bkset, SIGINT))
            {
                printf("sigint \n");
                exit(0);
            }
        }
    }
    return 0;
}
