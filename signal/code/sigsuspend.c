/**
 * 演示 sigsuspend 函数的使用
 * 运行程序后，发送SIGINT信号，终止程序，说明sigsuspend函数起作用，
 * 因为tset设置不阻塞，将set设置的覆盖掉
 * 重新运行程序，先发送SIGINT信号，此时打印出sig，
 * 此时sigsuspend已经返回，
 * 此时再发送SIGUSR1，程序无反应，说明掩码被重新设置为set，阻塞SIGUSR1
 * 然后
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
    sigsuspend(&tset);    // 将掩码设置为tset设置的掩码，直到有信号处理函数被调用返回后会恢复到set对应的掩码，
    while(1)
    {
        printf("loop\n");
        sleep(3);
    }
    return 0;
}
