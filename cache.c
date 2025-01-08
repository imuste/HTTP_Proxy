/*****************************************************************************
 *
 *      cache.c
 *
 *      Isabel Muste (imuste01)
 *      09/17/2024
 *      
 *      CS 112 HW01
 *
 *
 ****************************************************************************/

#include "cache.h"


/*
 * name:      newCache
 * purpose:   creates a new cache instance which is returned to the caller
 * arguments: the cache size
 * returns:   a reference to the cache struct
 * effects:   allocates memory for the cache struct and the underlying array
 */
cacheInfo *newCache(int size)
{
        cacheInfo *theInfo = malloc(sizeof(cacheInfo));
        assert(theInfo != NULL);

        theInfo->cacheArray = (cacheElement *)malloc(size * sizeof(cacheElement));
        assert(theInfo->cacheArray != NULL);
        theInfo->cacheSize = size;
        theInfo->nextAvailSlot = 0;
        theInfo->numItems = 0;
        theInfo->initialTime = getInitialTime();
        
        return theInfo;
}


/*****************************************************************
*                    PUT REQUEST FUNCTIONALITY
*****************************************************************/

/*
 * name:      putRequest
 * purpose:   puts a new request into the cache and evicts an old one if 
 *            necessary
 * arguments: cache struct, the full request, server response, URL, server port
 * returns:   none
 * effects:   none
 */
void putRequest(cacheInfo *thisCache, char *URL, char *serverResponse, 
        int serverResponseSize, int serverHeaderSize, int serverPort)
{       
        //determine if the file exists already
        bool responseFound = false;
        int cacheSlot = 0;

        //find the correct cache slot for this file
        for (int i = 0; i < thisCache->numItems; i++) {
                if (strcmp(thisCache->cacheArray[i].URL, URL) == 0) {
                        if (thisCache->cacheArray[i].serverPort == serverPort) {
                                cacheSlot = i;
                                responseFound = true;
                                break;
                        }
                }
        }

        if (responseFound) {
                // printf("updaing item in cache at slot %i\n", cacheSlot);
                freeCacheFields(thisCache, cacheSlot);
                storeRequest(thisCache, URL, serverResponse, serverResponseSize, 
                        serverHeaderSize, serverPort, cacheSlot);
        }
        else if (!responseFound && thisCache->numItems < thisCache->cacheSize) {
                // printf("Space in cache at slot %i\n", thisCache->nextAvailSlot);
                storeRequest(thisCache, URL, serverResponse, serverResponseSize, 
                        serverHeaderSize, serverPort, thisCache->nextAvailSlot);
                thisCache->nextAvailSlot++;
                thisCache->numItems++;
        }
        else if (!responseFound && thisCache->numItems >= thisCache->cacheSize) {
                int availableSlot = evictRequest(thisCache);
                // printf("need to evict item at index %i\n", availableSlot);
                storeRequest(thisCache, URL, serverResponse, serverResponseSize, 
                        serverHeaderSize, serverPort, availableSlot);
        }
        
}


/*
 * name:      storeRequest
 * purpose:   stores the information about the request in the cache
 * arguments: cache instance, URL, server response, cache slot
 * returns:   int indicating success or failure
 * effects:   none
 */
int storeRequest(cacheInfo *thisCache, char *URL, char *serverResponse, 
        int serverResponseSize, int serverHeaderSize, int serverPort, int cacheSlot)
{
        unsigned long long currTime = getCurrTime(thisCache);
        unsigned long long maxAge = getMaxAge(thisCache, serverResponse, 
                serverHeaderSize);

        thisCache->cacheArray[cacheSlot].URL = URL;
        thisCache->cacheArray[cacheSlot].fullServerResponse = serverResponse;
        thisCache->cacheArray[cacheSlot].serverResponseSize = serverResponseSize;
        thisCache->cacheArray[cacheSlot].serverPort = serverPort;
        thisCache->cacheArray[cacheSlot].storageTime = currTime;
        thisCache->cacheArray[cacheSlot].retrievalTime = currTime;
        thisCache->cacheArray[cacheSlot].staleTime = currTime + maxAge;
        thisCache->cacheArray[cacheSlot].maxAge = maxAge;
}


/*
 * name:      evictRequest
 * purpose:   evicts a stale item or the least recently retrieved item from the 
 *            cache
 * arguments: cache instance
 * returns:   the cache slot of the evicted item
 * effects:   frees memory for evicted item
 */
int evictRequest(cacheInfo *thisCache)
{
        unsigned long long currentTime = getCurrTime(thisCache);

        bool staleFileFound = false;
        int staleSlot = 0;

        unsigned long long oldestRetrievedTime = ULLONG_MAX;
        int oldestRetrievedSlot = 0;

        for (int i = 0; i < thisCache->cacheSize; i++) {
                //find any stale files
                if (thisCache->cacheArray[i].staleTime < currentTime) {
                        staleFileFound = true;
                        staleSlot = i;
                        break;
                }

                //if a file was retrieved, find the lowest retrieval time
                if (thisCache->cacheArray[i].retrievalTime < oldestRetrievedTime) {
                        oldestRetrievedTime = 
                                thisCache->cacheArray[i].retrievalTime;
                        oldestRetrievedSlot = i;
                }
        }

        //if one or more items were stale, return the first one found
        if (staleFileFound) {
                freeCacheFields(thisCache, staleSlot);
                return staleSlot;
        }

        //if every item has been retrieved and none are stale, return the slot 
        // of the item least recently retrieved
        else {
                freeCacheFields(thisCache, oldestRetrievedSlot);
                return oldestRetrievedSlot;
        }

}



/*****************************************************************************
*                          GET FILE FUNCTIONALITY
*****************************************************************************/

/*
 * name:      getResponse
 * purpose:   retrieves an item from the cache if it is not stale
 * arguments: cache instance, URL, item age reference
 * returns:   the cached response or NULL if item is stale or not found
 * effects:   none
 */
char *getResponse(cacheInfo *thisCache, char *URL, int serverPort, 
        int *responseSize, int *currAge)
{
        bool itemFound = false;
        unsigned long long currTime = getCurrTime(thisCache);

        for (int i = 0; i < thisCache->numItems; i++) {
                if ((strcmp(thisCache->cacheArray[i].URL, URL) == 0) 
                        && thisCache->cacheArray[i].serverPort == serverPort) {

                        if (currTime < thisCache->cacheArray[i].staleTime) {
                                itemFound = true;
                                thisCache->cacheArray[i].retrievalTime = currTime;
                                unsigned long long theAge = 
                                currTime - thisCache->cacheArray[i].storageTime;
                                *currAge = (int)(theAge / 1000000000);
                                *responseSize = thisCache->cacheArray[i].serverResponseSize;
                                return thisCache->cacheArray[i].fullServerResponse;
                        }
                        return NULL;
                }
        }
        return NULL;
}


/*****************************************************************************
*                            HELPER FUNCTIONS
*****************************************************************************/

/*
 * name:      getInitialTime
 * purpose:   gets the initial time at program startup
 * arguments: none
 * returns:   the initial time
 * effects:   none
 */
unsigned long long getInitialTime()
{
        struct timespec currTime;
        int returnVal = clock_gettime(CLOCK_MONOTONIC, &currTime);
        assert(returnVal != -1);
        unsigned long long theTime = (currTime.tv_sec * 1000000000ULL) + 
                currTime.tv_nsec;

        return theTime;
}

/*
 * name:      getCurrTime
 * purpose:   gets the current time minus the initial time to leave enough bits
 * arguments: none
 * returns:   the current time
 * effects:   none
 */
unsigned long long getCurrTime(cacheInfo *thisCache)
{
        struct timespec currTime;
        int returnVal = clock_gettime(CLOCK_MONOTONIC, &currTime);
        assert(returnVal != -1);
        unsigned long long theTime = (currTime.tv_sec * 1000000000ULL) + 
                currTime.tv_nsec - thisCache->initialTime;

        return theTime;
}

/*
 * name:      getMaxAge
 * purpose:   retrieves the max age of a cache item from the server 
 *            response header
 * arguments: the cache instance, the reponse header
 * returns:   the maximum age of the cache item in nanoseconds
 * effects:   none
 */
unsigned long long getMaxAge(cacheInfo *thisCache, char *serverResponse, 
        int serverHeaderSize)
{
        char *currentLine;
        char *maxAgeStr = NULL;
        unsigned long long maxAge;
        maxAge = ((unsigned long long)3600 * (unsigned long long)1000000000);

        int totalRead = 0;
        while (totalRead < serverHeaderSize) {
                currentLine = malloc(200);
                assert(currentLine != NULL);
                totalRead = readResponseLine(serverResponse, currentLine, totalRead);

                //we have potentially found the cache control header line
                if (currentLine[0] == 'C') {
                        bool totalWord = true;
                        char *word = "Cache-Control: max-age=";
                        for (int i = 1; i < 23; i++) {
                                if (currentLine[i] != word[i]) {
                                        totalWord = false;
                                }
                        }
                        if (totalWord) {
                                maxAgeStr = calloc(100, sizeof(char));
                                int i = 23;
                                while ((currentLine[i] != '\n') && (currentLine[i] != '\r')
                                && (currentLine[i] != ' ') && (currentLine[i] != '\0')) {
                                        maxAgeStr[i - 23] = currentLine[i];
                                        i++;
                                }
                                maxAge = ((unsigned long long)atoi(maxAgeStr)) 
                                        * 1000000000;
                                free(maxAgeStr);
                                free(currentLine);
                                break;
                        }
                }
                free(currentLine);
        }

        return maxAge;
}


/*
 * name:      readLine
 * purpose:   reads a line from the message and stores it in the lineBuffer
 * arguments: message buffer, line buffer, total bytes read
 * returns:   number of total bytes read
 * effects:   none
 */
int readResponseLine(char *messageBuffer, char *lineBuffer, int totalRead)
{
        int bytesRead = 0;

        // read one char at a time until we reach EOF or \n
        while ((messageBuffer[totalRead + bytesRead] != '\n') && 
                (messageBuffer[totalRead + bytesRead] != '\0')) {
                lineBuffer[bytesRead] = messageBuffer[totalRead + bytesRead];
                bytesRead++;
                
        }
        lineBuffer[bytesRead] = '\0';
        return totalRead + bytesRead + 1;
}

/*
 * name:      freeCacheFields
 * purpose:   frees any cache fields that aren't freed yet
 * arguments: the cache instance, the cacheslot to free from
 * returns:   none
 * effects:   frees memory allocated
 */
void freeCacheFields(cacheInfo *thisCache, int cacheSlot)
{
        if (thisCache->cacheArray[cacheSlot].URL != NULL) {
                free(thisCache->cacheArray[cacheSlot].URL);
                thisCache->cacheArray[cacheSlot].URL = NULL;
        }
        if (thisCache->cacheArray[cacheSlot].fullServerResponse != NULL) {
                free(thisCache->cacheArray[cacheSlot].fullServerResponse);
                thisCache->cacheArray[cacheSlot].fullServerResponse = NULL;
        }
        thisCache->cacheArray[cacheSlot].maxAge = -1;
        thisCache->cacheArray[cacheSlot].retrievalTime = -1;
        thisCache->cacheArray[cacheSlot].serverPort = -1;
        thisCache->cacheArray[cacheSlot].serverResponseSize = -1;
        thisCache->cacheArray[cacheSlot].staleTime = -1;
        thisCache->cacheArray[cacheSlot].storageTime = -1;
}


/*
 * name:      freeMemory
 * purpose:   frees all memory allocated for the cached files
 * arguments: cache instance
 * returns:   none
 * effects:   none
 */
void freeMemory(cacheInfo *thisCache)
{
        if (thisCache != NULL) {
                if (thisCache->cacheArray != NULL) {
                        for (int i = 0; i < thisCache->numItems; i++) {
                                if (thisCache->cacheArray[i].URL != NULL) {
                                        free(thisCache->cacheArray[i].URL);
                                        thisCache->cacheArray[i].URL = NULL;
                                }
                                if (thisCache->cacheArray[i].fullServerResponse != NULL) {
                                        free(thisCache->cacheArray[i].fullServerResponse);
                                        thisCache->cacheArray[i].fullServerResponse = NULL;
                                }
                        }
                        free(thisCache->cacheArray);
                        thisCache->cacheArray = NULL;
                }
        }

        free(thisCache);
        thisCache = NULL;
}



