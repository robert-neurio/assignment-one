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

//Robert Tan
//301006212

int
main(int argc, char **argv)
{
    // Declare Varables
    int     listenfd, connfd, n;
    struct sockaddr_in servaddr;
    struct sockaddr_in client_addr;
    char    buff[MAXLINE];
    char* server_address_temp;
    char* server_port_temp;
    char server_address[MAXLINE];
    char server_port[MAXLINE];
    socklen_t clen;
    // Set default port for daytime server to 1024
    int tunnelPort = 1024;
    int daytimePortServer = 1024;
    int sockfd;
    struct sockaddr_in daytime_servaddr;
    char    recvline[MAXLINE + 1];

    //Initialize socket for the Internet
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    //If there are not enough the right number of arguments, return error
    if (argc != 2) {
        printf("usage: tunnel <Port Number>\n");
        exit(1);
    }

    // Get port number set from command line arguments
    tunnelPort = atoi(argv[1]);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // Set Tunnel's Port Number
    servaddr.sin_port = htons(tunnelPort); /* tunnel server */

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    for ( ; ; ) {
        //Listen for connections
        connfd = accept(listenfd, (struct sockaddr *) &client_addr, &clen);

        struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&client_addr;
        struct in_addr ipAddr = pV4Addr->sin_addr;

        char clientIpAddress[MAXLINE];
        inet_ntop(AF_INET, &ipAddr, clientIpAddress, MAXLINE);
        
        char host[MAXLINE];

        //Prints Sender Info when there is a connection
        getnameinfo(&client_addr, sizeof client_addr, host, sizeof host, NULL, NULL, 0);
        printf("Sender Host Name: %s \n", host);
        printf("Sender IP Address: %s \n", clientIpAddress);
        printf("Sender Port is: %d\n", (int) ntohs(client_addr.sin_port));

        //Read Server IP Address and Port from Client
        n = read(connfd, buff, MAXLINE);
        if (n < 0) {
            printf("error in reading from socket");
            exit(1);
        }

        //Seperates the server's IP Address and Port into two string pointers
        server_address_temp = strtok(buff, " ");
        server_port_temp = strtok(NULL, " ");

        //Converts String Pointers to Strings
        strcpy(server_address, server_address_temp);
        strcpy(server_port, server_port_temp);

        daytimePortServer = atoi(server_port);

        //Initialize another socket for the Internet
        if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("socket error\n");
            exit(1);
        }

        //Connect to server
        bzero(&daytime_servaddr, sizeof(daytime_servaddr));
        daytime_servaddr.sin_family = AF_INET;
        daytime_servaddr.sin_port = htons(daytimePortServer);  /* daytime server */
        if (inet_pton(AF_INET, server_address, &daytime_servaddr.sin_addr) <= 0) {
            printf("inet_pton error for %s\n", server_address);
            exit(1);
        }

        if (connect(sockfd, (struct sockaddr *) &daytime_servaddr, sizeof(daytime_servaddr)) < 0) {
            printf("connect error\n");
            exit(1);
        }

        //Reads Time from the Daytime Server
        n = read(sockfd, recvline, MAXLINE);
        if (n < 0) {
            printf("error in reading from socket \n");
            exit(1);
        }
        printf("Sending Response to Sender from Server: %s", recvline);

        //Sends Time info to the client 
        n = write(connfd, recvline, MAXLINE);
        if (n < 0) {
            printf("error in reading from socket \n");
            exit(1);
        }
        close(connfd);  
    }
}

