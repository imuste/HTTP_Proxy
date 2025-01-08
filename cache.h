/**************************************************************
 *
 *      cache.h
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>


#ifndef CACHE_H


/*
 * name:      cacheElement struct
 * purpose:   stores information about a file in the cache such as the file name,
 *            the output file name, the file extension, its age, and timing information
 */
typedef struct {
        char *URL;
        char *fullServerResponse;
        int serverPort;
        int serverResponseSize;

        unsigned long long maxAge;
        unsigned long long storageTime;
        unsigned long long staleTime;
        unsigned long long retrievalTime;

} cacheElement;

/*
 * name:      cacheInfo struct
 * purpose:   stores information about a cache instance such as its size, the
 *            underlying cache array, if it's full, and the time it was created
 */
typedef struct {
        cacheElement *cacheArray;
        int cacheSize;
        int numItems;
        int nextAvailSlot;
        unsigned long long initialTime;
} cacheInfo;







/*****************************************************************
*                    FUNCTION DECLARATIONS
*****************************************************************/
cacheInfo *newCache(int size);

void putRequest(cacheInfo *thisCache, char *URL, char *serverResponse, 
        int serverResponseSize, int serverHeaderSize, int serverPort);
int storeRequest(cacheInfo *thisCache, char *URL, char *serverResponse, 
        int serverResponseSize, int serverHeaderSize, int serverPort, int cacheSlot);
int evictRequest(cacheInfo *thisCache);
char *getResponse(cacheInfo *thisCache, char *URL, int serverPort, 
        int *responseSize, int *currAge);

unsigned long long getInitialTime();
unsigned long long getCurrTime(cacheInfo *thisCache);
unsigned long long getMaxAge(cacheInfo *thisCache, char *serverResponse, int serverHeaderSize);
int readResponseLine(char *messageBuffer, char *lineBuffer, int totalRead);
void freeCacheFields(cacheInfo *thisCache, int cacheSlot);

void freeMemory(cacheInfo *thisCache);


#endif