# 3 System Programming Concepts


System calls allow user-space applications to request the kernel to perform privileged tasks.

All system calls have overhead, since the CPU must switch modes and transfer data between user-space and kernel-space.

The C standard library provides a lingua-franca wrapper over linux system calls.

Most system calls return an int, where a negative values indicates failure, which it is the programmers job to check.

For portability, clarity and flexibility, native c-types are often redefined for specific purposes. e.g. `typedef int pid_t` indicates that process id's are ints.
