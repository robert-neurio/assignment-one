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

//Robert Tan
//301006212

int
main(int argc, char **argv)
{
    //Declare Variables
    int     sockfd, n;
    char    recvline[MAXLINE + 1];
    char hostname[MAXLINE];
    char host[MAXLINE];
    struct sockaddr_in servaddr;
    int daytimePort = 1024;
    int error;
    struct addrinfo* result = NULL;
    struct addrinfo* res = NULL;
    char servername[MAXLINE];
    char serverIPaddress[MAXLINE];
    char tunnelname[MAXLINE];
    char tunnelIPaddress[MAXLINE];
    char message[MAXLINE];

    //If there are not enough the right number of arguments, return error
    if (argc != 3 && argc != 5) {
        printf("usage: client <Server IPaddress> <Server Port Number>\n");
        printf("usage: client <Tunnel IPaddress> <Tunnel Port Number> <Server IPaddress> <Server Port Number>\n");
        exit(1);
    }

    //Initialize socket for the Internet
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error\n");
        exit(1);
    }

    //Get port of the first server
    daytimePort = atoi(argv[2]);

    //If user only gives server name and port
    if(argc == 3){
        /* resolve the domain name into a list of addresses */
        getaddrinfo(argv[1], NULL, NULL, &result);

        /* loop over all returned results and return the last one in variables hostname and host*/
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
                strcpy(servername, hostname);
                strcpy(serverIPaddress, host);
            }
        }
        //Print info about the server
        printf("Server Name: %s \n", servername);
        printf("IP Address: %s \n", serverIPaddress);

        //Free Memory        
        freeaddrinfo(result);

        //Connect to server
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

        //Read time from server and prints it
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

    //If user gives tunnel and server arguments    
    } else {
        //Connect to the tunnel
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

        //Takes the 3rd and 4th arguments and concatenates them into a string, 
        //seperating them with a space
        bzero(message, MAXLINE);

        strcpy(message, argv[3]);
        strcat(message, " ");
        strcat(message, argv[4]);

        //Pass Server info to the Tunnel
        if (write(sockfd, message, strlen(message)) < 0)
            printf("error in writing on stream socket\n");

        //Read Time given from the Tunnel and saves it in the recvline variables
        n = read(sockfd, recvline, MAXLINE);
        if (n < 0) {
            printf("error in reading from socket");
            exit(1);
        }

        /* resolve the server domain name into a list of addresses */
        getaddrinfo(argv[3], NULL, NULL, &result);

        /* loop over all returned results and return the last one in variables hostname and host */
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
                strcpy(servername, hostname);
                strcpy(serverIPaddress, host);
            }
        }
        //Prints Server Info
        printf("Server Name: %s \n", servername);
        printf("IP Address: %s \n", serverIPaddress);
        //Prints the Time returned from the Tunnel
        printf("%s \n", recvline);

        /* resolve the tunnel domain name into a list of addresses */
        getaddrinfo(argv[1], NULL, NULL, &result);

        /* loop over all returned results and return the last one */
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
                strcpy(tunnelname, hostname);
                strcpy(tunnelIPaddress, host);
            }
        }
        //Prints Tunnel Info
        printf("Via Tunnel: %s \n", tunnelname);
        printf("IP Address: %s \n", tunnelIPaddress);
        printf("Port Number: %s \n", argv[2]);

        close(sockfd);
    }
    exit(0);
}
