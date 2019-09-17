# File IO

The files API is the common API for reading/writing to any kind of device in Unix. All I/O is done through this interface (even to sockets, pipes, terminals).

All system calls on files refer to a file decsriptor (int). One the kernel opens a file for a process, it returns an int. The process can then use that int to refer to the opened file.

Most programs expect that they will have three files opened for them automatically on startup: [0, 1, 2], referring to stdin, stout and stderr.

## Four key system calls

Four key system calls for file I/O.

1. `fd = open(pathname, flags, mode)`.
	* Request the file at `pathname` be opened and a file descriptor returned. `flags` controls whether the file is opened read/write/append, and `mode` can be used to set permissoins if a new file is being created.
2. `numread = read(fd, buffer, count)`.
	* Read up to `count` bytes from open file referenced by `fd` into `buffer`. The number of bytes actually read is returned.
3. `numwritten = write(fd, buffer, count)`.
	* Write up to `count` bytes from `buffer` to `fd`. Returns the number of byets actually written.
4. `status = close(fd)`.
	* Closes the file.

These four functions apply in all I/O cases.

Notes: 

* `read` is for genirically reading bytes (not specifically text), so do not expect the buffer it fills to be null terminated.
* `write` may be buffered by the kernel, so a succesful call does NOT guarantee that information has persisted to disk.
* `close` should be used explicitly and checked for errors. For instance, closing a file descriptor twice is a bad bug (and you need to check for it).


## Changing offset

For each open file, the kernel stores an offset count, indicating which byte of the file will be written to next. For a new or truncated file, this starts at 0. This offset is updated with each call to `read` and `write`, but can also be manually updated:

```
off_t lseek(int fd, off_t offset, int whence)
```

Where `offset` is the number of bytes relative to `whence`, where `whence` is either `SEEK_SET` (first byte of file), `SEEK_CUR` (current offset) or `SEEK_END` (byte after end of file)

Thus `lseek(fd, 0, SEEK_SET)` resets to the start of file, and `lseek(fd, -1, SEEK_END`) moves to the last byte of the file, and `lseek(fd, 0, SEEK_CUR)` returns the current offset without changing it.

Notes:

* `lseek` has nothing to do with the underlying hard disk, it simply updates the kernels record of how far into the file to start further operations.
* `lseek` can obviously not be applied to pipes or streams.
* It IS possible to `lseek` past the end of a file and write. This creates a 'hole' in the file, which is perfectly fine (remember, not 'seeking' into specific parts of the disk, but simply relative to the file's start - and files are obviously allowed to grow).

## ioctl

There is a special and very generic functoin `ioctl` which does everything that does not fit into the 5 operators above.