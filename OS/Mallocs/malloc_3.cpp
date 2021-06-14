#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <sys/mman.h>

#define MAX_SIZE 1e8
#define META_SIZE sizeof(MMD)
#define FREEWILDERNESS(X) (X->next == NULL && X->is_free == true)


struct MallocMetadata{
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
};

typedef MallocMetadata MMD;
static MMD* mmetadata = NULL;

static MMD* mapmetadata = NULL;

size_t _num_free_blocks()
{
    size_t count = 0;
    MMD* temp = mmetadata;
    while(temp != NULL) {
        if (temp->is_free)
            count ++ ;
        temp = temp->next;

    }
    return count;
}

size_t _num_free_byte()
{
    size_t count = 0;
    MMD* temp = mmetadata;
    while(temp != NULL) {
        if(temp->is_free)
            count += temp->size;
        temp = temp->next;

    }
    return count;
}

size_t  _num_allocated_blocks()
{
    size_t count = 0;
    MMD* temp = mmetadata;
    while(temp != NULL) {
        count++;
        temp = temp->next;

    }
    temp = mapmetadata;
    while(temp != NULL) {
        count++;
        temp = temp->next;

    }
    return count;
}

size_t  _num_allocated_bytes()
{
    size_t count = 0;
    MMD* temp = mmetadata;
    while(temp != NULL) {
        count += temp->size;
        temp = temp->next;

    }
    temp = mapmetadata;
    while(temp != NULL) {
        count+= temp->size;
        temp = temp->next;
    }
    return count;
}

size_t _size_meta_data()
{
    return META_SIZE;
}

size_t _num_meta_data_bytes()
{
    _num_allocated_blocks()*_size_meta_data();
}

MMD* get_MMD_ptr(void* p)
{
    return (MMD*)p - 1;
}

MMD* find_free(MMD ** last, size_t size)
{
    MMD* curr;
    if(size < 128*1024)
        curr = mmetadata;
    else
        curr = mapmetadata;

    while (curr && !(curr->is_free && curr->size >= size))
    {
        if(FREEWILDERNESS(curr))
        {
            if(size < 128*1024)
            {
                void *request = sbrk(size - curr->size);
                if (request == (void *) -1)
                    return NULL;
            }
            else
            {
                void* request = mmap(0,size - curr->size,PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
                if(request == (void*) MAP_FAILED)
                    return NULL;
            }
            curr->size = size;
            *last = curr;
            return curr;
        }
        *last = curr;
        curr = curr->next;
    }
    if(curr == NULL)
        return curr;
    if(curr->size - size - META_SIZE >= 128) //split if large enough
    {
        int size2 = curr->size - size - META_SIZE;
        MMD* block = (MMD*)((uintptr_t ((void*)(curr+1))) + size);
        block->size = size2;
        block->is_free = true;
        block->next = curr->next;
        block->prev = curr;
        curr->next=block;
        curr->size = size;
    }
    return curr;
}

MMD* allocate_MMD(MMD* last, size_t size) {
    MMD *block;
    block = (MMD *) sbrk(0);
    if (size < 128 * 1024)
    {
        void *request = sbrk(size + META_SIZE);
        if (request == (void *) -1)
            return NULL;
    }
    else
    {
        void* request = mmap(0,size + META_SIZE,PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
        if(request == (void*) MAP_FAILED)
            return NULL;
    }
    if(last) {
        last->next = block;
        block->prev = last;
    }
    else block->prev = NULL;
    block->size=size;
    block->next = NULL;
    block->is_free = false;
    return block;

}



void* sbrkmalloc(size_t size)
{
    MMD* block;
    if(!mmetadata)
    {
        block = allocate_MMD(NULL,size);
        if(!block)
            return NULL;
        mmetadata = block;
    }
    else
    {
        MMD* last = mmetadata;
        block = find_free(&last,size);
        if(!block) {
            block = allocate_MMD(last, size);
            if (!block)
                return NULL;
        }
        else
        {
            block->is_free = false;
        }
    }
    return (block+1);
}


void* mmapmalloc(size_t size)
{
    MMD* block;
    if(!mapmetadata)
    {
        block = allocate_MMD(NULL,size);
        if(!block)
            return NULL;
        mapmetadata = block;
    }
    else
    {
        MMD* last = mapmetadata;
        block = find_free(&last,size);
        if(!block) {
            block = allocate_MMD(last, size);
            if (!block)
                return NULL;
        }
        else
        {
            block->is_free = false;
        }
    }
    return (block+1);
}


void* smalloc(size_t size)
{
    if(size == 0 || size > MAX_SIZE)
        return NULL;
    if(size < 128*1024)
        return sbrkmalloc(size);
    else
        return mmapmalloc(size);
}


void ifFree_join (MMD* block)
{
    MMD* next = block->next;
    if(next != NULL && next->is_free)
    {
        block->size += (next->size + META_SIZE);
        block->next = next->next;
    }
    MMD* prev = block->prev;
    if(prev != NULL && prev->is_free)
    {
        prev->size += (block->size + META_SIZE);
        prev->next = block->next;
    }
}

void sfree(void* p)
{
    if (p == NULL)
        return;
    MMD* block = get_MMD_ptr(p);
    block->is_free = true;
    if(block->size >= 128*1024)
        munmap(block,block->size + META_SIZE);
    else ifFree_join(block);
}


void* scalloc(size_t num, size_t size)
{
    if(size == 0 || num*size > MAX_SIZE)
        return NULL;
    void* p = smalloc(size*num);
    if(!p)
        return NULL;
    memset(p,0,size*num);
    return p;
}

void* srealloc(void* oldp, size_t size)
{
    if( size == 0 || size > MAX_SIZE)
        return NULL;

    if(!oldp)
        return smalloc(size);

    MMD* block =  get_MMD_ptr(oldp);
    block->is_free = true;
    if(FREEWILDERNESS(block) && block->size < size)
    {
        if(size < 128*1024)
        {
            void *request = sbrk(size - block->size);
            if (!(request == (void *) -1))
            {
                block->size = size;
            }
        }
    }
    block->is_free = false;
    if(block->size >= size)
        return oldp;
    void* np;
    np = smalloc(size);
    if(!np)
        return NULL;
    memcpy(np,oldp,size);
    sfree(oldp);
    return np;
}


