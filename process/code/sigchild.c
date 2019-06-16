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
        // 这里无论加不加后两个标志都无关紧要，因为使用了SA_NOCLDSTOP，根本不会在停止或开始的时候发送SIGCHILD信号
        while(((ret = waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED)) > 0)  )
        {
            printf("wait ret: %d...\n", ret);
            perror("errno:");
            if (WIFSIGNALED(status))
            {
                printf("child %d exit by signal:%d(%s)\n", ret, WTERMSIG(status), strerror(WTERMSIG(status)));
            }
            if (WIFSTOPPED(status))
            {
                printf("child %d stoped by signal:%d:(%s)\n", ret, WSTOPSIG(status), strerror(WSTOPSIG(status)));
            }
            
        }
        perror("wait error");
    }
}

int main(int argc, char const *argv[])
{
    struct sigaction sig;
    // 此时子进程停止或重新运行不会向父进程发送SIGCHILD信号
    sig.sa_flags |= SA_NOCLDSTOP;
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
