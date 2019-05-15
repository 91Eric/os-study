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
```


```
> data是用户自定义数据，在使用epoll_ctl时使用data中哪个成员，那么在epoll_wait处理的时候就使用哪个成员做判断，常用的就是fd变量

#### 返回值
>成功返回0，失败返回-1，并设置errno

##### 注
> 一般EPOLL_CTL_ADD 和 EPOLL_CTL_DEL是需要成对使用的，在退出的时候都要先EPOLL_CTL_DEL，再close文件描述符

---

### epoll_wait

#### 函数定义
```

 #include <sys/epoll.h>

       int epoll_wait(int epfd, struct epoll_event *events,int maxevents, int timeout);
       int epoll_pwait(int epfd, struct epoll_event *events,int maxevents, int timeout,
        const sigset_t *sigmask);

```

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




