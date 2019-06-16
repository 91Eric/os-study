# wait 和 waitpid 
> 父进程如何获知子进程状态变化，终止或者收到诸如SIGSTOP等停止，可以使用wait , waitpid 或者信号SIIGCHLD

## wait 
> 这个函数**可以且仅可以**阻塞的监听任一子进程 ***终止***，同时可以通过参数返回子进程的 ***终止状态***，对于因为收到信号停止，函数并不会有响应
### 函数定义
```
#include <sys/types.h>
#include <sys/wait.h>

pid_t wait(int *status);

```
### 参数
> status: 子进程终止状态，如果为NULL,则不关心终止状态

### 返回值
> 正常情况下返回退出子进程的pid，失败返回-1，此时errno常见的错误是ECHILD，表示此进程没有要等待的子进程，其实就是父进程根本没有子进程

### 例程
```
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
            if (WIFSIGNALED(status))
            {
                printf("child %d exit by signal:%d(%s)", ret, WTERMSIG(status), strerror(WTERMSIG(status)));
            }
        }
        perror("wait error");
    }
    return 0;
}
```
>  这个程序运行后，对子进程发送除了SIGSTOP,SIGTTIN 外的默认是结束进程的信号，父进程能够捕获到子进程退出，但是使用SIGSTOP等信号及其回复信号，父进程是捕获不到的

## waitpid
> wait存在诸多限制
1. 无法指定等待的子进程，只能按顺序等待下一个子进程的终止
2. wait是阻塞的，只有错误或者有子进程终止才会返回
3. wait指针对子进程终止的情况，对于信号导致的子进程停止或者恢复的情况没法处理

### 函数定义
```
#include <sys/types.h>
#include <sys/wait.h>

pid_t waitpid(pid_t pid, int *status, int options);

```
> 这个函数的返回值和status参数与wait含义一致，这里pid参数可以控制等待的进程，而不是wait只能等待所有的子进程，oprions参数则可以控制是否阻塞，是否接收停止或恢复信号

### 参数说明
1. pid:指定等待的子进程
[1] 大于0，表示等待进程ID为pid的子进程。
[2] 等于0， 与父进程同一进程组的所有子进程
[3] 小于-1，等待组标识符与pid绝对值相等的所有子进程
[4] 等于-1，等待所有子进程，wait(&staus)等价于waitpid(-1, &status, 0)

2. options：按位或的掩码，可以包含0个或多个如下标志
[1] WUNTRACED: 除了返回终止子进程的信息外，还返回因信号停止的子进程信息
[2] WCONTIUNED: 返回收到SIGCONT的子进程信息
[3] WNOHANG: 如果pid指定的子进程没有状态变化，则立即返回，即不阻塞。在这种情况下，返回0，但是如果没有对应的pid进程，则报错，错误号ECHILD

---

## 等待状态值

---

# SIGCHILD 信号
> 由于子进程的终止属于异步的，父进程无法获知子进程何时 ***终止***，因此就需要使用wait或者waitpid查询，但是要么阻塞，要么就是轮询，浪费CPU,解决这个可以使用SIGCHILD信号处理，因为只要子进程终止，就会向父进程发送这个消息，但 默认父进程忽略此消息，因此可以使用sigaction函数对此信号处理（必须用sigaction函数，因为signal调用后就将信号置为默认方式），但是这里又出现一个问题，我们知道使用sigaction设置的信号，在信号处理期间会将引发处理的信号暂时加入到进程的阻塞信号中(除非制定了SA_NODEFER标志)，而阻塞的信号是不会排队的，即阻塞期间假如有多个子进程发送了SIGCHILD信号，那么解除阻塞后只会发送一次信号，就会导致watpid只能处理一个子进程的退出，因此正确的处理方式是，在信号处理函数中循环调用带有WNOHANG标志的函数处理,直到没有信号要处理（0），或者根本没有子进程要等待返回ECHILD错误
```
while(waitpid(-17++, &status, WNOHANG) > 0)
{
    continue;
}
```
## SIGCHILD信号特殊说明
> 当子进程收到信号停止或者恢复运行的时候，也有可能向父进程发送SIGCHILD信号，可以通过设置SA_NOCLDSTOP标志位来禁用这一功能，默认情况下是不禁用的，程序可以参照 ***sigchild_default.c***，这时候，只要子进程停止或继续，都会收到信号，只不过要看waitpid是不是处理这两种情况了。禁用请参考 ***sigchild.c***


