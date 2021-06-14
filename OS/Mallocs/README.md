malloc implementations using C++.

-----------------------------
malloc_1 : naïve malloc using only sbrk - no free.

----------------------------------
malloc_2 : simple malloc using sbrk with meta data -> include calloc, malloc, realloc and free. without split and merge blocks

---------------------------------
malloc_3 : simple malloc using sbrk and mmap (for bigger allocations) with meta data -> include calloc, malloc, realloc and free. including split and merge blocks.

------------------------------
malloc_4: same as malloc 3 but with alignment (for 64bit systems). as in all block sizes are a multiple of 8.
