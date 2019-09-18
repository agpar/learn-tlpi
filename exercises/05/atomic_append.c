#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

/* Exercise 5-3
 *
 * Demonstration of why atomic append is necessary.
 * When command is run with the final 'x' argument specified, files
 * are not opened in append more and are subject to race conditions
 * when seeking to the end of files, the lseek and write in the for 
 * loop can be interupted by another process.
 */

int USE_APPEND = 1;

int main(int argc, char *argv[]) {
    if (argc < 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s {filename} {num-bytes} [x]", argv[0]);

    if (argc == 4) {
        if (strcmp(argv[3], "x") == 0) {
            printf("Append mode turned off!");
            USE_APPEND = 0;
        } else {
            usageErr("Unknown argument '%s'", argv[3]);
        }
    }

    int fd;
    int openFlags = O_CREAT | O_WRONLY;
    if (USE_APPEND)
        openFlags = openFlags | O_APPEND;
    if (-1 == (fd = open(argv[1], openFlags, S_IRUSR | S_IWUSR)))
        errExit("open");

    int numBytes = atoll(argv[2]);
    char buf[] = {'a'};
    for (int i = 0; i < numBytes; i++) {
        if (-1 == lseek(fd, 0, SEEK_END))
            errExit("lseek");
        if (-1 == write(fd, buf, 1))
            errExit("write");
    }
    
    exit(EXIT_SUCCESS);
}
