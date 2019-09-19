#include <fcntl.h>
#include <sys/uio.h>
#include "tlpi_hdr.h"

/* Exercise 5-7
 *
 * A test implementation of readv and writev using the
 * same iovec struct.
 */

int my_readv(int fd, const struct iovec *iov, int iovcnt) {
    if (iovcnt < 0) {
        errno = EINVAL;
        return -1;
    }

    int numRead, numReadTotal = 0;
    for(int i = 0; i < iovcnt; i++) {
       if (-1 == (numRead = read(fd, iov[i].iov_base, iov[i].iov_len)))
           return -1;

       if (numRead < iov[i].iov_len) {
           return 0;
       }

       numReadTotal += numRead;
    }

    return numReadTotal;
}

int my_writev(int fd, const struct iovec *iov, int iovcnt) {
    if (iovcnt < 0) {
        errno = EINVAL;
        return -1;
    }

    int numWrote, numWroteTotal = 0;
    for(int i = 0; i < iovcnt; i++) {
       if (-1 == (numWrote = write(fd, iov[i].iov_base, iov[i].iov_len)))
           return -1;

       numWroteTotal += numWrote;
    }

    return numWroteTotal;
};


char template[] = "/tmp/scatter_test_XXXXXX";
int main(int argc, char *argv[]) {
    // Test a read.
    struct iovec iov[2];
    char buf1[3] = "12\0";
    char buf2[3] = "34\0";
    iov[0].iov_base = &buf1;
    iov[0].iov_len = 2;
    iov[1].iov_base = &buf2;
    iov[1].iov_len = 2;

    printf("Testing...\n");

    int fd;
    if (-1 == (fd = mkstemp(template))) {
        errExit("mkstemp");
    }

    //Test Writing
    int numWrote = my_writev(fd, (const struct iovec *)&iov, 2);
    if (numWrote != 4) {
        fatal("Failed to write 4 bytes");
    }

    // Reset for read test.
    lseek(fd, 0, SEEK_SET);
    memcpy(buf1, "\0\0\0", 3);
    memcpy(buf2, "\0\0\0", 3);
    
    
    // Test reading
    int numRead = my_readv(fd, (const struct iovec *)&iov, 2);
    if (numRead != 4) {
        fatal("Failed to read 4 bytes, read %d bytes", numRead);
    }
    if(strcmp(buf1, "12") != 0) {
        fatal("Failed to read into buf1!");
    }
    if(strcmp(buf2, "34") != 0) {
        fatal("Failed to read into buf2!");
    }

    printf("Success!");
    unlink(template);
    exit(EXIT_SUCCESS);

}

