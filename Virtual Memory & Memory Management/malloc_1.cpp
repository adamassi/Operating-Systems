#include  <unistd.h>
#include  <stdio.h>

void* smalloc(size_t size){
    if (size==0||size>100000000){
        return NULL;
    }
    void* ptr= sbrk((intptr_t)size);
    if(ptr == (void*)(-1)){
        return NULL;
    }
    return ptr;
    
}