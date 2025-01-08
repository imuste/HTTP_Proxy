/****************************************************************************
 *
 *      client.c
 *
 *      Isabel Muste (imuste01)
 *      09/17/2024
 *      
 *      CS 112 HW01
 * 
 *      ...
 *      
 *
 ****************************************************************************/

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
#include <arpa/inet.h>


/*
 * name:      main
 * purpose:   opens the commands file and initializes a new cache instance
 * arguments: argc, argv
 * returns:   exit success
 * effects:   none
 */
int main(int argc, char *argv[])
{       
        char *fileName = argv[1];
        int fd = open(fileName, O_RDONLY);
        assert(fd != -1);

        struct stat st;
        stat(fileName, &st);
        size_t fileSize = st.st_size;

        char *fileContents = malloc(fileSize + 1);
        assert(fileContents != NULL);

        int bytesRead = read(fd, fileContents, fileSize);
        assert(bytesRead == fileSize);

        fileContents[fileSize] = '\0';

        // printf("file contents: %s\n", fileContents);

        //send the file contents to proxy
        struct sockaddr_in clientAddress, proxyAddress;

        int clientSD = socket(AF_INET, SOCK_STREAM, 0);
        assert(clientSD != -1);

        //ensure to close the socket and port after termination
        int opt = 1;
        setsockopt(clientSD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        //Client setup to bind the socket to port 9099 and any IP address
        //this is the setup specifying the client info
        memset(&clientAddress, 0, sizeof(struct sockaddr_in));
        clientAddress.sin_family = AF_INET;
        clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        clientAddress.sin_port = htons(9095);
        int returnVal = bind(clientSD, (struct sockaddr *)&clientAddress, 
                sizeof(clientAddress));
        assert(returnVal != -1);

        //Client setup to connect the socket to the proxy and its IP address
        //this is the setup specifying the proxy info
        memset(&proxyAddress, 0, sizeof(struct sockaddr_in));
        proxyAddress.sin_family = AF_INET;
        proxyAddress.sin_addr.s_addr = inet_addr("10.4.2.18");
        proxyAddress.sin_port = htons(9098);
        returnVal = connect(clientSD, (struct sockaddr *)&proxyAddress, 
                sizeof(proxyAddress));
        assert(returnVal != -1);

        //write into the socket
        returnVal = write(clientSD, fileContents, fileSize + 1);
        assert(returnVal != -1);
        shutdown(clientSD, SHUT_WR);


        //read any information from the socket
        char *buffer = malloc(10485760);
        int totalBytesRead = 0;
        int bufferSize = 10485760;
        returnVal = 1;
        while (returnVal != 0) {
                returnVal = read(clientSD, buffer + totalBytesRead, 
                        bufferSize - totalBytesRead);
                totalBytesRead += returnVal;
                assert(returnVal != -1);
        }
        buffer[totalBytesRead] = '\0';

        //server response in client
        printf("%s", buffer);



        //close the socket
        shutdown(clientSD, SHUT_RDWR);
        close(clientSD);
        close(fd);
        free(fileContents);
        free(buffer);
        
        return EXIT_SUCCESS;
}