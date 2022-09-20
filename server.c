#include <netinet/in.h>
#include <time.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <netdb.h>

#define MAXLINE     4096    /* max text line length */
#define LISTENQ     1024    /* 2nd argument to listen() */
#define DAYTIME_PORT 3333
#define PAYLOADLINE 64

struct message{
    int addrlen, timelen, msglen;
    char addr[MAXLINE];
    char currtime[MAXLINE];
    char payload[MAXLINE];
};

int
main(int argc, char **argv)
{
    int    listenfd, connfd;
    struct sockaddr_in servaddr, psa;
    char   tbuf[MAXLINE], abuf[16], wbuf[PAYLOADLINE], pbuf[MAXLINE], mbuf[MAXLINE];
    char   peername[MAXLINE], peeraddr[INET_ADDRSTRLEN];
    char   padding[] = "     ";
    time_t ticks;
    struct sockaddr curraddr, peersock;
    socklen_t addrlen = sizeof(curraddr), peerlen = sizeof(peersock);
    struct message msg;
    FILE *fp;
    int fpc;

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

        /* obtain socket info */
        bzero(&curraddr, sizeof(curraddr));
        getsockname(connfd, (struct sockaddr *)&curraddr, &addrlen);

        /* process socket info */
        memset(&msg, 0, sizeof msg);
        printf("addresslen: %d\n", (int)addrlen);
        inet_ntop(AF_INET, &curraddr.sa_data, abuf, sizeof(abuf));
        printf("address: %s\n", abuf);

        /* fill message address */
        memcpy(msg.addr, abuf, sizeof(abuf));
        msg.addrlen = (int)addrlen;
        msg.msglen = 0;

        /* fill message time */
        ticks = time(NULL);
        snprintf(tbuf, sizeof(tbuf), "%.24s\r\n", ctime(&ticks));
        msg.timelen = (int)strlen(tbuf);
        memcpy(msg.currtime, tbuf, sizeof(tbuf));

        /* fill message payload */
        fp = popen("who", "r");
        if (fp == NULL) {
            printf("popen returned NULL\n");
            exit(1);
        }
        int line = 0;
        while (fgets(wbuf, PAYLOADLINE, fp) != NULL) {
            // printf("%s", wbuf);
            if (line >= 1) strcat(pbuf, padding);
            strcat(pbuf, wbuf);
            line++;
        }
        memcpy(msg.payload, pbuf, MAXLINE);
        printf("lines: %d\n", line);
        printf("%s", pbuf);
        printf("payload size: %ld\n", sizeof(pbuf));

        fpc = pclose(fp);
        if (fpc == -1) {
            printf("pclose failed\n");
            exit(1);
        }
        msg.msglen = sizeof(msg);

        /* pack and transmit message */
        snprintf(mbuf, MAXLINE, "IP Address: %s\nTime: %sWho: %s", msg.addr, msg.currtime, msg.payload);
        printf("%s", mbuf);
        write(connfd, mbuf, sizeof(mbuf));

        /* print peer info */
        if (getpeername(connfd, &psa, &peerlen) != 0) {
            printf("getpeername failed\n");
        }
        /*
        psa.sin_family = AF_INET;
        inet_ntop(AF_INET, &peersock, &peeraddr, INET_ADDRSTRLEN);
        int r;
        if ((r = inet_pton(AF_INET, peeraddr, &psa.sin_addr)) <= 0) {
            printf("inet_pton error: %d\n", r);
        }*/
        int s;
        if ((s = getnameinfo(&psa, sizeof(psa), peername, sizeof(peername), NULL, 0, 0)) != 0) {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
        }
        printf("Requested by address: %s\n", inet_ntoa(psa.sin_addr));
        printf("Requested by hostname: %s\n", peername);

        close(connfd);
    }
}

