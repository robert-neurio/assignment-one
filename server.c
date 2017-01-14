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
    int     listenfd, connfd;
    struct sockaddr_in servaddr;
    struct sockaddr_in client_addr;
    char    buff[MAXLINE];
    time_t ticks;
    socklen_t clen;
    // Set default port for daytime server to 1024
    int daytimePort = 1024;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (argc != 2) {
        printf("usage: server <Port Number>\n");
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
        
        printf("Client IP Address: %s \n", clientIpAddress);

        /*struct hostent *he;
        struct in_addr ipv4addr;
        inet_pton(AF_INET, clientIpAddress, &ipv4addr);
        he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
        printf("Host name: %s\n", he->h_name);*/

        char host[MAXLINE];

        getnameinfo(&client_addr, sizeof client_addr, host, sizeof host, NULL, NULL, 0);
        printf("Client Host Name: %s \n", host);

        printf("Port is: %d\n", (int) ntohs(client_addr.sin_port));

        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "Time: %.24s\r\n", ctime(&ticks));
        write(connfd, buff, strlen(buff));
        printf("Sending response: %s", buff);

        close(connfd);
    }
}

