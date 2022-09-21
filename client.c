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
    int    sockfd, n, e;
    char   recvline[MAXLINE + 1], servname[MAXLINE], tunname[MAXLINE];
    char   *tempaddr, *sname, *sport, *tname, *tport;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in sa, *ad;

    if (argc != 3 && argc != 5) {
        printf("usage: client <Optional: Tunnel IP/Name> <Optional: Tunnel Port> <Server IP/Name> <Server Port>\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

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

    freeaddrinfo(servinfo);

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

    if (argc == 3) {
        sname = argv[1];
        sport = argv[2];
    }

    if (argc == 5) {
        sname = argv[3];
        sport = argv[4];
        tname = argv[1];
        tport = argv[2];
    }

    sa.sin_family = AF_INET;
    sa.sin_port = htons(atoi(sport));
    if (inet_pton(AF_INET, sname, &sa.sin_addr) <= 0) {
        printf("inet_pton error for %s\n", sname);
        exit(1);
    }
    if ((e = getnameinfo(&sa, sizeof(sa), servname, sizeof(servname), NULL, 0, 0)) != 0) {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(e));
    }

    if ((e = getaddrinfo(sname, sport, &hints, &servinfo)) != 0) {
        printf("getaddrinfo error: %s\n", gai_strerror(e));
    }
    ad = (struct sockaddr_in *)servinfo->ai_addr;
    tempaddr = inet_ntoa(ad->sin_addr);

    printf("Server Name: %s\n", servname);
    printf("IP Address: %s\n", tempaddr);

    freeaddrinfo(servinfo);

    if ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;        /* null terminate */
        if (fputs(recvline, stdout) == EOF) {
            printf("fputs error\n");
            exit(1);
        }
    }

    if (argc == 5) {
        sa.sin_family = AF_INET;
        sa.sin_port = htons(atoi(tport));
        if (inet_pton(AF_INET, tname, &sa.sin_addr) <= 0) {
            printf("inet_pton error for %s\n", tname);
            exit(1);
        }
        if ((e = getnameinfo(&sa, sizeof(sa), tunname, sizeof(tunname), NULL, 0, 0)) != 0) {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(e));
        }

        if ((e = getaddrinfo(tname, tport, &hints, &servinfo)) != 0) {
            printf("getaddrinfo error: %s\n", gai_strerror(e));
        }
        ad = (struct sockaddr_in *)servinfo->ai_addr;
        tempaddr = inet_ntoa(ad->sin_addr);

        printf("\nVia Tunnel: %s\n", tunname);
        printf("IP Address: %s\n", tempaddr);
        printf("Port Number: %s\n\n", tport);
    }

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

