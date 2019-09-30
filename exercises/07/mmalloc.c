#define _GNU_SOURCE
#include "tlpi_hdr.h"

/* Excercise 7-2
 *
 * Implement malloc and free. I'm wondering if I've tried to do something
 * like this before. This took a while, but was a fun exercise.
 *
 * This implementation returns uses "first" allocation strategy for recycling
 * memory. Nodes are merged at every mfree call, and the last node is expanded
 * when the sbrk is increased.
 */

typedef struct FreeNode{
    // The absolute size of the memory, including the room for a size_t.
    size_t size;
    struct FreeNode *prev;
    struct FreeNode *next;
} FreeNode;

#define MINALLOC sizeof(FreeNode)
#define MINSBRK 1000
#define SHRINK_THRESHOLD 10000

static FreeNode *FREE_HEAD = NULL;
static FreeNode *FREE_TAIL = NULL;

/* Insert the given node in correct memory order */
void insertNode(FreeNode *node) {
    // Start the list, if there is none.
    if (FREE_HEAD == NULL) {
        FREE_HEAD = node;
        FREE_HEAD->prev = NULL;
        FREE_TAIL = FREE_HEAD;
        return;
    }

    // Insert before the head, if memory ordering requires.
    if (node < FREE_HEAD) {
        node->prev = NULL;
        node->next = FREE_HEAD;
        FREE_HEAD = node;
        return;
    }

    // Find the correct index to insert into.
    FreeNode *curr = FREE_HEAD;
    while(curr->next != NULL && curr->next < node) 
        curr = curr->next;

    node->next = curr->next;
    node->prev = curr;
    curr->next = node;
    if (node->next == NULL)
        FREE_TAIL = node;
    return;
}

/* Remove a freenode from the list, keeping FREE_HEAD and FREE_TAIL updated */
void removeNode(FreeNode *node) {
    if (node->prev != NULL)
        node->prev->next = node->next;
    if (node->next != NULL)
        node->next->prev = node->prev;

    if (FREE_HEAD == node) {
        FREE_HEAD = node->next;
        if (FREE_TAIL == node)
            FREE_TAIL = FREE_HEAD;
    } else {
        if (FREE_TAIL == node)
            FREE_TAIL = node->prev;
        if (FREE_HEAD == node)
            FREE_HEAD = FREE_TAIL;
    }
}

/* Return the first FreeNode which can accommodate the request, or return NULL 
 *
 * minSize is expected to be the REAL size needed (includes room to record size)
 * */
FreeNode *findNode(size_t minSize) {
    if (FREE_HEAD == NULL)
        return NULL;

    /* Need to find a node which is either:
     *    - Exactly the size requested.
     *    - The size requested + enough room for a FreeNode.
     */

    FreeNode *curr = FREE_HEAD;
    while (curr != NULL && 
           curr->size != minSize && 
           curr->size < minSize + sizeof(FreeNode))
        curr = curr->next;

    return curr;
}

// True if two nodes are adjacent in memory.
int canCombineNodes(FreeNode *left, FreeNode *right) {
    if(left == NULL)
        return 0;

    if (((void *) left) + left->size == right)
        return 1;
    return 0;
}

// Expands left to include right, removing right.
FreeNode *combineNodes(FreeNode *left, FreeNode *right) {
    removeNode(right);
    left->size += right->size;
    return left;
}

// Combine all neighbouring nodes.
FreeNode *combineNeighbours(FreeNode *node) {
    while (canCombineNodes(node, node->next))
        node = combineNodes(node, node->next);
    while (canCombineNodes(node->prev, node))
        node = combineNodes(node->prev, node);
    return node;
}

FreeNode *growHeap(size_t growSize) {
    size_t newSize = max(growSize, MINSBRK);
    FreeNode *newNode = sbrk(newSize);
    if (newNode == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    newNode->size = newSize;
    newNode->prev = FREE_TAIL;
    newNode->next = NULL;
    if (newNode->prev == NULL) {
        FREE_HEAD = FREE_TAIL = newNode;
    } else {
        newNode->prev->next = newNode;
    }
    newNode = combineNeighbours(newNode);
    return newNode;
}


void mfree(void *ptr) {
    // We expect ptr - sizeof(size_t) to be the real size of the block, such that
    // realStart + blockSize - 1 is the last writable byte.
    void *realStart = (ptr - sizeof(size_t));
    size_t realSize = *(size_t *)(realStart);

    FreeNode *node = (FreeNode *) realStart;
    node->size = realSize;
    node->prev = node->next = NULL;
    insertNode(node);
    node = combineNeighbours(node);

    // Shrink the heap if possible.
    if (node == FREE_TAIL &&
        node->size > SHRINK_THRESHOLD &&
        (void *) node + node->size == sbrk(0))
    {
        removeNode(node);
        sbrk(-(node->size));
    }
}

void *mmalloc(size_t size) {
    // Try to find an existing node to use - if not, grow heap to hold.
    size_t realSize = max(size + sizeof(size_t), MINALLOC);
    FreeNode *insertionAddress = findNode(realSize);
    if (insertionAddress == NULL) {
        insertionAddress = growHeap(realSize);
    }

    // Divide the space, if necessary.
    if (insertionAddress->size > realSize) {
        FreeNode *newNode = (FreeNode *)(((void *)insertionAddress) + realSize);
        newNode->size = insertionAddress->size - realSize;
        newNode->prev = insertionAddress;
        newNode->next = insertionAddress->next;
        insertionAddress->next = newNode;
    }

    // Remove the insertionAddress from the free node list.
    removeNode(insertionAddress);

    // Record the size of the block.
    *((size_t *) insertionAddress) = realSize;

    // Return a pointer just past the size indicator.
    return (void *)(((size_t *)insertionAddress) + 1);
}


int main(int argc, char *argv[]) {
    // Test with some strings.

    char* str1 = "Lorem ipsum dolor sit amet.";
    char* str2 = "Mauris pellentesque finibus leo ac.";
    char* fry2 = "Mauris finibus pellentesque ac leo.";
    char* str3 = "Vivamus eget ultrices lacus, vitae.";

    char *test1 = mmalloc(strlen(str1) + 1);
    char *test2 = mmalloc(strlen(str2) + 1);
    char *test3 = mmalloc(strlen(str3) + 1);
    strcpy(test1, str1);
    strcpy(test2, str2);
    strcpy(test3, str3);

    char *oldTest2 = test2;
    mfree(test2);
    test2 = mmalloc(strlen(fry2) + 1);
    strcpy(test2, fry2);

    if (oldTest2 != test2)
        errExit("mmalloc did not take lowest free block");

    if (0 != strcmp(str1, test1))
        errExit("test1 was corruped");

    if (0 != strcmp(fry2, test2))
        errExit("test2 was corruped");

    if (0 != strcmp(str3, test3))
        errExit("test3 was corrupted");
    
    mfree(test2);
    mfree(test3);
    mfree(test1);
}

