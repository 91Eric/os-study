/**
 * @brief: 这个程序用于验证sigprocmask 的作用，
 * 此程序运行后会发现不会对SIGINT 信号响应，即程序依旧运行，
 * 并会打印sigint（正常情况下默认会终止程序）
*/

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
