#include "tlpi_hdr.h"
#include <fcntl.h>

/* Exercise 5-5
 * 
 * Confirm that duped file descriptors share an offset.
 */

char template[] = "/tmp/dup_test_XXXXXX";
int SEEK_TARGET = 100;

int main(int argc, char * argv[]) {

    int fd1, fd2;
    if (-1 == (fd1 = mkstemp(template))) {
        errExit("mkstemp");
    }
    fd2 = dup(fd1);

    printf("Running tests...\n");

    // Assert that seeking in fd1 also seeks in fd2.
    if (-1 == lseek(fd1, SEEK_TARGET, SEEK_SET))
        errExit("lseek");
    if (lseek(fd2, 0, SEEK_CUR) != SEEK_TARGET) {
        printf("fd1 and fd2 do NOT seek together!!");
        exit(1);
    }

    // Assert flags are equal.
    int flags1, flags2;
    if (-1 == (flags1 = fcntl(fd1, F_GETFL)))
        errExit("fcntl");
    if (-1 == (flags2 = fcntl(fd2, F_GETFL)))
        errExit("fcntl");
    if(flags1 != flags2) {
        printf("fd1 and fd2 do NOT share open flags!");
        exit(1);
    }

    printf("Success!");

    unlink(template);
    if (-1 == close(fd1))
        errExit("close");
    if (-1 == close(fd2))
        errExit("close");
}
        
