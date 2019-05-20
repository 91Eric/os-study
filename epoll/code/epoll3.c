/*
 * 此程序验证SIGPIPE问题
*/

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif

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
#include <signal.h>

#define MAX_BUF 128
#define MAX_FILES 5
#define MAX_FD_NUM 4

char* sig_table[32] = {"SIGHUP", "SIGINT", "SIGQUIT"};
int g_fd_table[MAX_FD_NUM] = {0,};
int g_fd_num = 0;

static void sigint_handler(int signum);

static void close_fd()
{
    for (int i = 1; i < g_fd_num; i++)
    {
        if (g_fd_table[i] > 0)
        {
            epoll_ctl(g_fd_table[0], EPOLL_CTL_DEL, g_fd_table[i], NULL);
            close(g_fd_table[i]);
        }
        close(g_fd_table[0]);
    }
}

static void signal_init()
{
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
}

static void sigint_handler(int signum)
{
    printf("sig %s\n", sig_table[signum - 1]);
    close_fd();
    printf("sig deal ok\n");
    exit(-1);
}

int main(int argc, char const *argv[])
{
    int sfd, epfd, nfds;

    struct epoll_event ev, events[MAX_FILES];
    struct sockaddr_in addr;
    sig_table[12] = "SIGPIPE";
    signal_init();
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
        close(sfd);
        close(epfd);
        return -2;
    }

    g_fd_table[g_fd_num++] = epfd;
    g_fd_table[g_fd_num++] = sfd;
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
                perror("wait continue\n");
                continue;
            }
            perror("wait error\n");
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
                    if (g_fd_num < MAX_FD_NUM)
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
                        g_fd_table[g_fd_num++] = cfd;
                    }
                    else
                    {
                        struct sockaddr_in c_addr;
                        socklen_t len = sizeof(struct sockaddr);
                        int cfd = accept(sfd, (struct sockaddr*)&c_addr, &len);
                        close(cfd);
                        printf("max acceppt \n");
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
                            if (send(events[i].data.fd, buf, cnt, 0) == -1)
                            {
                                perror("send failed");
                                close(events[i].data.fd);
                            }
                        }
                    }
                    else if (events[i].events & (EPOLLIN | EPOLLRDHUP))
                    {
                        printf("client close cmd is %x\n", events[i].events);
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &ev) == -1)
                        {
                            perror("delete fd failed\n");
                        }
                        if (send(events[i].data.fd, "ok", 2 , 0) > 0)
                        {
                            if (send(events[i].data.fd, "ok", 2 , 0) < 0)
                            if (errno == EPIPE)
                            {
                                perror("CLIENT CLOSED\n");
                            }
                        }
                        close(events[i].data.fd);
                    }
                    else
                    {
                        perror("unkown cmd\n");
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
    close_fd();

    return 0;
}
