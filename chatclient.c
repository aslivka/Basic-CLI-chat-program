#define _POSIX_C_SOURCE 200112L

/******************************************************************
**  Author: Artem Slivka
**  Date: 10/23/18
**  Description: Project1 - chatclient.c
**  Implementation of client chat program using sockets.
**  To run ./chatclient <server-hostname> <port#>
******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAXDATASIZE 600 // max number of bytes we can recv/send at once 
#define MAX_HANDLE_LEN 10

/* Function prototypes */
void checkSaveArguments(int numArgs, char* hostArg, char* portArg, char* port, char* hostName);
int connectToServer(char *hostName, char *port, int *sockfd, char address[], int addressSize);
void saveUserHandle(char* handle);
void startSession(int sockfd, char* handle);
int sendMessage(char* message, char* clientHandle, int socket);
int receiveMessage(char* message, char* clientHandle, int socket);


int main(int argc, char *argv[]){
    int sockfd, result;
    char address[INET6_ADDRSTRLEN];
    char port[6];
    char hostName[INET6_ADDRSTRLEN];
    char clientHandle[MAX_HANDLE_LEN + 1];

    /* Check and save input arguments */
    checkSaveArguments(argc, argv[1], argv[2], port, hostName);

    result =  connectToServer(hostName, port, &sockfd, address, sizeof(address));
    if(result == 0){
         /* Starting session with server */
        startSession(sockfd, clientHandle);
    }

    return 0;
}

/* Returns sockaddr struct, IPv4 or IPv6 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET){  return &(((struct sockaddr_in*)sa)->sin_addr);}
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* Creates network address, client socket, and connects to server socket 
   Returns 0 if connection was successful, other values if not.
   Reference: https://beej.us/guide/bgnet/html/single/bgnet.html 
*/
int connectToServer(char *hostName, char *port, int *sockfd, char address[], int addressSize){
    int rv;
    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    /* Creating address */
    if((rv = getaddrinfo(hostName, port, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next){
        /* Creating socket */
        if( (*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        /* Connecting to socket */
        if(connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(*sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if(p == NULL){
        fprintf(stderr, "client: failed to connect\n");
        return -1;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), address, addressSize);
    printf("client: connected to %s\n", address);
    freeaddrinfo(servinfo); /* all done with this structure */

    return 0;
}

/* Function checks and saves hostname/port arguments */
void checkSaveArguments(int numArgs, char* hostArg, char* portArg, char* port, char* hostName){
    if (numArgs != 3){
        fprintf(stderr,"usage: client hostname port\n");
        exit(1);
    }
    strcpy(port, portArg);
    strcpy(hostName, hostArg);
}

/*Function saves client user's handle. Handle is prepended to all 
output sent by client through socket. */
void saveUserHandle(char* handle){
    char tempHandle[32];
    int hLen;
    /* Save user's handle, wait for valid input if handle > 10 chars */
    do{
        memset(tempHandle, 0, 32 * sizeof(char));
        printf("Enter client name(or handle): ");
        fgets(tempHandle, 32, stdin);

        /* Clear newline char from string, fgets stores it */
        tempHandle[strlen(tempHandle)-1] = '\0';
        hLen = strlen(tempHandle);
        if(hLen > MAX_HANDLE_LEN){
            printf("Enter shorter name(<= 10 characters)\n");
        }
    }while(hLen > MAX_HANDLE_LEN);
    strcpy(handle, tempHandle);
}

/* Sends message from client to server socket */
int sendMessage(char* message, char* clientHandle, int socket){
    int result;
    /*If sending quit message to server, send it, then exit */
    if(strstr(message, "\\quit") != 0){
        send(socket, message, strlen(message), 0);
        printf("%s> Client exiting, closed by client!\n", clientHandle);
        return -1;
    }
    /*This is where sending to server happens*/
    result = send(socket, message, strlen(message), 0);
    if(result == -1){  /* Send error */
        perror("send");
        return -1;
    } 
    return 0;
}

/* Receives message from server and displays data */
int receiveMessage(char* message, char* clientHandle, int socket){
    int result = recv(socket, message, MAXDATASIZE-1, 0);
    /* If recv error happens, exit */
    if(result  == -1){
        perror("recv");
        return -1;
    }
    else{
        /*Displaying message on client side */
        message[result] = '\0';  
        printf("%s\n", message); 
        /*If received quit message from server side, return -1 so we can exit loop */
        if(strstr(message, "\\quit") != 0){
            printf("%s> Client exiting, closed by server!\n", clientHandle);
            return -1;
        }
    }
    return 0;
}

/* Starts client-server chat session. It is called only once during program execution. 
   Reference: https://beej.us/guide/bgnet/html/single/bgnet.html
*/
void startSession(int sockfd, char* clientHandle){
    /* Initializing local variables, mostly preparing rx/tx buffers for chat session */
    char rxBuffer[MAXDATASIZE];
    char txBuffer[MAXDATASIZE];
    int numBytes, handleLen, result;
    saveUserHandle(clientHandle);
    handleLen = strlen(clientHandle);

    /* send/receive loop 
        There are three conditions to exit this while loop:
        1. after receiving quit from server side 
        2. after sending quit from client side
        3. enter keyboard interrupt Ctrl-C
    */
    while(1){
        /*Clearing tx/rx buffer for next iteration of send/receive */
        memset(txBuffer, 0, MAXDATASIZE);
        memset(rxBuffer, 0, MAXDATASIZE);

        /*Copying handle to txBuffer */  
        strcat(txBuffer, clientHandle); 
        strcat(txBuffer, "> ");
        printf("%s", txBuffer);
        handleLen = strlen(txBuffer);

        /* Saving data to be sent to server to txBuffer string  */
        fgets(txBuffer + handleLen, MAXDATASIZE - handleLen, stdin);

        /* Clear newline char from string since fgets stores it */
        if(strlen(txBuffer) > 1){
            txBuffer[strlen(txBuffer)-1] = '\0';
        }

        result = sendMessage(txBuffer, clientHandle, sockfd);
        /* If sending data failed or quitting client, exit loop*/
        if(result == -1){  
            break;
        } 
        else{ /* Otherwise, if succesfully sent data, wait to receive and display data from server */
            result = receiveMessage(rxBuffer, clientHandle, sockfd);
            /* If recv error or server quits, exit loop*/
            if(result  == -1){
                break;
            }
        }   
    }
    close(sockfd);
}