#include <unistd.h>
#define MAX_SIZE 1e8

void* smalloc(size_t size)
{
    if(size == 0  or size > MAX_SIZE)
        return NULL;
    void* allocated = sbrk(size);
    if (allocated == (void*)(-1))
        return NULL;
    else return allocated;

}
