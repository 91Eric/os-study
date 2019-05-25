#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc <= 2)
    {
        printf("param is error \nusage:\n" 
        "./a.out pid signum\n");
        raise(SIGKILL);   //　发信号给自己杀死自己
    }
    int pid = strtol(argv[1], NULL, 10);
    int sig = strtol(argv[2], NULL, 10);

    printf("pid is %d, sig is %d\n", pid, sig);
    int k = kill(pid, sig);
    if (sig != 0)
    {
        if (k == -1)
        {
            perror("kill err is");
        }
    }
    else
    {
        if (k == 0)
        {
            printf("process exist\n");
        }
        else 
        {
            if (errno == EPERM)
            {
                printf("no acess\n");
            }
            else if (errno == ESRCH)
            {
                printf("no process\n");
            }
            else
            {
                perror("error\n");
            }
        }
    }

}