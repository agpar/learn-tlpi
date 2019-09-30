# Memory Allocation

Anytime you need dynamic amounts of memory at run time, you need dynamic memory allocation.

## Manipulating the program break

Resizing the heap is as simple as changing the pointer to the end of the heap (the *program break*). By incrementing the address of the program break, any memory between the new break and the old becomes eligible for new writing.

You can manipulate the program break with the following calls

```
#include <unistd.h>

int brk(void *end_data_segment);
void *sbrk(intptr_t increment);
```

`brk` is a system call which sets the program break to the address passed in as an argument. It returns 0 on success and -1 on error.

`sbrk` is a library function that increments the break by the amount passed in and returns a `void *` to the *previous* location of the break.

## Using malloc and free

In practice, people tend to use `malloc` and `free` rather than directly modifying the program break. These functions offer a useful level of abstraction, allowing small blocks of memory to be allocated quickly and for memory to be recycled.

`void *malloc(size_t size)` attempts to allocate `size` bytes of memory on the heap, and returns a pointer to it if successful. `NULL` is returned if not. Pointers are aligned on a byte boundary. It's important to remember that `malloc` can fail, even if it nearly never does.

`void free(void *ptr)` frees the memory pointed to by `ptr`. This does not always modify the program break - 'freed' regions are stored in a list to be re-used later. This reduces the number of system calls actually performed, and allows memory in-between other used regions to be recycled.

Of course all memory allocated while a process is running is de-allocated when the process concludes. In many cases it is OK not to `free` things (small programs that run quickly), however there are advantages to freeing frequently even if it seems unnecessary.

### How it works

When you `malloc` and are returned `ptr`, slightly more memory than you requested is actually allocated. The memory at `ptr-1` actually contains the *length* of the block of memory requested. When `free(ptr)` is later called, the length can be accessed and used to mark the entire region of memory as free. 

When `free(ptr)` is called, the locations of the previous block of freed memory and the next one are written at `ptr`. So the list of freed memory is actually stored in the free memory itself. 

Taken together, it's obvious why accidentally writing bytes somewhere you shouldn't can so massively break things. The programmer is sharing memory with the memory management system.

It also makes it clear why `free(ptr)` should only be called on a pointer returned from malloc. Otherwise whatever garbage at lies at `ptr-1` will be interpreted as the length of the block to free!

## Other heap allocators

The following two functions are also included in `stdlib.h`

```
void *calloc(size_t numitems, size_t size)
void *realloc(void *ptr, size_t size)
```

`calloc` is useful for allocating an array all at once. It also initializes the returned memory to `0`, which may or may not be desired.

`realloc` re-sizes the memory pointed to by `ptr` to the new size. This may involve changing the address of `ptr`.

## Runtime stack allocation

It is also possible to allocate memory on the stack at runtime using `void * alloca(size_t size)`. This is slightly nice as you don't have to worry about freeing it, but is generally discouraged and is not as portable as `malloc`.