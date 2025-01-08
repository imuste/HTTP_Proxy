/****************************************************************************
 *
 *      proxyDriver.c
 *
 *      Isabel Muste (imuste01)
 *      09/017/2024
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

#include "proxy.h"





/*
 * name:      main
 * purpose:   opens the commands file and initializes a new cache instance
 * arguments: argc, argv
 * returns:   exit success
 * effects:   none
 */
int main(int argc, char *argv[])
{       
        int port = atoi(argv[1]);

        cacheInfo *thisCache = newCache(10);
        proxy *thisProxy = newProxy(port, thisCache);
        
        proxyListening(thisProxy);

        freeMemory(thisCache);


        return EXIT_SUCCESS;
}




