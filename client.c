#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>

#define MAXLINE     4096    /* max text line length */
#define DAYTIME_PORT 3333

int
main(int argc, char **argv)
{
    int     sockfd, n;
    char    recvline[MAXLINE + 1], hostname[MAXLINE];
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in sa;

    if (argc != 3 && argc != 5) {
        printf("usage: client <Optional: Tunnel IP/Name> <Optional: Tunnel Port> <Server IP/Name> <Server Port>\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int e;
    if ((e = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        printf("getaddrinfo error: %s\n", gai_strerror(e));
        exit(1);
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket creation failed");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect failed");
            continue;
        }

        break;
    }

    if (p == NULL) {
        printf("client: failed to connect\n");
        exit(1);
    }

    /* send server info to tunnel */
    if (argc == 5) {
        printf("Sent: %s\n", argv[3]);
        if (write(sockfd, argv[3], INET_ADDRSTRLEN) == -1) {
            perror("First Write Error: ");
        }
        printf("Sent: %s\n", argv[4]);
        if (write(sockfd, argv[4], sizeof(argv[2])) == -1) {
            perror("Second Write Error: ");
        }
    }

    /* get hostname of server */
    sa.sin_family = AF_INET;
    sa.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &sa.sin_addr) <= 0) {
        printf("inet_pton error for %s\n", argv[1]);
        exit(1);
    }
    int s;
    if ((s = getnameinfo(&sa, sizeof(sa), hostname, sizeof(hostname), NULL, 0, 0)) != 0) {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
    }
    printf("Server Name: %s\n", hostname);

    freeaddrinfo(servinfo);

    if ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;        /* null terminate */
        if (fputs(recvline, stdout) == EOF) {
            printf("fputs error\n");
            exit(1);
        }
    }
    if (n < 0) {
        perror("read error");
        exit(1);
    }

    exit(0);
}

