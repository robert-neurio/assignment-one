#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define MAXLINE     4096    /* max text line length */
// Tips: https://en.wikipedia.org/wiki/Getaddrinfo

int
main(int argc, char **argv)
{
    int     sockfd, n;
    char    recvline[MAXLINE + 1];
    char hostname[MAXLINE];
    char host[MAXLINE];
    struct sockaddr_in servaddr;
    int daytimePort = 1024;
    int error;
    struct addrinfo* result = NULL;
    struct addrinfo* res = NULL;

    if (argc != 3 && argc != 5) {
        printf("usage: client <Server IPaddress> <Server Port Number>\n");
        printf("usage: client <Tunnel IPaddress> <Tunnel Port Number> <Server IPaddress> <Server Port Number>\n");
        exit(1);
    }

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error\n");
        exit(1);
    }

    daytimePort = atoi(argv[2]);

    if(argc == 3){
        /* resolve the domain name into a list of addresses */
        getaddrinfo(argv[1], NULL, NULL, &result);

        /* loop over all returned results and return the first one */
        for (res = result; res != NULL; res = res->ai_next) {   
            inet_ntop (res->ai_family, res->ai_addr->sa_data, host, MAXLINE);
            void *ptr;
            switch (res->ai_family){
                case AF_INET:
                    ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
                    break;
                case AF_INET6:
                    ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
                break;
            }
            inet_ntop (res->ai_family, ptr, host, MAXLINE);

            error = getnameinfo(res->ai_addr, res->ai_addrlen, hostname, MAXLINE, NULL, 0, 0); 
            if (error != 0) {
                fprintf(stderr, "Error in getnameinfo: %s\n", gai_strerror(error));
                continue;
            }
            if (*hostname != '\0'){
                printf("Server Name: %s \n", hostname);
                printf("IP Address: %s \n", host);
                break;
            }
        }          
        //Free Memory        
        freeaddrinfo(result);

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(daytimePort);  /* daytime server */
        if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
            printf("inet_pton error for %s\n", argv[1]);
            exit(1);
        }

        if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            printf("connect error\n");
            exit(1);
        }

        while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
            recvline[n] = 0;        /* null terminate */
            if (fputs(recvline, stdout) == EOF) {
                printf("fputs error\n");
                exit(1);
            }
        }
        if (n < 0) {
            printf("read error\n");
            exit(1);
        }
    } else {

    }

    exit(0);
}



