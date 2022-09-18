#include <netinet/in.h>
#include <time.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>

#define MAXLINE     4096    /* max text line length */
#define LISTENQ     1024    /* 2nd argument to listen() */
#define DAYTIME_PORT 3333

struct message{
    int addrlen, timelen, msglen;
    char addr[MAXLINE];
    char currtime[MAXLINE];
    char payload[MAXLINE];
};

int main(int argc, char **argv)
{
    int     listenfd, connfd;
    struct sockaddr_in servaddr;
    struct sockaddr *curraddr;
    socklen_t *addrlen = 0;
    char    buff[MAXLINE];
    time_t ticks;
    struct message msg;

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

    if (listen(listenfd, LISTENQ) == -1) {
        printf("Failed to establish listening socket\n");
        perror("listen");
    }

    for ( ; ; ) {
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);

        ticks = time(NULL);
        
        /* get current ip address */
        bzero(&curraddr, sizeof(curraddr));
        getsockname(connfd, curraddr, addrlen);

        /* set message */
        memset(&msg, 0, sizeof msg);
        long len = (long) addrlen;
        msg.addrlen = (int) len;
        printf("addresslen: %ld\n", len);
        msg.timelen = 0;
        msg.msglen = 0;
        memcpy(msg.addr, curraddr->sa_data, sizeof(curraddr->sa_data));

        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        write(connfd, buff, strlen(buff));
        printf("Sending response: %s", buff);

        close(connfd);
    }
}

