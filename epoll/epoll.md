# epoll 学习笔记

> 记录学习epoll过程中的一些知识

## epoll 的使用

> 使用epoll只需要简单的三步就能实现对不同句柄的特定事件的监听
### epoll_create

#### 函数定义

```
#include <sys/epoll.h>

int epoll_create(int size)
int epoll_create1(int flags)

```
> 这个就像open函数一样，返回一个句柄，这个句柄用于后续所有epoll_xxx函数的第一个参数，从 **2.6.8** 以后 szie参数已经无用
#### 返回值
> 成功返回非负的文件描述符，失败返回-1， 并且errno被设置为确定的值（好多函数失败返回都是-1，比如socket系列函数）
---
### epoll_ctl

#### 函数定义
```
#include <sys/epoll.h>

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

```
> 这个函数用于对文件描述符fd执行某种操作（op）,参数由event指定

#### 参数
1. epfd: epoll_create函数返回的文件描述符
2. op: 执行的操作，有一下几种：

```
 EPOLL_CTL_ADD: 将fd添加到epfd
 EPOLL_CTL_MOD: 修改fd对应的参数，由event指定
 EPOLL_CTL_DEL: 将fd从epfd删除
```
3. fd: 就是要监听的字符，注意这个fd不能是普通的文件
4. event： fd对应的参数，这个参数指定了fd本身对什么事件感兴趣以及一些其他参数，定义如下

```
typedef union epoll_data
{
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;

struct epoll_event
{
  uint32_t events;	/* Epoll events */
  epoll_data_t data;	/* User data variable */
} __EPOLL_PACKED;

```
> 其中event用于指定感兴趣的事件，主要有以下几种
1. EPOLLIN: 可读取事件
2. EPOLLOUT:　普通数据可写
3. EPOLLRDHUP: 套接字对端关闭（自2.6.17，对端调用close，特别适用于检查对端是否关闭）
4. EPOLLERR: 有错误发生
5. EPOLLHUP: 出现挂断
6. EPOLLONESHOT: 事件只接收一次通知，第一次通知后就会将对应的文件描述符永久的置为非激活状态，不会再有新通知到来
> 将一个不可能触发该事件发生发生的套接字加入epoll，将会导致hup事件的上报,例如socket还没有connect或listen就加入epoll事件句柄
> data是用户自定义数据，在使用epoll_ctl时使用data中哪个成员，那么在epoll_wait处理的时候就使用哪个成员做判断，常用的就是fd变量



#### 返回值
>成功返回0，失败返回-1，并设置errno

##### 注
> 一般EPOLL_CTL_ADD 和 EPOLL_CTL_DEL是需要成对使用的，**在退出的时候都要先EPOLL_CTL_DEL，再close文件描述符,如果顺序反了，EPOLL_CTL_DEL会报错**

> EPOLLOUT(写)监听的使用场，一般说明主要有以下三种使用场景:
1. 对客户端socket只使用EPOLLIN(读)监听，不监听EPOLLOUT(写)，写操作一般使用socket的send操作
2. 客户端的socket初始化为EPOLLIN(读)监听，有数据需要发送时，对客户端的socket修改为EPOLLOUT(写)操作，这时EPOLL机制会回调发送数据的函数，发送完数据之后，再将客户端的socket修改为EPOLL(读)监听
3. 对客户端socket使用EPOLLIN 和 EPOLLOUT两种操作，这样每一轮epoll_wait循环都会回调读，写函数，这种方式效率不是很好

---

### epoll_wait

#### 函数定义
```

 #include <sys/epoll.h>

       int epoll_wait(int epfd, struct epoll_event *events,int maxevents, int timeout);
       int epoll_pwait(int epfd, struct epoll_event *events,int maxevents, int timeout,
        const sigset_t *sigmask);

```
> 这个函数用于监听使用epoll_ctl添加到由epoll_create创建的文件描述符中的文件描述符，所有就绪的文件描述符存放于events

#### 参数
1. epfd: epoll_create函数返回的文件描述符
2. events: 用于存放返回就绪事件的数组
3. maxevents: 关注的最多事件数，一般就是events数组的大小
4. timeout: 超时时间，如果为-1，一直阻塞直到有至少一个事件就绪，如果为０，则执行一次非阻塞式的检查，看哪个事件就绪，如果大于０，则至多阻塞timeout秒，直到有事件就绪或捕捉到一个信号(比如SIGINT等信号)

#### 返回值
>成功返回0，失败返回-1，并设置errno

### 一个简单的例子
```
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_BUF 128
#define MAX_FILES 5

int main(int argc, char const *argv[])
{
    int sfd, epfd, nfds;
    struct epoll_event ev, events[MAX_FILES];
    struct sockaddr_in addr;
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
    {
        perror("socket create failed\n");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(65535);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind failed\n");
        close(sfd);
        return -1;
    }

    if (listen(sfd, 10) == -1)
    {
        perror("listen failed\n");
        close(sfd);
        return -1;
    }

    epfd = epoll_create(MAX_FILES);
    if (epfd == -1)
    {
        printf("epoll_create failed\n");
        return -2;
    }

    ev.events = EPOLLIN;
    ev.data.fd = sfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev) == -1)
    {
        perror("epoll_ctl failed, err is:");
        return -2;
    }

    while(1)
    {
        nfds = epoll_wait(epfd, events, MAX_FILES, 20);
        if (nfds == -1)
        {
            if (errno == EINTR)
            {
                printf("wait continue\n");
                continue;
            }
            printf("wait error\n");
            goto END;
        }
        else if (nfds == 0)
        {
            // printf("wait timeout\n");
        }
        else
        {
            for (int i = 0; i < nfds; i++)
            {
                if (events[i].data.fd == sfd)
                {
                    struct sockaddr_in c_addr;
                    socklen_t len = sizeof(struct sockaddr);
                    int cfd = accept(sfd, (struct sockaddr*)&c_addr, &len);
                    if (cfd == -1)
                    {
                        perror("accpet failed");
                    }
                    ev.events = EPOLLIN | EPOLLRDHUP;
                    ev.data.fd = cfd;
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev) == -1)
                    {
                        perror("add cfd to epoll failed\n");
                        goto END;
                    }
                }
                else
                {
                    char buf[MAX_BUF] = {0};
                    if (events[i].events & EPOLLIN && !(events[i].events & EPOLLRDHUP))
                    {
                        
                        ssize_t cnt = recv(events[i].data.fd, buf, MAX_BUF - 1, 0);
                        if (cnt < 0)
                        {
                            perror("recv data failed\n");
                            close(events[i].data.fd);
                            if (epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &ev) == -1)
                            {
                                perror("delete fd failed\n");
                            }
                        }
                        else
                        {
                            printf("recv date is %s, len is %d\n", buf, cnt);
                        }
                    }
                    else
                    {
                        perror("unkown cmd\n");
                        printf("cmd is %x\n", events[i].events);
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &ev) == -1)
                        {
                            perror("delete fd failed\n");
                        }
                        close(events[i].data.fd);
                    }
                }
            }
        }
    }

END:
    close(sfd);
    close(epfd);

    return 0;
}


```




