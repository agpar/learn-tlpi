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
                usageErr("Unknown opt!");
        }
    return optind;
}

int *open_files(char **fileNames, int fileCount) {
    int *fds = malloc(fileCount);
    int openFlags = O_CREAT | O_WRONLY;
    
    if (!OPT_APPEND)
        openFlags = openFlags | O_TRUNC;

    mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH; /* rw-rw-rw- */

    int fd;
    for (int i = 0; i < fileCount; i++) {
        if((fd = open(fileNames[i], openFlags, filePerms)) == -1)
            errExit("open %s", fileNames[i]);
        if(OPT_APPEND)
            lseek(fd, 0, SEEK_END);
        fds[i] = fd;
    }
    return fds;
}

int main(int argc, char* argv[]) {
    int i;
    ssize_t numRead;
    char buf[BUF_SIZE];

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [out-file]...", argv[0]);

    // Open all the files given as input
    int fileInd = parse_opt(argc, argv);
    char **fileNames = &argv[fileInd];
    int *fds = open_files(fileNames, argc - fileInd);

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
