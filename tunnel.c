#include <netinet/in.h>
#include <time.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE     4096    /* max text line length */
#define LISTENQ     1024    /* 2nd argument to listen() */

#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <linux/if_link.h>

int
main(int argc, char **argv)
{
    // Declare Varables
    int     listenfd, connfd, n;
    struct sockaddr_in servaddr;
    struct sockaddr_in client_addr;
    char    buff[MAXLINE];
    char* server_address;
    char* server_port;
    socklen_t clen;
    // Set default port for daytime server to 1024
    int daytimePort = 1024;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (argc != 2) {
        printf("usage: tunnel <Port Number>\n");
        exit(1);
    }

    // Get port number set from command line arguments
    daytimePort = atoi(argv[1]);

    //printf("Dayime Port Number is: %d \n", daytimePort);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // Set Daytime Server's Port Number
    servaddr.sin_port = htons(daytimePort); /* daytime server */

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    for ( ; ; ) {
        connfd = accept(listenfd, (struct sockaddr *) &client_addr, &clen);

        struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&client_addr;
        struct in_addr ipAddr = pV4Addr->sin_addr;

        char clientIpAddress[MAXLINE];
        inet_ntop(AF_INET, &ipAddr, clientIpAddress, MAXLINE);
        
        char host[MAXLINE];

        getnameinfo(&client_addr, sizeof client_addr, host, sizeof host, NULL, NULL, 0);
        printf("Sender Host Name: %s \n", host);
        printf("Sender IP Address: %s \n", clientIpAddress);
        printf("Sender Port is: %d\n", (int) ntohs(client_addr.sin_port));

        n = read(connfd, buff, MAXLINE);
        if (n < 0) {
            printf("error in reading from socket");
            exit(1);
        }

        printf("%s \n", buff);

        server_address = strtok(buff, " ");
        server_port = strtok(NULL, " ");

        printf("%s \n", server_address);
        printf("%s \n", server_port);

        n = write(connfd, buff, MAXLINE);
        if (n < 0) {
            printf("error in reading from socket");
            exit(1);
        }

        printf("Sending response: %s \n", buff);

        close(connfd);

        
    }
}

