# memoryFunctions
My own C implementations of the heap allocation functions of malloc, calloc, free and realloc. It uses linked lists to track blocks of occupied and freed up memory.
To allocate the memory, I use the sbrk() functions, see man page https://linux.die.net/man/2/sbrk.
