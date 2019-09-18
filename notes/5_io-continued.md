# IO Continued

There are a number of other concerns in IO, including atomicity, file open flags, the kernel data structures representing file descriptors, and other ways of interacting with files that the kernel provides.

## Atomicity

All system calls are guaranteed atomic - only one system call can be in progress at a time. Since race conditions are common in IO, it is important to consider how to ensure your program operations can be thread safe.

The `O_EXCL` flag, passed to `open`, causes the call to fail if the file already exists. Since system calls are atomic, this is a useful tool.

The `O_APPEND` flag, passed to `open` guarantees that all writes are appended to the end of a file. Since file writing is atomic, this effectively resolves the need for any further synchronization between processes (so long as they all use `O_APPEND` and the file system supports atomic appends - most do). `O_APPEND` basically works by forcing every subsequent `write` call to the file in question to come bundled with a `lseek(fd, 0, SEEK_END)` which executes before but in the same atomic step as the `write`.

## fcntl

`int fctnl(int fd, int cmd, ...)` provides numerous ways to interact with already open files.

The access flags of a file (set by `open`) can be retrieved using `int flags = fcntl(fd, F_GETFL)`. 

Some flags can be modified using `fctnl(fd, F_SETFL, flags)`. This is useful in cases where the file was opened by another process (for instance, stdin) and when working with pipes and sockets.

## File descriptors and files

It is possible to have multiple descriptors pointing at a single file. It is helpful to overview how the kernel represents file descriptors.

* **File descriptor tables**: Each PROCESS gets a file descriptor table, which holds flags and a pointer to the corresponding entry in the open file table.
* **Open file table**: A SYSTEM-WIDE table containing information about every open file, including the access flags and mode set by `open`, the current file offset, and a reference to the i-node of the file.
* **I-node table**: Maintained by the file system, used as an interface between the directory hierarchy and blocks on disk.

The relationships between all tables are many-to-one, meaning there can be many process-specific file descriptors pointing to the same open file, and many open files pointing to the same i-node entry.

Note that this data-layout implies that file-offset is not necessarily process-specific.

## Duplicating file descriptors

The command `./script > results 2>&1` works by setting file descriptors 1 and 2 to point to the same entry in the open file table, thus `stdout` and `stderr` both point to `results` and share a single offset.

The same effect can be done with the `int dup(int oldFd)` system call. It returns the index of a new entry in the process's file descriptor table that points to the same entry in the system wide open file table as `oldFd`. The int returned is the next free file descriptor.

The system call `int dup2(int oldFd, int newFd)` can be used to specify *which* file descriptor to use as a duplicate of `oldFd`. For example, `dup2(1, 2)` redirects `stderr` to `stdout`, closing the old file pointed to by `stdout` if necessary. This is another case where the atomicity of system calls is important, as trying to implement `dup2` as separate calls to `close` and `dup` would be subject to races.

A `dup3` system call exists, allowing one to set the `O_CLOSEXEC` flag while duping.

## IO at offset
System calls `pread` and `pwrite` mirror the `read` and `write` system calls, with the addition that an `offset` is supplied in the arguments. The read/write is then performed at the given offset *without* updating the file's system-wide offset. This is equivalent to atomically saving the current offset, changing to the specific offset, performing the read/write, then resetting the offset.

This is particularly useful in a threading context, as all threads share a single process-wide file-descriptor table.

## Scattered (vectorized) IO

Two system calls can be used to IO with a ordered series of buffers.

```
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
```

The value returned is the number of bytes read/written. `iovcnt` is the length of `*iov`, which is an array of structs with the following shape:

```
struct iovec {
    void *iov_base;   // the buffer to be filled
    size_t iov_len;   // the size (in bytes) of the buffer
}
```

Essentially, these can be thought of as performing a series of reads/writes in order into/from the buffers of the `iovecs` in `*iov`. The buffers are filled/emptied in order, and each is completely filled/emptied before passing on to the next. 

These are useful when reading in structured data directly from a binary dump, or when you have many buffers to write all at once and require speed and atomicity of the write (the alternative is to concat them to a single buffer yourself).

Note, `preadv` and `pwritev` calls also exist, which are analogous to `pwrite` and `pread`.

## Truncating

The system calls
```
int truncate(const char *pathname, off_t length);
int ftruncate(int fd, off_t length);
```

Can be used to truncate a file to the given length in bytes. Note, `ftruncate` requires an open file descriptor, but does *not* change the file offset.

## Nonblocking IO

The `O_NONBLOCK` flag can be used in an `open` to set a file to non-blocking mode. This causes the `open` and all subsequent reads/writes to return an error if accessing the file would require blocking. 

This flag is used for terminals, pipes and sockets, and is usually ignored for regular files (kernel buffering makes all operations on data-files appear non blocking).

## Large files
A separate API exists for opening large files on 32 bit systems. This can be enabled by simply defining `#define _FILE_OFFSET_BITS 64` before any headers, which replaces all calls to `open` to calls to `open64`. It's important to use the correct types (`off_t` instead of int) to ensure that the replacement happens correctly!

This appears to be largely irrelevant in modern systems?

## `/dev/fd`

The kernel provides a representation of a processes file descriptor table in `/dev/fd`. So `open("/dev/fd/1", O_WRONLY) == dup(1)`.

This is mostly useful in the shell, as some commands expect files as input and don't read from piped in data. e.g. `ls | diff /def/fd/1 oldFiles` will compare the results of `ls` to `oldFiles` (as the result of `ls` is going to stdout).

Aliases for 0,1,2 also exist at `/dev/stdin`, `/dev/stdout`, `/dev/stderr`.

