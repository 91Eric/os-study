/**
 * 示例SIGCHILD信号的使用，以及SA_NOCLDSTOP标志的使用对信号的影响
 * 使用SA_NOCLDSTOP标志后无论waitpid是否有相应标志都不会产生SIGCHILD信号
*/

#ifndef _POSIX_SOURCE
    #define _POSIX_SOURCE
#endif

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static void sig_handler(int sig_num)
{

    if (sig_num == SIGCHLD)
    {
        printf("sigchild\n");
        int status = 0;
        pid_t ret = -1;
        // 这里可以不处理STOP和CONT信号，此时只等待终止进程，加上就会等待停止或开始SA_NOCLDSTOP
        // 如果不加，那么、waitpid对于这两个信号产生的SIGCHILD信号毫无反应，ret为0，因为只产生不接收
        // 如果加上WUNTRACED 和 WCONTINUED，waitpi对于STOP响应，但是CONT却会同时产生EINTR，暂时没想明白这个信号哪来的
        while((ret = waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED)) > 0)
        {
            printf("wait ret: %d...\n", ret);
            if (errno == EINTR)
            {
                continue;
            }
            if (WIFSIGNALED(status))
            {
                printf("child %d exit by signal:%d(%s)\n", ret, WTERMSIG(status), strerror(WTERMSIG(status)));
            }
            if (WIFSTOPPED(status))
            {
                printf("child %d stoped by signal:%d:(%s)\n", ret, WSTOPSIG(status), strerror(WSTOPSIG(status)));
            }
            if (WIFCONTINUED(status))
            {
                printf("child %d continued by signal:SIGCONT\n", ret);
            }
            
        }
        if (ret == 0)
        {
            printf("no child terminal\n");
        }
        else
        {
            perror("wait error");
        }
    }
}

int main(int argc, char const *argv[])
{
    struct sigaction sig;
    // 默认情况下，子进程停止或重新运行会向父进程发送SIGCHILD信号
    
    sig.sa_flags = 0;
    sig.sa_handler = sig_handler;
    sigemptyset(&sig.sa_mask);

    if (sigaction(SIGCHLD, &sig, NULL) < 0)
    {
        perror("sigaction error:");       
    }

    pid_t child = -1;
    child = fork();

    if (child == -1)
    {
        perror("fork failed");
        exit(-1);
    }
    else if (child == 0)
    {
        printf("pid:%d\n", getpid());
        do 
        {
            printf("child\n");
            sleep(2);
        } while(1);

    }
    else
    {
        while(1)
        {
            sleep(1);
            printf("father\n");
        }        

    }
    return 0;
}
