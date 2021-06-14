#include <unistd.h>
#include <cstring>
#define MAX_SIZE 1e8
#define META_SIZE sizeof(MMD)


struct MallocMetadata{
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
};

typedef MallocMetadata MMD;
static MMD* mmetadata = NULL;


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
     MMD* curr = mmetadata;
     while (curr && !(curr->is_free && curr->size >= size))
     {
         *last = curr;
         curr = curr->next;
     }
     return curr;
}

MMD* allocate_MMD(MMD* last, size_t size)
{
     MMD* block;
     block = (MMD*)sbrk(0);
     void* request = sbrk(size + META_SIZE);
     if(request == (void*) -1)
         return NULL;
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

void* smalloc(size_t size)
{
     if(size == 0 || size > MAX_SIZE)
         return NULL;
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



void sfree(void* p)
{
     if (p == NULL)
         return;
     MMD* block = get_MMD_ptr(p);
     block->is_free = true;
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

