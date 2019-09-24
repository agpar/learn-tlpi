# Processes

This chapter describes processes, with a description of virtual memory.

## Processes and Programs

A process is a running program, an entity defined by the kernel to which system resources are allocated.

A program is a file containing type identification, instructions, program entry point address, data, symbol and relocation tables, shared library and dynamic linking information, and other information.

The kernel maintains a number of data structures in order to support the program.

## PID and PPID

Each running process is assigned a positive unique integer, its process ID (PID). The kernel can also return the parent processes PID, the PPID. The following calls always succeed:

```
pid_t getpid(void)
pid_t getppid(void)
```

Most Linux systems can accommodate approximately 32k processes.

The special `init` process gets pid 1, and it is at the root of the process-parent tree. If a process is orphaned, `init` becomes its new parent.

## Memory layout of a process

The memory allocated to a process is split into a number of *segments*.

* Text segment: Read only, machine-language instructions. Can be shared.
* Initialized data segment: Explicitly initialized global and static variables.
* Uninitialized data segment: Room for uninitialized global and static variables. Zeroed out before execution starts. This is a distinct segment, as, unlike the initialized data segment, no values here are loaded from the binary.
* The stack: A dynamically growing and shrinking segment for stack frames.
* The heap: An area where memory can be dynamically allocated at run time.

## Virtual memory management

The kernel maps the addresses a process sees to hidden actual addresses in memory. This is done to ensure safety, allow memory sharing, allow programs to execute with only parts of their memory actually loaded, and exploit *temporal* and *spatial* locality (the fact that sequential program execution and loops means that the same addresses and ones close to them are repeatedly accessed).

The kernel divides RAM into page frames and process address spaces into pages (about 4kb in modern systems) and manages a mapping between them. 

A process does not know which pages are actually loaded - if one is requested that is not, a *page fault* occurs, where the kernel suspends the process and loads its requested page.

The kernel uses a *page table* for each process, which maps pages in the processes virtual address space to a page-frame in ram (or indicates that the page is not loaded). Not all pages need to be mapped, as many areas are unused. The set of valid pages can be changed with stack or heap growth and memory mapping.

Advantages of this situation:

* Processes are by default isolated from each other and the kernel.
* Processes can share memory if appropriate. This happens automatically when you load two copies of a program (read only machine instructions are shared), or can be done via a system call.
* Read/write/execute permissions can be assigned to address ranges.
* Programmers don't need to be concerned with physical ram layout. Each process gets the illusion that it has a huge, uninterrupted memory block.
* A processes virtual memory space can outgrow ram and be mapped on to disk transparently.

## Stack and stack frames

The stack grows linearly with each function call. The *stack pointer* register points to the top of the stack.

A user stack frame contains all function-local variables and copies of the program counter and stack pointer registers - all the information necessary to pick up where you left off when returning to that frame.

## Command line arguments

In a typical c program, `main` is passed `int argc` and `char *argv[]`. The last entry of, `argv[argc]` is `NULL`, so you could `while` loop over `argv` even without knowing its size.

There are other (non portable) ways of accessing these arguments, such as having a process read the `/proc/self/cmdline` file.

## Environment List

Each process has access to an environment map which maps strings to strings. This map is a copy inherited from the processes parent. This copy allows a one-way once-only communication from parent to child.

In most shells, a value can be added to the environment using the `export` command and removed with the `unset` command. Without using the `export` command, Bourne-style shells will only save the value in a shell-local environment and it will not be passed to the kernel-managed process environment (and hence children will not see it).

In a c program, the environment list is stored in global `char **environ`, which has a similar structure to `argv`.

You can also get a reference to the environment as a third argument to main, like: `int main(int argc, char *argv[], char *envp[])`.

The library function `char *getenv(const char *name)` returns the *value* of the environment entry referenced by `name`, or it returns NULL.

The library function `int putenv(char *string)`, where string is of the form `"name=value"`, puts a new entry into the processes environment. Note, the value of the string is NOT copied, so you must use a static variable and not overwrite it. `int setenv(const char *name, const char *value, int overwrite)` is a nicer interface, where strings are actually copied before writing to the environment.

The `int clearenv(void)` library function really only does `environ = NULL`. Note this can lead to memory leaks (see `setenv`), but in practice this is usually not an issue as you would typically only clear the environment once (when a process starts).

## Long-jumping

The standard C `goto` command can only move within a function. It is *possible* to jump between functions, but highly difficult to do in a safe and clear way.

```
#include <setjmp.h>

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val)
```

`setjmp` returns 0 on first call and saves the program counter and stack pointer into `env`. Note, `env` is usually made global, as otherwise it needs to be passed down the stack. 

When `longjmp(env, val)` is called, execution jumps to the call of `setjmp` where `env` was last set and returns `val`. Thus, the call to `setjmp` is often called as the condition on a switch statement.

```
switch (setjmp(env)) {
    case 0:
        // Jump point has just been set, program can continue
        break;
    case 1:
        // We just jumped here from a call to longjmp(env, 1)
        break;
    ...
}
```

It is highly difficult to use this correctly as you must always ensure that the stack frame where `setjmp` was last called is still open when you `longjmp`, or else the program will fail in a spectacular way. The compiler offers no way to make this guarantee for you. Further, any stack frames between the jump call and target are simply deleted, potentially disrupting function invariants and preventing clean up of resources. Optimizing compilers have difficulty with this kind of stack-abuse, and any variables that you depend on for your logic will need to be declared `volatile`. 

The SUSv3 and C99 standards outline a small set of cases where is appropriate to call `setjmp`.

At best, this seems like it *could* be used to emulate exception-handling and generator like behavior in C, but comes with many caveats.