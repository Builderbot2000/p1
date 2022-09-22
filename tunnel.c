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

int
main(int argc, char **argv)
{
    int    listenfd, connfd, sockfd, n;
    struct sockaddr_in servaddr;
    char   servname[INET_ADDRSTRLEN], servport[5], mbuf[MAXLINE];
    struct addrinfo hints, *servinfo, *p;

    if (argc < 2) {
        printf("usage: tunnel <Portnum>\n");
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

            /* Receive client request */
            read(connfd, servname, INET_ADDRSTRLEN);
            // printf("server name: %s\n", servname);
            read(connfd, servport, sizeof(servport));
            // printf("server port: %s\n", servport);

            /* Forward client request */
	    memset(&hints, 0, sizeof hints);
	    hints.ai_family = AF_INET;
	    hints.ai_socktype = SOCK_STREAM;

            int e;
	    if ((e = getaddrinfo(servname, servport, &hints, &servinfo)) != 0) {
	        printf("getaddrinfo error: %s\n", gai_strerror(e));
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
            // printf("successfully connected\n");

            /* Request message from server and return message to client */
	    if ((n = read(sockfd, mbuf, MAXLINE)) < 0) {
                printf("read error\n");
                exit(1);
            }
            mbuf[n] = 0;        /* null terminate */
            write(connfd, mbuf, MAXLINE);

            if ((n = read(sockfd, mbuf, MAXLINE)) < 0) {
                printf("read error\n");
                exit(1);
            }
            mbuf[n] = 0;        /* null terminate */
            write(connfd, mbuf, MAXLINE);

    }
}
