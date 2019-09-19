#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

/* Exercise 5-4
 *
 * Reimplemtation of dup and dup2 using fcntl. 
 * main creates test files to test output.
 */

int mdup(int oldFd) {
    if (-1 == fcntl(oldFd, F_GETFL)) {
        return -1;
    }
    int newFd;
    if (-1 == (newFd = fcntl(oldFd, F_DUPFD)))
        return -1;
    return newFd;
}

int mdup2(int oldFd, int newFd) {
    // Fail if oldFd is not open.
    if (-1 == fcntl(oldFd, F_GETFL)) {
        return -1;
    }

    if (oldFd == newFd)
        return newFd;

    // Check if newFile is open.
    int newFdOpen;
    if (-1 == fcntl(oldFd, F_GETFL)) {
        if (errno == EBADF) {
            newFdOpen = 0;
        } else {
            errExit("fcntl");
        }
    }

    // Close newFile if it is open.
    if (newFdOpen) {
        if (-1 == close(newFd))
            errExit("close");
    }

    // Duplicate newfile to old file.
    int newnewFd = mdup(oldFd);
    if (newnewFd != newFd)
        errExit("Failed to assign new file to %d", newFd);

    return newFd;
}

int main(int argc, char *argv[]) {
    int openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    mode_t mode = S_IRUSR | S_IWUSR;
    int fd1, fd_dup, fd2;

    // Test mdup fails if oldFd is not open.
    fd1 = 5;
    fd_dup = mdup(fd1);
    if (errno != EBADF)
        errExit("mdup set errno incorrectly");

    // Test mdup works in happy path
    if (-1 == (fd1 = open("mdupTest", openFlags, mode)))
        errExit("open");
    fd_dup = mdup(fd1);

    char *test1 = "test with main fd\n";
    char *test2 = "test with dup fd\n";
    if (-1 == write(fd1, test1, strlen(test1)))
        errExit("write");
    if (-1 == write(fd_dup, test2, strlen(test2)))
        errExit("write");
    
    // Test the mdup2 works in happy path
    if (-1 == (fd2 = open("mdup2Test", openFlags, mode)))
        errExit("open");
    fd_dup = mdup2(fd2, fd_dup);

    if (-1 == write(fd2, test1, strlen(test1)))
        errExit("write");
    if (-1 == write(fd_dup, test2, strlen(test2)))
        errExit("write");
}
