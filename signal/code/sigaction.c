/**
 * @brief: 基本的sigaction函数使用示例
 * 没有设置标志位，原因，当前内核不支持一些标志
 * 没有设置阻塞信号 
*/
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE    200112L
#endif
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static void sh(int sn)
{
    printf("sig\n");
}

int main()
{
    struct sigaction sig;
    sig.sa_handler = sh;
    sigemptyset(&sig.sa_mask);

    if ( sigaction(SIGINT, &sig, NULL) )
    {
        perror("sigaction error:");
    }
    while(1)
    {
        sleep(2);
        printf("loop\n");
    }
    return 0;
}