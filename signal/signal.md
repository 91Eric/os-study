# 信号的一些使用

## 一些常用的信号情况
### SIGTERM
> 这是标准的终止进程的信号，由kill或killall命令产生，我们常用的KILL -SIGKILL 方式是<b>错误的方式</b>，后者应用程序无法对其进行任何处理(无法捕获信号并处理)，因此程序会被强制停止，而前者是可以捕获并处理的，这允许我们对程序进行一些清理工作，例如关闭一些文件，子进程等
### SIGINT
> 使用ctrl+c产生的信号，默认行为是退出进程



## 注册信号处理函数

### signal
> 这种方式已经不推荐使用，这个系统调用不具有可移植性，通常只有用于将某些信号的处理方式设置为SIG_IGN或SIG_DFL时，才使用这个函数，否则， ***如果使用自定义的处理函数时，第一次响应之后系统会默认将对应的信号处理方式设置为默认的方式***，在 POSIX.1中解决了这个问题，使用signaction函数代替signal 函数（建议一律使用signaction），详见后面的signaction函数部分

#### 函数定义
```
(void)(*signal(int sig, void (*handler)(int)))(int)

简化后

typedef void (*sighanlder_t)(int)

sighanlder_t signal(int sig, hanlder_t hanlder)
```
#### 参数说明
1. sig: 就是要处理的信号，使用SIGXX的符号，因为不同的系统可能同一个SIGXX对应的数字不一致，因此避免直接使用数字
2. hanlder: 信号处理函数，有三种方式:

[1] SIG_DFL:　采用默认方式，用于还原原先对信号的设置

[2] SIG_IGN: 忽略信号（后面有阻塞信号，两者不一样）

[3] 自定义处理函数，形如
```
void sig_hanlder(int sig_num)
{

}
```
#### 返回值

> 可能是原先的处理函数地址，可能是SIG_DFL,SIG_IGN之一，如果调用失败，返回SIG_ERR
```
#define SIG_ERR	((__sighandler_t) -1)		/* Error return.  */
#define SIG_DFL	((__sighandler_t) 0)		/* Default action.  */
#define SIG_IGN	((__sighandler_t) 1)
```

### signaction
> signaction函数提供了比signal更灵活的方式，可以在不改变原有信号处理的情况下获取到信号的处理方式，并且可以对信号处理设置各种属性进行更加精确的控制

#### 函数定义
```
#include <signal.h>

int sigaction(int signum, const struct sigaction *act,
            struct sigaction *oldact);
```
#### 参数
1. signum: 想要处理的信号，SIGXX
2. act: 用于设置当前信号的处理设置，其定义如下
```
struct sigaction {
    void     (*sa_handler)(int);
    void     (*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t   sa_mask;
    int        sa_flags;
    void     (*sa_restorer)(void);
};

```
3. olsact: 原先的信号处理设置
##### struct sigaction说明
> sa_handler 和 sa_sigaction 分别针对于不同的信号响应方式，前置与signal
响应函数一样，是sigaction默认的处理函数方式，而后者可以携带更多的信息给信号处理函数，要启用此形式的处理函数，sa_flags 需要增加SA_SIGINFO标志，<b>对于标准信号和实时信号都可以使用这两种方式，只看是否设置SA_SIGINFO标志</b>

> sa_mask 与 sigprocmask 函数一样的结构体，初始化，增加或删除，查询等都与sigprocmask函数一致，具体功能参看 ***sigprocmask 与sigaction 函数对于掩码设置的区别*** 

> sa_flags: 用于设置一些行为，比如SA_NODEFER,SA_RESETHAND,SA_SIGINFO


#### 注
> 要使用此函数，必须启用POSIX.1特性，有如下几种方式

１. 在对应的源文件任何#命令之前使用#define _POSIX_SOURCE 启用这个特性

２. 在对应的源文件任何#命令之前使用#define _POSIX_C_SOURCE 启用这个特性,这个宏需要定义几个特定的值用于支持不同的POSIX.X特性，例如POSIX.1对应199506L，这些值其实就是对应的版本成为标准的时间。
示例程序如下
```
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199506L
#endif

或

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif

＃include <stdio.h>


...
```
３. 在gcc　编译参数中使用-D的方式引入上述宏定义，示例程序如下:
```
gcc -D_POSIX_C_SOURCE=199506L

gcc -D_POSIX_SOURCE
```

> 示例代码见sigaction.c 

### sigprocmask
> 这个函数用于设置信号掩码，此功能也可以使用sigaction实现，这个函数的使用方式如下

```
sigset_t set, oldset;
sigemptyset(&set);   // 必须使用这种方式的初始化，不能使用memset等方式，因为sigset_t的实现有差异
sigaddset(&set, SIGXX);
sigprocmask(SIG_BLOCK, &set, &oldset);

do something 

sigprocmask(SIG_SETMASK, &oldset, NULL);   // 恢复原先的设置

示例程序见sigmask.c 程序

```
#### sigprocmask 与sigaction 函数对于掩码设置的区别
1. sigprocmask 一旦设置，除非再次调用sigprocmask 将信号解除，否则会一直阻塞信号
2. sigaction 中的sa_mask 字段的作用其实是为了防止在信号处理函数执行时被某些信号打断，因此会在信号处理函数调用前将这些信号临时加入阻塞信号中，调用完成后会恢复原样
3. sigaction函数默认会将捕获的信号同时加入阻塞信号集中，这是为了防止在信号处理函数执行过程中，同样信号到来导致不断递归中断自己（假如在执行，又来了同样信号，就会中断现在的处理函数再调用处理函数），可以设置sa_flags标志包含SA_NODEFE标志位禁用此默认行为
---

## 信号的发送
>　信号的发送可以是一个进程向另一个进程发送信号，也可是自身向自身发送
### kill
> kill函数用于一个进程向另一个进程或进程组发送信号

#### 函数定义
```
int kill(int pid, int sig)
```
#### 参数

1. pid: 一个或多个目标进程，具体由pid不同参数指定
2. sig:　要发送的信号，即SIGXX，有一种特殊情况，当sig为０时，即空信号可以用于检测对应进程是否存在

#### 返回值
> 成功返回０，失败返回-1, errno 被设置为ESRCH(查无此进程)、EPERM(没有权限)或EINVAL(指定的信号不合法)

### raise
> 这个函数用于向自身发送信号

### sigqueue
> 这个函数主要是为了实时信号提出的（也适用标准信号），配合sigaction函数使用，通过sa_flags 是否设置SA_SIGINFO标志位决定使用哪种处理函数

## 信号的同步生成和异步生成
> 进程何时接收信号一般是无法确定的，为了确定两者之间的时机，需要了解信号的同步生成和异步生成

### 异步生成
> 当一个进程向另一个进程发送信号时，这就是异步生成，此时信号的生成与接收的时机是不确定的。

### 同步生成
> 如果信号是由于进程本身执行造成的，即是自身产生的，就是同步生成，此时信号是立即传送的

1. 执行某些机器语言指令产生了硬件错误，导致对应的信号产生硬件信号
2. 进程使用raise或kill 等向自身发送信号

### 注
> 异步还是同步是对信号产生方式的讨论，和信号本身无关，任何信号都可以是这两种信号，看使用方式而已

--- 

## 信号传递的时机和顺序
### 传递时机
> 对于同步信号，会立即传递，但是异步信号，即使没有阻塞该信号，在信号产生与实际传递到进程之间依旧会产生一段（瞬时）延时，在此期间，信号处于<b>等待状态</b>，这是因为信号的传递是需要一定时机的：进程正在执行，且发生由内核态到用户态的下一次切换时，即只有如下情况信号才会被传递
1. 进程在前一调度超时后，再次获得调度（在一个时间片的开始处）
2. 系统调用完成时

### 传递顺序
> 当解除对多个信号的阻塞时，一般可以理解为按照信号值的 ***升序*** 来调用，但是不能依赖这一约定，因为标准中没有对标准信号的传递顺序做强制规定