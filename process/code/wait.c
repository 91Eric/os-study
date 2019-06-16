/**
 * 验证wait函数基本使用
 * 向子进程发送SIGKILL 信号，此时主进程会收到子进程终止的消息
 * 向子进程发送SIGSTOP 信号，此时主进程不会收到子进程停止消息
 * 主进程阻塞在wait，因为不会输出wait...
*/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main()
{
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
        int status = 0;
        pid_t ret = -1;
        while((ret = wait(&status)) != -1)
        {
            printf("wait...\n");
            if (WIFSIGNALED(status))
            {
                printf("child %d exit by signal:%d(%s)", ret, WTERMSIG(status), strerror(WTERMSIG(status)));
            }
        }
        perror("wait error");
    }
    return 0;
}