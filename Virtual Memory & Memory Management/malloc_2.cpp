#include  <unistd.h>
#include <stdio.h>
#include <string.h>
//#include <iostream>


struct MallocMetadata {
    //the size without meta data
    size_t size; //this is unsigned int , so it cant be negative
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
};


MallocMetadata* freel = NULL; // list of the free space we can use

size_t numb= 0;           //num_free_blocks
size_t numby = 0 ;         //num_free_bytes
size_t nallocatedb = 0;   //num_allocated_blocks
size_t nallocatedby = 0;   //num_allocated_bytes
size_t nmeta_data_b = 0;  //num_meta_data_bytes

size_t _size_meta_data();


void* smalloc(size_t size)
{
    if(size == 0 || size > 100000000){
        return NULL;
    }
    MallocMetadata* curr_ptr=freel;
    
    //the second while takes care of everything , so this first while is useless 5lat
    // //try to find block with exactly same size 5lat
    while (curr_ptr){
        /* code */
        if(size <= curr_ptr->size && curr_ptr->is_free == true)
        {
            curr_ptr->is_free = false;
            numb--;
            //numby-=min_ptr->size;
            numby = numby - curr_ptr->size;
            curr_ptr++;
            return (void*) curr_ptr ;
        }
        curr_ptr=curr_ptr->next;
    }

    //try to find minimum block , if there is a free block with enough size
    //MallocMetadata* curr_ptr=freel;
    // MallocMetadata* min_ptr = NULL;
    // size_t min_size = 0;
    // while (curr_ptr){
    //     /* code */
    //     if(curr_ptr->size >= size && curr_ptr->is_free == true)
    //     {
    //         if(min_ptr == NULL){
    //             min_ptr = curr_ptr;
    //             min_size = curr_ptr->size;
    //         }
    //         if(curr_ptr->size < min_size){
    //             min_size = curr_ptr->size;
    //             min_ptr = curr_ptr;
    //         }
    //         curr_ptr=curr_ptr->next;
    //     }
    // }
    // if(min_ptr != NULL)
    // {
    //     min_ptr->is_free = false;
    //     numb--;
    //     //numby-=min_ptr->size;
    //     numby = numby - min_ptr->size;
    //     min_ptr++;
    //     return (void*)min_ptr ;
    // }


    /// if there is no enough space
    void* ptr= sbrk(size + sizeof(MallocMetadata));
    if(ptr == (void*)(-1)){
        return NULL;
    }
    nallocatedb++;
    nallocatedby+=size;
    nmeta_data_b+=sizeof(MallocMetadata);

    MallocMetadata* new_block =  (MallocMetadata*) ptr;
    new_block->size =size;
    new_block->is_free = false;
    new_block->next = NULL;
    /// if the list is empty
    if (!freel){
        /* code */
        freel=new_block;
        //new_block->next=NULL;
        new_block->prev=NULL;
    }
    else{
        curr_ptr=freel;
        while (curr_ptr->next){
            /* code */
            curr_ptr=curr_ptr->next;
        }
        new_block->prev=curr_ptr;
        curr_ptr->next=new_block;
        //new_block->next=NULL;
        
    }
    return (void*)(++new_block);

    
    
}
void* scalloc(size_t num, size_t size)
{
    if(size == 0 || num == 0){
        return NULL;
    }
    if(size*num > 100000000){
        return NULL;
    }
    //try to find minimum block , if there is a free block with enough size
    // MallocMetadata* curr_ptr=freel;
    // MallocMetadata* min_ptr = NULL;
    // size_t min_size = 0;
    // while (curr_ptr){
    //     /* code */
    //     if(curr_ptr->size >= size*num && curr_ptr->is_free == true)
    //     {
    //         if(min_ptr == NULL){
    //             min_ptr = curr_ptr;
    //             min_size = curr_ptr->size;
    //         }
    //         if(curr_ptr->size < min_size){
    //             min_size = curr_ptr->size;
    //             min_ptr = curr_ptr;
    //         }
    //     }
    //     curr_ptr=curr_ptr->next;
    // }
    // if(min_ptr != NULL){
    //     min_ptr->is_free = false;
    //     numb--;
    //     //numby-=min_ptr->size;
    //     numby = numby - min_ptr->size;
    //     min_ptr++;
    //     memset(min_ptr, 0 ,size*num );

    //     return (void*)min_ptr ;
    // }
    MallocMetadata* curr_ptr=freel;
    while (curr_ptr){
        /* code */
        if(size*num <= curr_ptr->size && curr_ptr->is_free == true)
        {
            curr_ptr->is_free = false;
            numb--;
            //numby-=min_ptr->size;
            numby = numby - curr_ptr->size;
            curr_ptr++;
            memset(curr_ptr, 0 ,size*num );
            return (void*) curr_ptr ;
        }
        curr_ptr=curr_ptr->next;
    }

    void* ptr= sbrk(size*num + sizeof(MallocMetadata));
    if(ptr == (void*)(-1)){
        return NULL;
    }
    nallocatedb++;
    nallocatedby+=size*num;
    nmeta_data_b+= sizeof(MallocMetadata);

    MallocMetadata* new_block =  (MallocMetadata*) ptr;
    new_block->size =size*num;
    new_block->is_free = false;
    new_block->next = NULL;
    /// if the list is empty
    if (!freel){
        /* code */
        freel=new_block;
        //new_block->next=NULL;
        new_block->prev=NULL;
    }
    else{
        curr_ptr=freel;
        while (curr_ptr->next){
            /* code */
            curr_ptr=curr_ptr->next;
        }
        new_block->prev=curr_ptr;
        curr_ptr->next=new_block;
        //new_block->next=NULL;   
    }
    new_block++;
    memset(new_block , 0 , num*size);
    return (void*)(new_block);

}

void sfree(void* p)
{
    if(!p){
        return;
    }
    MallocMetadata* ptr = ((MallocMetadata*)p - 1);
    if(ptr->is_free == true){
        return;
    }
    ptr->is_free = true;
    numb++;
    numby += ptr->size;
}
void* srealloc(void* oldp, size_t size)
{
    if( oldp == NULL){
        return smalloc(size);
    }
    if(size == 0 || size > 100000000){
        return NULL;
    }
    MallocMetadata* rptr = ((MallocMetadata*)oldp - 1);
    if(size <= rptr->size){
        return oldp;
    }
    
    MallocMetadata* curr_ptr=freel;
    while (curr_ptr){
        /* code */
        if(size <= curr_ptr->size && curr_ptr->is_free == true)
        {
            curr_ptr->is_free = false;
            numb--;
            //numby-=min_ptr->size;
            numby = numby - curr_ptr->size;
            curr_ptr++;
            memmove(curr_ptr,oldp,rptr->size);//rptr size and not size
            sfree(oldp);
            return (void*) curr_ptr ;
        }
        curr_ptr=curr_ptr->next;
    }
    void* ptr= sbrk(size + sizeof(MallocMetadata));
    if(ptr == (void*)(-1)){
        return NULL;
    }
    //numb++;
    //numby+=size;
    nallocatedb++;
    nallocatedby+=size;
    nmeta_data_b+= sizeof(MallocMetadata);

    MallocMetadata* new_block =  (MallocMetadata*) ptr;
    new_block->size =size;
    new_block->is_free = false;
    new_block->next = NULL;
    curr_ptr=freel;
    while (curr_ptr->next){
        /* code */
        curr_ptr=curr_ptr->next;
    }
    new_block->prev=curr_ptr;
    curr_ptr->next=new_block;
    new_block++;
    memmove(new_block,oldp,rptr->size);//rptr size and not size
    sfree(oldp);
    //new_block->next=NULL;
    return (void*)(new_block);


    
}
size_t _num_free_blocks(){

    return numb;
}

size_t _num_free_bytes(){
    return numby;
}

size_t _num_allocated_blocks(){
    return nallocatedb;
}

size_t _num_allocated_bytes(){

    return nallocatedby;
}

size_t _num_meta_data_bytes(){

    return nmeta_data_b;
}

size_t _size_meta_data(){
    return sizeof(MallocMetadata);

}
// int main() {
//     // Print a string to the console
//     void *base = sbrk(0);
//     void*a= smalloc(1);
//     void*b= smalloc(1);
//     void *after = sbrk(0);
//     sfree(a);
//     //std::cout << "Hello, world!" << std::endl;
//     return 0;
// }