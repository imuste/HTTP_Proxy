/****************************************************************************
 *
 *      proxy.c
 *
 *      Isabel Muste (imuste01)
 *      09/17/2024
 *      
 *      CS 112 HW01
 * 
 *      provides the funcitonality for the proxy interface which communicates
 *      with clients and servers to transfer and relay messsages between the 
 *      two.
 *      
 *
 ****************************************************************************/

#include "proxy.h"



/*
 * name:      newProxy
 * purpose:   allocates memory for a new proxy
 * arguments: the proxy port number
 * returns:   the proxy
 * effects:   none
 */
proxy *newProxy(int portNumber, cacheInfo *thisCache)
{
        proxy *theProxy = malloc(sizeof(proxy));
        assert(theProxy != NULL);

        theProxy->proxyPort = portNumber;
        theProxy->theCache = thisCache;
        
        return theProxy;
}


/*
 * name:      proxyStart
 * purpose:   establishes a connection to any incoming clients and processes 
 *            their requests
 * arguments: the proxy
 * returns:   none
 * effects:   none
 */
void proxyListening(proxy *theProxy)
{
        //socket setup
        struct sockaddr_in proxyAddress;
        socklen_t proxyAddressLength;
        int listenSD = socket(AF_INET, SOCK_STREAM, 0);
        assert(listenSD != -1);
        theProxy->listenSD = listenSD;

        //ensure to close the socket and port after termination
        int opt = 1;
        int returnVal = setsockopt(listenSD, SOL_SOCKET, SO_REUSEADDR, &opt, 
                sizeof(opt));
        assert(returnVal != -1);

        //setup to bind the socket to port specified port and any IP address
        //this is the setup specifying the Proxy info
        memset(&proxyAddress, 0, sizeof(struct sockaddr_in));
        proxyAddress.sin_family = AF_INET;
        proxyAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        proxyAddress.sin_port = htons(theProxy->proxyPort);
        returnVal = bind(listenSD, (struct sockaddr *)&proxyAddress, 
                sizeof(struct sockaddr_in));
        assert(returnVal != -1);

        //listen for incoming connections
        returnVal = listen(listenSD, 500);
        assert(returnVal != -1);
        proxyAddressLength = sizeof(proxyAddress);
        
        while (true) {
                int clientSD = accept(listenSD, 
                        (struct sockaddr *)&proxyAddress, &proxyAddressLength);
                assert(clientSD != -1);
                theProxy->clientSD = clientSD;
                clientProcess(theProxy);
        }

        close(listenSD);
}


/*****************************************************************************
*                      FUNCTIONALITY FOR EACH CLIENT
*****************************************************************************/

/*
 * name:      clientProcess
 * purpose:   handles all function calls for a single client to be processed
 * arguments: the proxy
 * returns:   none
 * effects:   none
 */
void clientProcess(proxy *theProxy)
{
        //read the client request and get the important information from header
        readRequestFromClient(theProxy);
        parseClientMessage(theProxy);

        //check cache for request
        int age = 0;
        int responseSize = 0;
        char *cachedResponse = 
                getResponse(theProxy->theCache, theProxy->URL, 
                        theProxy->serverPort, &responseSize, &age);

        //item not in cache or stale
        if (cachedResponse == NULL) {
                sendRequestToServer(theProxy);
                readServerResponse(theProxy);
                putRequestInCache(theProxy);
        }
        //item was retrieved from cache
        else {
                theProxy->contentAge = age;
                theProxy->serverResponseSize = responseSize;
                addAgeToResponse(theProxy, cachedResponse);
        }

        sendResponseToClient(theProxy);
        shutdown(theProxy->clientSD, SHUT_RDWR);
        close(theProxy->clientSD);

        freeClientInfo(theProxy);
}


/*
 * name:      readRequestFromClient
 * purpose:   reads the request coming from the connected client
 * arguments: the proxy
 * returns:   none
 * effects:   none
 */
void readRequestFromClient(proxy *theProxy)
{
        //read any information from the client
        int totalBytesRead = 0;
        int bufferSize = 10485760;
        char *buffer = malloc(bufferSize);
        assert(buffer != NULL);
        int returnVal = 1;

        char *EOFRequest = "\r\n\r\n";
        bool EOFRequestFound = false;

        while (!EOFRequestFound) {
                returnVal = read(theProxy->clientSD, buffer + totalBytesRead, 
                        bufferSize - totalBytesRead);
                totalBytesRead += returnVal;
                assert(returnVal != -1);

                char *endStr = strstr(buffer, EOFRequest);
                if (endStr != NULL) {
                        EOFRequestFound = true;
                        int delimiterPos = endStr - buffer + 4;
                        int bytesAfterDelimiter = totalBytesRead - delimiterPos;
                        totalBytesRead -= bytesAfterDelimiter;
                }
        }
        buffer[totalBytesRead] = '\0';
        theProxy->clientRequest = buffer;
}



/*
 * name:      parseClientMessage
 * purpose:   process each line of the client request and extracts the important
 *            information and lines
 * arguments: the proxy
 * returns:   none
 * effects:   none
 */
void parseClientMessage(proxy *theProxy)
{
        int messageLength = strlen(theProxy->clientRequest);
        char *currentLine;
        char *getRequest = NULL;
        char *hostMsg = NULL;

        int totalRead = 0;
        while (totalRead < messageLength) {
                currentLine = malloc(300);
                assert(currentLine != NULL);
                totalRead = readLine(theProxy->clientRequest, currentLine, 
                        totalRead);

                //we have potentially found the get header line
                if (currentLine[0] == 'G') {
                        bool totalWord = true;
                        char *word = "GET ";
                        for (int i = 1; i < 4; i++) {
                                if (currentLine[i] != word[i]) {
                                        totalWord = false;
                                }
                        }
                        //we have found the get header line
                        if (totalWord) {
                                getRequest = malloc(strlen(currentLine) + 1);
                                strcpy(getRequest, currentLine);
                        }
                }
                else if (currentLine[0] == 'H') {
                        bool totalWord = true;
                        char *word = "Host: ";
                        for (int i = 1; i < 6; i++) {
                                if (currentLine[i] != word[i]) {
                                        totalWord = false;
                                }
                        }
                        //we have found the get header line
                        if (totalWord) {
                                hostMsg = malloc(strlen(currentLine) + 1);
                                strcpy(hostMsg, currentLine);
                        }
                }
                free(currentLine);
        }
        theProxy->clientHostLine = hostMsg;
        theProxy->clientReqLine= getRequest;
        theProxy->URL = getFullURL(theProxy);

        //extract the server port
        int serverPort = getServerPort(theProxy, hostMsg);
        if (serverPort == -1) {
                serverPort = 80;
        }
        theProxy->serverPort = serverPort;
}


/*
 * name:      sendRequestToServer
 * purpose:   establishes a connection with the server and forwards the client 
 *            request to the server
 * arguments: the proxy
 * returns:   none
 * effects:   none
 */
void sendRequestToServer(proxy *theProxy)
{
        //get host IP address
        struct hostent *hostInfo;
        char *hostName = getHostURL(theProxy); 
        hostInfo = gethostbyname(hostName);
        assert(hostInfo != NULL);
        free(hostName);

        //send the file contents to proxy
        struct sockaddr_in serverAddress;

        int serverSD = socket(AF_INET, SOCK_STREAM, 0);
        assert(serverSD != -1);
        theProxy->serverSD = serverSD;

        //ensure to close the socket and port after termination
        int opt = 1;
        int returnVal = setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, 
                &opt, sizeof(opt));
        assert(returnVal != -1);

        //Client setup to connect the socket to the server and its IP address
        //this is the setup specifying the server info
        memset(&serverAddress, 0, sizeof(struct sockaddr_in));
        serverAddress.sin_family = AF_INET;
        // serverAddress.sin_addr = *theProxy->IPaddress;
        serverAddress.sin_addr = *(struct in_addr *)hostInfo->h_addr;
        serverAddress.sin_port = htons(theProxy->serverPort);
        returnVal = connect(serverSD, (struct sockaddr *)&serverAddress, 
                sizeof(serverAddress));
        assert(returnVal != -1);

        //write into the socket
        returnVal = write(serverSD, theProxy->clientRequest, 
                strlen(theProxy->clientRequest));
        assert(returnVal != -1);
        shutdown(serverSD, SHUT_WR);
}


/*
 * name:      readServerResponse
 * purpose:   reads the response coming from the connected server
 * arguments: the proxy
 * returns:   none
 * effects:   closes the server connection
 */
void readServerResponse(proxy *theProxy) 
{
        //setup variables for server reading
        int totalBytesRead = 0;
        int bufferSize = 10485760;
        char *buffer = malloc(bufferSize);
        assert(buffer != NULL);
        int returnVal = 1;

        char *contentLengthHeader = "Content-Length: ";
        bool contentLengthFound = false;
        char *EOFRequest = "\r\n\r\n";
        bool EOFRequestFound = false;
        int bytesAfterHeader = 0;
        int headerEnd = 0;
        int totalBytesNeeded = 0;

        //read until we have found an end of header delimiter
        while (!EOFRequestFound) {
                returnVal = read(theProxy->serverSD, buffer + totalBytesRead, 
                        bufferSize - totalBytesRead);
                totalBytesRead += returnVal;
                assert(returnVal != -1);

                //check if we have found end of header delimiter
                char *endStr = strstr(buffer, EOFRequest);
                if (endStr != NULL) {
                        EOFRequestFound = true;
                        headerEnd = endStr - buffer + 4;
                        bytesAfterHeader = totalBytesRead - headerEnd;
                }
        }
        // buffer[totalBytesRead] = '\0';

        //find the content-length header field and get the content length
        int contentLength = 0;
        char *contentStr = strstr(buffer, contentLengthHeader);
        if (contentStr != NULL) {
                contentLengthFound = true;
                char *contentLengthStr = calloc(100, sizeof(char));
                int offset = contentStr - buffer + 16;
                int i = 0;
                while (buffer[offset] != '\r' && buffer[offset] != '\n' 
                        && buffer[offset] != '\0') {
                        contentLengthStr[i] = buffer[offset];
                        offset++;
                        i++;
                }
                contentLengthStr[i] = '\0';
                contentLength = atoi(contentLengthStr);
                totalBytesNeeded = headerEnd + contentLength;
                free(contentLengthStr);
        }

        //read the remaing bytes from the server response body
        while (bytesAfterHeader < contentLength) {
                returnVal = read(theProxy->serverSD, buffer + totalBytesRead, 
                        bufferSize - totalBytesRead);
                totalBytesRead += returnVal;
                bytesAfterHeader += returnVal;
                assert(returnVal != -1);
        }

        // buffer[totalBytesRead] = '\0';
        theProxy->serverResponse = buffer;
        theProxy->serverResponseSize = totalBytesRead;
        theProxy->serverHeaderSize = headerEnd;

        //close the socket
        shutdown(theProxy->serverSD, SHUT_RDWR);
        close(theProxy->serverSD);
}


/*
 * name:      sendResponseToClient
 * purpose:   forwards the server response to the connected client
 * arguments: the proxy
 * returns:   none
 * effects:   none
 */
void sendResponseToClient(proxy *theProxy)
{
        //write into the socket
        int returnVal = write(theProxy->clientSD, theProxy->serverResponse, 
                                theProxy->serverResponseSize);
        assert(returnVal != -1);
        shutdown(theProxy->clientSD, SHUT_WR);
}


/*
 * name:      freeClientInfo
 * purpose:   free allocated memory for an individual client
 * arguments: the proxy
 * returns:   none
 * effects:   frees allocated memory
 */
void freeClientInfo(proxy *theProxy)
{
        if (theProxy->clientRequest != NULL) {
                free(theProxy->clientRequest);
                theProxy->clientRequest = NULL;
        }
        if (theProxy->clientHostLine != NULL) {
                free(theProxy->clientHostLine);
                theProxy->clientHostLine = NULL;
        }
        if (theProxy->clientReqLine != NULL) {
                free(theProxy->clientReqLine);
                theProxy->clientReqLine = NULL;
        }
        if (theProxy->serverResponse != NULL) {
                free(theProxy->serverResponse);
                theProxy->serverResponse = NULL;
        }
        if (theProxy->URL != NULL) {
                free(theProxy->URL);
                theProxy->URL = NULL;
        }
        theProxy->serverPort = -1;
        theProxy->contentAge = -1;
        theProxy->clientSD = -1;
        theProxy->serverSD = -1;
        theProxy->serverResponseSize = -1;
        theProxy->serverHeaderSize = -1;

}


/*****************************************************************************
*                         CACHE HANDELING FUNCTIONS
*****************************************************************************/

/*
 * name:      putRequestInCache
 * purpose:   adds the request to the cache by calling cache module function
 * arguments: the proxy
 * returns:   none
 * effects:   none
 */
void putRequestInCache(proxy *theProxy)
{       
        //allocate space for URL
        int copySize = strlen(theProxy->URL) + 1;
        char *URLCopy = malloc(copySize);
        assert(URLCopy != NULL);
        strcpy(URLCopy, (theProxy->URL));

        //allocate space for full server reponse
        copySize = theProxy->serverResponseSize;
        char *serverCopy = malloc(copySize);
        assert(serverCopy != NULL);
        memcpy(serverCopy, theProxy->serverResponse, copySize);

        putRequest(theProxy->theCache, URLCopy, serverCopy, 
                theProxy->serverResponseSize, theProxy->serverHeaderSize, 
                theProxy->serverPort);
}


void addAgeToResponse(proxy *theProxy, char *cachedResponse)
{       
        //make string of int age
        char ageStr[50];
        sprintf(ageStr, "%d", theProxy->contentAge);
        int ageStrLength = strlen(ageStr);
        int ageLineLength = ageStrLength + 7;

        //create the new age line
        char *ageLine = malloc(ageLineLength + 1);
        assert(ageLine != NULL);
        sprintf(ageLine, "Age: %s\r\n", ageStr);

        //calculate sizings for new cache response
        int oldResponseSize = theProxy->serverResponseSize;
        int newResponseSize = oldResponseSize + ageLineLength;
        char *newResponse = malloc(newResponseSize);
        assert(newResponse != NULL);

        //add the new age line after the first line in the header
        char *lineDelim = "\r\n";
        char *endFirstLine = strstr(cachedResponse, lineDelim);
        if (endFirstLine != NULL) {
                int bytesFirstLine = endFirstLine - cachedResponse + 2;
                memcpy(newResponse, cachedResponse, bytesFirstLine);
                memcpy(newResponse + bytesFirstLine, ageLine, ageLineLength);
                memcpy(newResponse + bytesFirstLine + ageLineLength, 
                cachedResponse + bytesFirstLine, oldResponseSize - bytesFirstLine);
        }
        free(ageLine);
        if (theProxy->serverResponse != NULL) {
                free(theProxy->serverResponse);
                theProxy->serverResponse = NULL;
        }
        theProxy->serverResponse = newResponse;
        theProxy->serverResponseSize = newResponseSize;
}



/*****************************************************************************
*                       HELPER FUNCTIONS
*****************************************************************************/


/*
 * name:      readLine
 * purpose:   reads a line from the message and stores it in the lineBuffer
 * arguments: file descriptor, buffer, total bytes read
 * returns:   number of total bytes read
 * effects:   none
 */
int readLine(char *messageBuffer, char *lineBuffer, int totalRead)
{
        int bytesRead = 0;

        // read one char at a time until we reach EOF or \r\n
        while ((messageBuffer[totalRead + bytesRead] != '\n') && 
                (messageBuffer[totalRead + bytesRead] != '\0')) {
                if ((messageBuffer[totalRead + bytesRead] == '\r') && 
                        (messageBuffer[totalRead + bytesRead + 1] == '\n')) {
                        break;   
                }
                lineBuffer[bytesRead] = messageBuffer[totalRead + bytesRead];
                bytesRead++;
                
        }

        lineBuffer[bytesRead] = '\0';
        return totalRead + bytesRead + 2;
}

/*
 * name:      getServerPort
 * purpose:   extracts a server port from the host line in the client request
 * arguments: the proxy, the host line
 * returns:   the server port or -1 if no port was found
 * effects:   none
 */
int getServerPort(proxy *theProxy, char *hostMsg)
{
        int bytesRead = 5;
        bool portFound = false;
        char *portStr = calloc(100, sizeof(char));
        int portCtr = 0;
        int portNumber = -1;

        //read through the enture line
        while ((hostMsg[bytesRead] != '\0')) {
                //find server port deliminator
                if (hostMsg[bytesRead] == ':') {
                        portFound = true;
                }
                else if (portFound) {
                        portStr[portCtr] = hostMsg[bytesRead];
                        portCtr++;
                }
                bytesRead++;
        }
        portStr[portCtr] = '\0';

        if (portFound) {
                portNumber = atoi(portStr);
        }

        free(portStr);
        return portNumber;
}


/*
 * name:      getHostURL
 * purpose:   gets the host URL without the port number
 * arguments: the proxy
 * returns:   the host URL
 * effects:   none
 */
char *getHostURL(proxy *theProxy)
{
        char *URL = malloc(150);
        assert(URL != NULL);
        int ogLineCtr = 6;
        int URLctr = 0;

        while (theProxy->clientHostLine[ogLineCtr] != ' ' 
                && theProxy->clientHostLine[ogLineCtr] != '\n' 
                && theProxy->clientHostLine[ogLineCtr] != '\r' 
                && theProxy->clientHostLine[ogLineCtr] != '\0'
                && theProxy->clientHostLine[ogLineCtr] != ':') {

                URL[URLctr] = theProxy->clientHostLine[ogLineCtr];
                URLctr++;
                ogLineCtr++;
        }
        URL[URLctr] = '\0';

        return URL;
}


/*
 * name:      getFullURL
 * purpose:   gets the full URL including the path
 * arguments: the proxy
 * returns:   the full URL
 * effects:   none
 */
char *getFullURL(proxy *theProxy)
{
        char *URL = malloc(150);
        assert(URL != NULL);
        int ogLineCtr = 4;
        int URLctr = 0;

        while (theProxy->clientReqLine[ogLineCtr] != ' ' 
                && theProxy->clientReqLine[ogLineCtr] != '\n' 
                && theProxy->clientReqLine[ogLineCtr] != '\r' 
                && theProxy->clientReqLine[ogLineCtr] != '\0') {

                URL[URLctr] = theProxy->clientReqLine[ogLineCtr];
                URLctr++;
                ogLineCtr++;
        }
        URL[URLctr] = '\0';

        return URL;
}


/*
 * name:      freeProxy
 * purpose:   frees the proxy memory allocations
 * arguments: the proxy
 * returns:   none
 * effects:   none
 */
void freeProxy(proxy *theProxy)
{
        if (theProxy != NULL) {
                free(theProxy);
                theProxy = NULL;
        }

}