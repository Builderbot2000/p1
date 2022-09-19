#include <netinet/in.h>
#include <time.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <stdlib.h>

#define MAXLINE     4096    /* max text line length */
#define LISTENQ     1024    /* 2nd argument to listen() */
#define DAYTIME_PORT 3333

struct message{
    int addrlen, timelen, msglen;
    char addr[MAXLINE];
    char currtime[MAXLINE];
    char payload[MAXLINE];
};

int
main(int argc, char **argv)
{
    int     listenfd, connfd;
    struct sockaddr_in servaddr;
    char    buff[MAXLINE];
    time_t ticks;

    if (argc != 2) {
        printf("usage: server <Portnum>\n");
        exit(1);
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1])); /* daytime server */

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    for ( ; ; ) {
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);

        struct sockaddr curraddr;
        bzero(&curraddr, sizeof(curraddr));
        socklen_t addrlen = sizeof(curraddr);
        struct message msg;

        /* obtain socket info */
        getsockname(connfd, (struct sockaddr *)&curraddr, &addrlen);

        /* process socket info */
        memset(&msg, 0, sizeof msg);
        printf("addresslen: %d\n", (int)addrlen);
        char buf[16];
        inet_ntop(AF_INET, &curraddr.sa_data, buf, sizeof(buf));
        printf("address: %s\n", buf);

        /* assemble message */
        memcpy(msg.addr, buf, sizeof(buf));
        msg.addrlen = (int)addrlen;
        msg.msglen = 0;

        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        msg.timelen = sizeof(buff);
        memcpy(msg.currtime, buff, sizeof(buff));

        write(connfd, buff, strlen(buff));
        printf("Sending response: %s", buff);

        close(connfd);
    }
}

