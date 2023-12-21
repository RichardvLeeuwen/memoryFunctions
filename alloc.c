#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct Block {
    size_t blockSize;
    size_t *pointer;
    struct Block *next;
};

struct Block *heapHead = NULL; //new blocks get attached at the head of the list, there is a list heap and list freeBlocks
struct Block *freeHead = NULL;

void *mymalloc(size_t size) {

    if(size == 0)   {
        return NULL;
    }

    while(size % 8 != 0) { //aligns to sizeof(long) bytes
        size++;
    }

    if(freeHead != NULL) {
        int isFound = 0;

        struct Block *currentBlock;
        struct Block *prevBlock;
        currentBlock = freeHead;
        prevBlock = NULL;

        if (currentBlock->blockSize >= size) {
            isFound = 1;
        }
        while ((currentBlock != NULL) && (isFound == 0)) { //looks through freelist for large enough block
            if (currentBlock->blockSize >= size) {
                isFound = 1;
            }
            else {
                prevBlock = currentBlock;
                currentBlock = currentBlock->next;
            }
        }

        if (isFound == 1) {
            if (currentBlock == freeHead) { //removes found block from freelist if head
                freeHead = freeHead->next;
            }
            else if (currentBlock->next == NULL) { //removes found block from freelist if tail
                prevBlock->next = NULL;
            }
            else { //removes found block from freelist if neither head or tail
                prevBlock->next = currentBlock->next;
            }

            int bytesUnused = currentBlock->blockSize - size;

            if((bytesUnused % 8 == 0) && (bytesUnused != 0)) { //splits freeblock if space unused and properly aligned
                currentBlock->blockSize = size;

                struct Block *newFreeBlock;
                newFreeBlock = sbrk(sizeof(struct Block));
                newFreeBlock->blockSize = bytesUnused;
                newFreeBlock->pointer = currentBlock->pointer+ (size/8);

                newFreeBlock->next = freeHead;
                freeHead = newFreeBlock;
            }

            currentBlock->next = heapHead;
            heapHead = currentBlock;

            return heapHead->pointer;
        }
    }

    struct Block *newBlock; //if no available freeblock to reuse, create new one
    newBlock = sbrk(sizeof(struct Block));
    newBlock->pointer = sbrk(size);
    newBlock->blockSize = size;

    newBlock->next = heapHead;
    heapHead = newBlock;

    return heapHead->pointer;
}

void *mycalloc(size_t nmemb, size_t size) {

    size_t totalSize = nmemb * size;

    if(totalSize == 0)   {
        return NULL;
    }

    while(totalSize % 8 != 0) {
        totalSize++;
    }

    size_t *callocPtr = mymalloc(totalSize);
    memset(callocPtr, 0, totalSize);

    return callocPtr;
}

void myfree(void *ptr) {

    if(ptr == NULL) {
        return;
    }
    if(heapHead == NULL) {
        return;
    }

    int isFound = 0;

    struct Block *currentBlock;
    struct Block *prevBlock;
    currentBlock = heapHead;
    prevBlock = NULL;

    if(ptr == currentBlock->pointer) {
        isFound = 1;
    }
    while((currentBlock != NULL)&&(isFound==0)) { //looks through heap for the block to be freed
        if(ptr == currentBlock->pointer) {
            isFound = 1;
        }
        else  {
            prevBlock = currentBlock;
            currentBlock = currentBlock->next;
        }
    }

    if(isFound==1) {
        if(currentBlock==heapHead) {  //removes found block from heap if head
            heapHead = heapHead->next;
        }
        else if(currentBlock->next==NULL) { //removes found block from heap if tail
            prevBlock->next = NULL;
        }
        else { //removes found block from heap if neither head or tail
            prevBlock->next = currentBlock->next;
        }

        struct Block *cBlock;
        struct Block *pBlock = NULL;
        cBlock = freeHead;
        int isMerged = 0; //if true, the newly freed block is merged with an already existing free block

        while (cBlock != NULL) { //merges blocks by checking if blocks are connected with the newly freed block coming from the head
            if((cBlock->pointer+((cBlock->blockSize + sizeof(struct Block))/8))==currentBlock->pointer) {
                cBlock->blockSize = cBlock->blockSize + currentBlock->blockSize + sizeof(struct Block);

                isMerged = 1;
            }
            pBlock = cBlock;
            cBlock = cBlock->next;
        }

        cBlock = freeHead;
        pBlock = NULL;

        while (cBlock != NULL) { //merges blocks by checking if blocks are connected with the newly freed block coming from the tail
            if((currentBlock->pointer+((currentBlock->blockSize + sizeof(struct Block))/8))==cBlock->pointer) {
                currentBlock->blockSize = currentBlock->blockSize + cBlock->blockSize + sizeof(struct Block);

                if(pBlock == NULL) { //absorb the found block into the newly freed block
                    freeHead = freeHead->next;
                }
                else if(cBlock->next == NULL) {
                    pBlock->next = NULL;
                }
                else {
                    pBlock->next = cBlock->next;
                }
            }
            pBlock = cBlock;
            cBlock = cBlock->next;
        }

        if(isMerged == 0) {
            currentBlock->next = freeHead;
            freeHead = currentBlock;
        }
    }

    return;
}

void *myrealloc(void *ptr, size_t size) {

    if(ptr == NULL && size == 0) {
        return NULL;
    }
    else if(ptr == NULL && size != 0) {
        size_t *newPtr = mymalloc(size);
        return newPtr;
    }
    else if(ptr != NULL && size == 0) {
        myfree(ptr);
        return NULL;
    }

    int isFound = 0;

    struct Block *currentBlock;
    currentBlock = heapHead;

    if(ptr == currentBlock->pointer) {
        isFound = 1;
    }
    while((currentBlock != NULL)&&(isFound==0)) { //looks for the pointer in heap
        if(ptr == currentBlock->pointer) {
            isFound = 1;
        }
        else {
            currentBlock = currentBlock->next;
        }
    }

    if(currentBlock->blockSize >= size) {
        return currentBlock->pointer;
    }
    else if(currentBlock->blockSize < size) {
        size_t *newPtr = mymalloc(size);
        memcpy(newPtr, currentBlock->pointer, currentBlock->blockSize);
        return newPtr;
    }

    return NULL;
}

/*
 * Enable the code below to enable system allocator support for your allocator.
 * Doing so will make debugging much harder (e.g., using printf may result in
 * infinite loops).
 */
#if 0
void *malloc(size_t size) { return mymalloc(size); }
void *calloc(size_t nmemb, size_t size) { return mycalloc(nmemb, size); }
void *realloc(void *ptr, size_t size) { return myrealloc(ptr, size); }
void free(void *ptr) { myfree(ptr); }
#endif
