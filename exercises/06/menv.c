#include <string.h>
#include <stdlib.h>
#include "tlpi_hdr.h"

/* Excercie 6-3
 *
 * Implement setenv and unsetenv using getenv and putenv.
 * Here I try to be slightly fancy and truncate the 
 * environment at the last entry - may be buggy.
 */


extern char **environ;

int msetenv(char *name, char *value, int overwrite) {
    if(name == NULL || *name == '\0' || index(name, '=') != NULL) {
        errno = EINVAL;
        return -1;
    }

    if (getenv(name) != NULL && !overwrite)
        return 0;

    char *newStr;
    int nameLen = strlen(name);
    int valueLen = strlen(value);
    int newLen = nameLen + valueLen + 2;
    if(NULL == (newStr = malloc(newLen)))
        return -1;

    strncpy(newStr, name, nameLen);
    newStr[nameLen] = '=';
    strncpy((newStr + nameLen + 1), value, valueLen + 1);
    putenv(newStr);
    return 0;
}

void removeEmptyStrings(char **environ) {
    char **ep1, **ep2; ep1 = ep2 = environ;
    char *swap;
    while(*ep1 != NULL) {
        if(**ep1 != '\0') {
            ep1++; ep2 = max(ep1, ep2);
        } else {
            while(*ep2 != NULL && **ep2 == '\0') {
                ep2++;
            }
            if (*ep2 == NULL) {
                *ep1 = NULL;
                
                /* Not sure whether or not to attempt to free memory,
                 * as some strings in environ may be static?

                ep1++;
                while(*ep1 != NULL) {
                    free(*ep1);
                    ep1++
                }
                */
                return;
            } else {
                swap = *ep1;
                *ep1 = *ep2;
                *ep2 = swap;
                **ep2 = '\0';
                ep1++;
            }
        }
    }
}

int munsetenv(char *name) {
    if(name == NULL || *name == '\0' || index(name, '=') != NULL) {
        errno = EINVAL;
        return -1;
    }
    int nameLen = strlen(name);
    char **ep = environ;
    while(*ep != NULL) {
        if (strncmp(*ep, name, nameLen) == 0)
            if (*(*ep + nameLen) == '=')
                **ep = '\0';
        ep++;
    }

    removeEmptyStrings(environ);
    return 0;
}

int main(int argc, char *argv[]) {
    msetenv("hello", "bonjour", 1);
    msetenv("good", "goodday", 1);
    munsetenv("hello");
    return 0; 
}
