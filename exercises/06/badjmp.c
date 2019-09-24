#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>


/* Excercise 6-2.
 *
 * Illustrating effects of jumping into an already returned function.
 * In my case, the "jump" into the memory of the returned function 
 * happens to succeed, but the program then gets "lost" and exits with
 * a failure.
 */

jmp_buf env;

int badSet() {
    switch(setjmp(env)) {
        case 0:
            return 0;
            break;
        case -1:
            printf("setjmp failed");
            exit(EXIT_FAILURE);
            break;
        case 1:
            printf("Jumped back into a returned function!\n");
            return 1;
    }
    return 0;
}

int main() {
    int result = badSet();
    printf("Jumping into a function that has already returned\n");
    longjmp(env, 1);
    printf("Made it back to main!");
    exit(EXIT_SUCCESS);
}

