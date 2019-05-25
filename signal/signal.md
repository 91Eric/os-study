# 信号的一些使用

## 一些常用的信号情况
### SIGTERM
> 这是标准的终止进程的信号，由kill或killall命令产生，我们常用的KILL -SIGKILL 方式是<b>错误的方式</b>，后者应用程序无法对其进行任何处理，因此程序会被强制停止，而前者是可以捕获并处理的，这允许我们对程序进行一些清理工作，例如关闭一些文件，子进程等


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

### sigprocmask
> 这个函数用于设置信号掩码，此功能也可以使用sigaction实现，这个函数的使用方式如下

```
sigset_t set, oldset;
sigemptyset(&set);   // 必须使用这种方式的初始化，不能使用memset等方式，因为sigset_t的实现有差异
sigaddset(&set, SIGXX);
sigprocmask(SIG_BLOCK, &set, &oldset);

do something 

sigprocmask(SIG_SETMASK, &oldset, NULL);   // 恢复原先的设置

```
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
> 这个函数用于向自身发送信号，

