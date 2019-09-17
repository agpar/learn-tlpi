#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <tlpi_hdr.h>
#include <unistd.h>

/* Implements behaviour similar to tee */ 

int BUF_SIZE = 1024;
int OPT_APPEND = 0;

int parse_opt(int argc, char *argv[]) {
    char c;
    while ((c = getopt(argc, argv, "a")) != -1)
        switch (c) {
            case 'a':
                OPT_APPEND = 1;
                break;
            default:
                usageErr("Unknown opt %c", c);
        }
    return optind;
}

// TODO break this up into small procedures.
//
int main(int argc, char* argv[]) {
    int *fds;
    int fd, i, openFlags;
    mode_t filePerms;
    ssize_t numRead;
    char buf[BUF_SIZE];

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [out-file]...", argv[0]);

    int fileInd = parse_opt(argc, argv);
    char **fileNames = &argv[fileInd];

    // Open all the files given as input.
    fds = malloc(sizeof(int *) * (argc -1));
    openFlags = O_CREAT | O_WRONLY;
    if (!OPT_APPEND)
        openFlags = openFlags | O_TRUNC;

    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH; /* rw-rw-rw- */

    for (i = 0; i < argc - fileInd; i++) {
        if((fd = open(fileNames[i], openFlags, filePerms)) == -1)
            errExit("open %s", fileNames[i]);
        if(OPT_APPEND)
            lseek(fd, 0, SEEK_END);
        fds[i] = fd;
    }

    while((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        for (i = 0; i < argc - fileInd; i++) {
            if (write(fds[i], buf, numRead) == -1)
                errExit("write %s", fileNames[i]);
        }
        if(write(STDOUT_FILENO, buf, numRead) == -1)
            errExit("write stdout");
    }

    for (i = 0; i < argc - fileInd; i++)
        close(fds[i]);
}
