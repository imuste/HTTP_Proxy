/**************************************************************
 *
 *      proxy.h
 *
 *      Isabel Muste (imuste01)
 *      09/17/2024
 *      
 *      CS 112 HW01
 * 
 *      ...
 *
 *
 **************************************************************/

#include <math.h> 
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "cache.h"


#ifndef PROXY_H

typedef struct {
        //consistent throughout program
        int proxyPort;
        int listenSD;

        //socket Info
        int clientSD;
        int serverSD;

        //client/server information
        int serverPort;
        int contentAge;
        int serverResponseSize;
        int serverHeaderSize;
        char *clientRequest;
        char *serverResponse;
        char *clientHostLine;
        char *clientReqLine;
        char *URL;

        cacheInfo *theCache;
} proxy;


/*****************************************************************
*                    FUNCTION DECLARATIONS
*****************************************************************/
proxy *newProxy(int portNumber, cacheInfo *thisCache);
void proxyListening(proxy *theProxy);
void clientProcess(proxy *theProxy);

void readRequestFromClient(proxy *theProxy);
void parseClientMessage(proxy *theProxy);
void sendRequestToServer(proxy *theProxy);
void readServerResponse(proxy *theProxy);
void sendResponseToClient(proxy *theProxy);
void freeClientInfo(proxy *theProxy);

void putRequestInCache(proxy *theProxy);

int readLine(char *messageBuffer, char *lineBuffer, int totalRead);
int getServerPort(proxy *theProxy, char *hostMsg);
char *getHostURL(proxy *theProxy);
char *getFullURL(proxy *theProxy);
void addAgeToResponse(proxy *theProxy, char *cachedResponse);
void freeProxy(proxy *theProxy);

// void printProxyFields(proxy *theProxy);

#endif