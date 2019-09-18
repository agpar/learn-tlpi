#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"


/* Exercise 5-2
 *
 * If a file is oppened with O_APPEND, seeking to the start of the file
 * before writing does nothing, as EVERY write will be preceded by a
 * lseek(fd, 0, SEEK_END) transparently.
 */

int main(int argc, char *argv[]) {
    
    if(argc != 3 || strcmp(argv[0], "--help") == 0)
        usageErr("%s {file} {string}", argv[0]);

    int fd;
    if(-1 == (fd = open(argv[1], O_WRONLY | O_EXCL |O_APPEND, 
                                 S_IRUSR | S_IWUSR)))
        errExit("open");

    if(-1 == lseek(fd, 0, SEEK_SET))
        errExit("lseek");

    if(-1 == write(fd, argv[2], strlen(argv[2])))
        errExit("write");

    exit(EXIT_SUCCESS);
}
