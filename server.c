/****************************************************************************
 *
 *      server.c
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


        struct sockaddr_in serverAddress;
        socklen_t serverAddressLength;

        int ogSD = socket(AF_INET, SOCK_STREAM, 0);
        assert(ogSD != -1);

        //ensure to close the socket and port after termination
        int opt = 1;
        setsockopt(ogSD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        //setup to bind the socket to port 9099 and any IP address
        //this is the setup specifying the server info
        memset(&serverAddress, 0, sizeof(struct sockaddr_in));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddress.sin_port = htons(9099);
        int returnVal = bind(ogSD, (struct sockaddr *)&serverAddress, 
                sizeof(struct sockaddr_in));
        assert(returnVal != -1);

        //listen for incoming connections
        returnVal = listen(ogSD, 500);
        assert(returnVal != -1);

        //block until a connection request is seen
        serverAddressLength = sizeof(serverAddress);

        while (1) {
                int newSD = accept(ogSD, (struct sockaddr *)&serverAddress, 
                        &serverAddressLength);
                assert(newSD != -1);


                //read any information from the socket
                char *buffer = malloc(10485760);
                int totalBytesRead = 0;
                int bufferSize = 10485760;
                returnVal = 1;
                while (returnVal != 0) {
                        returnVal = read(newSD, buffer + totalBytesRead, 
                                bufferSize - totalBytesRead);
                        totalBytesRead += returnVal;
                        assert(returnVal != -1);
                }
                buffer[totalBytesRead] = '\0';

                //write into the socket
                returnVal = write(newSD, fileContents, fileSize + 1);
                assert(returnVal != -1);
                shutdown(newSD, SHUT_WR);

                //close necessary connections
                shutdown(newSD, SHUT_RDWR);
                close(newSD);
                free(buffer);
        }


        close(ogSD);
        close(fd);

        return EXIT_SUCCESS;
}