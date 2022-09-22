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
    int    tnameflag = 1, snameflag = 1;  // name flag: indicates if input name argument is a hostname or ip
    char   recvline[MAXLINE + 1], servname[MAXLINE], tunname[MAXLINE];
    char   *tempaddr, *sname, *sport, *tname, *tport;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in sa, *ad;

    if (argc != 3 && argc != 5) {
        printf("usage: client optional:<tunnel_ip/name> optional<tunnel_port> <server_ip/name> <server_port>\n");
        exit(1);
    }

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

    /* Set name flags and arg references */
    struct sockaddr_in ipv4test;
    snameflag = inet_pton(AF_INET, sname, &(sa.sin_addr));
    memset(&ipv4test, 0, sizeof ipv4test);
    if (argc == 5) tnameflag = inet_pton(AF_INET, tname, &(sa.sin_addr));

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

    if (snameflag == 1) {
        sa.sin_family = AF_INET;
        sa.sin_port = htons(atoi(sport));
        if (inet_pton(AF_INET, sname, &sa.sin_addr) <= 0) {
            printf("inet_pton failed for %s\n", sname);
        }
        if ((e = getnameinfo(&sa, sizeof(sa), servname, sizeof(servname), NULL, 0, 0)) != 0) {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(e));
        }
    }
    else {
        memcpy(servname, sname, strlen(sname));
    }

    if ((e = getaddrinfo(sname, sport, &hints, &servinfo)) != 0) {
        printf("getaddrinfo error: %s\n", gai_strerror(e));
    }
    ad = (struct sockaddr_in *)servinfo->ai_addr;
    tempaddr = inet_ntoa(ad->sin_addr);

    /* send server info to tunnel */
    if (argc == 5) {
        // printf("Sent: %s\n", tempaddr);
        if (write(sockfd, tempaddr, INET_ADDRSTRLEN) == -1) {
            perror("Server name write error: ");
        }
        // printf("Sent: %s\n", sport);
        if (write(sockfd, sport, sizeof(sport)) == -1) {
            perror("server port write error: ");
        }
    }

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
        if (tnameflag == 1) {
            sa.sin_family = AF_INET;
            sa.sin_port = htons(atoi(tport));
            if (inet_pton(AF_INET, tname, &sa.sin_addr) <= 0) {
                printf("inet_pton failed for %s\n", tname);
            }
            if ((e = getnameinfo(&sa, sizeof(sa), tunname, sizeof(tunname), NULL, 0, 0)) != 0) {
                fprintf(stderr, "getnameinfo: %s\n", gai_strerror(e));
            }
        }
        else {
            memcpy(tunname, tname, strlen(tname));
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

