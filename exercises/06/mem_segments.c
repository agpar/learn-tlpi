#include <stdio.h>
#include <stdlib.h>

/* Exercise 6-1
 *
 * This exercise intends to show that uninitialized data segment
 * is not included in the binary of the file. The compiled size
 * of this file is far lower than the 10MB needed to store the
 * contents of mbuf in main. This is because only the size is
 * needed for uninitialzed data to be allocated at run time.
 */

char globBuf[65536];            // Uninitialized data segment
int primes[] = {2,3,5,7};       // Initialized data segment

static int square(int x) {      // Allocated in frame for square()
    int result = x * x;         // Allocated in frame for square()
    return result;              // Return passed via register
}

static void doCalc(int val) {   // Allocated in frame for doCalc()
    printf("The square of %d is %d\n", val, square(val));
    if (val < 1000) {
        int t;
        t = val * val * val;    // Allocated in from for doCalc()
        printf("The cube of %d is %d\n", val, t);
    }
}

int main() {
    static int key = 9973;      // Initialized data segment
    static char mbuf[10240000]; // Uninitialized data segment
    char *p;                    // Allocated in frame for main()

    p = malloc(1024);           // Point to memory in heap

    doCalc(key);

    exit(EXIT_SUCCESS);
}
