#include  <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#define MAX_ORDER 11
////TODELEET////////
#include <iostream>
#include <vector>


struct MallocMetadata {
    //the size without meta data
    size_t size; //this is unsigned int , so it cant be negative
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
};

MallocMetadata* freel = NULL; // list of the free space we can use
MallocMetadata* mmapl = NULL; // list of the mmap space we can use
MallocMetadata* orders[MAX_ORDER];
MallocMetadata* global_used_list = NULL;


size_t numb= 0;           //num_free_blocks
size_t numby = 0 ;         //num_free_bytes
size_t nallocatedb = 0;   //num_allocated_blocks
size_t nallocatedby = 0;   //num_allocated_bytes
size_t nmeta_data_b = 0;  //num_meta_data_bytes
int first=1; //is it first alloc
int real_malloc=1;

size_t _size_meta_data();

//helper funs
void AddToOrders(int i,MallocMetadata* second_half)
{
    MallocMetadata* temp = orders[i];
    if(temp == NULL) {
        orders[i] = second_half;
        second_half->prev = NULL;
        second_half->next = NULL;

        return;
    }
    while(temp!= NULL) {

        if( temp >second_half)
        {
            if(temp->prev == NULL) {
                //temp->prev->next = second_half;
                 
                orders[i]=second_half;
                second_half->next=temp;
                second_half->prev=NULL;
                temp->prev=second_half;
                return;
                
            }
            second_half->prev = temp->prev;
            second_half->next = temp;
            temp->prev = second_half;
            second_half->prev->next=second_half;
            // if(temp == orders[i]) {
            //     orders[i] = second_half;
            // }
            return;
        }
        temp = temp->next;
    }
    MallocMetadata* head = orders[i];
     while (head->next)
     {
        head=head->next;
     }
     head->next=second_half;
     second_half->next=NULL;
     second_half->prev=head;

}
void cutBlock(size_t size, MallocMetadata* found, int i)
{

    if( ((found->size )> (128- sizeof(MallocMetadata) )&& (found->size + sizeof(MallocMetadata)) / 2 < ( size + sizeof(MallocMetadata) ))|| i == 0) {
        found->is_free = false;
        return;
    }
    i--;
    MallocMetadata* second_half =  (MallocMetadata*)((char*)found + (size_t)((found->size + sizeof(MallocMetadata))/2));
    second_half->size = (found->size + sizeof(MallocMetadata))/2 - sizeof(MallocMetadata);
    found->size = (found->size + sizeof(MallocMetadata))/2 - sizeof(MallocMetadata);
    second_half->is_free= true;
    numb++;
    nallocatedb++;
    nallocatedby-=sizeof(MallocMetadata);
    numby = numby - sizeof(MallocMetadata);
    nmeta_data_b+=sizeof(MallocMetadata);

    AddToOrders(i,second_half);
    cutBlock(size,found,i);

}
// void cutBlockons(size_t size, MallocMetadata* found, int i)
// {

//     if( ((found->size )> (128- sizeof(MallocMetadata) )&& (found->size + sizeof(MallocMetadata)) / 2 <= ( size + sizeof(MallocMetadata) ))|| i == -1) {
//         found->is_free = false;
//         return;
//     }
//     i--;
//     MallocMetadata* second_half =  (MallocMetadata*)((char*)found + (size_t)((found->size + sizeof(MallocMetadata))/2));
//     second_half->size = (found->size + sizeof(MallocMetadata))/2 - sizeof(MallocMetadata);
//     found->size = (found->size + sizeof(MallocMetadata))/2 - sizeof(MallocMetadata);
//     second_half->is_free= true;
//     numb++;
//     nallocatedb++;
//     numby = numby - sizeof(MallocMetadata);
//     nmeta_data_b+=sizeof(MallocMetadata);

//     AddToOrders(i,second_half);
//     cutBlock(size,found,i);

// }


void* smalloc(size_t size)
{
    if(first){
        for(int i = 0; i<MAX_ORDER ; i++){
            orders[i] = NULL;
        }
        //alligment and allocating the 32 block
        void *pbreak = sbrk(0);
        size_t pb = (size_t) pbreak % (1024 * 128 * 32);
        sbrk((128*1024*32) - pb);
        void *blocks = sbrk(128 * 1024 * 32);
        if (blocks == (void *) (-1)) {
            return NULL;
        }
        nallocatedb+=32;
        nallocatedby=(32*1024*128) - sizeof(MallocMetadata) * 32;
        numb += 32;
        numby += (32*1024*128) - sizeof(MallocMetadata) * 32;
        nmeta_data_b+=sizeof(MallocMetadata)*32;
        /// in the array insert the 32 block
        MallocMetadata* previous_block = (MallocMetadata *) blocks;
        for (int i = 0; i <= 31; i++) {
            MallocMetadata *curr_block = (MallocMetadata *) blocks;
            if (i == 0) {
                orders[10] = curr_block;
                //curr_block->cookies = (long)global_random_value;
                curr_block->next = NULL;
                curr_block->prev = NULL;
                curr_block->size = (1024 * 128) - sizeof(MallocMetadata);
                curr_block->is_free = true;

            } else {
                //curr_block->cookies = (long )global_random_value;
                curr_block->next = NULL;
                curr_block->prev = previous_block;
                curr_block->size = (1024 * 128) - sizeof(MallocMetadata);
                curr_block->is_free = true;/////
                previous_block->next = curr_block;
                previous_block = (MallocMetadata*)((char*)previous_block + 128*1024);
            }
            blocks = ((char*)blocks + 128*1024);
        }
        first=0;
    }
    if(size == 0 || size > 100000000){
        return NULL;
    }

   
     ////large alloccations
    if (size>=128*1024){
        /* code */
        void* new_m= mmap(NULL, sizeof(MallocMetadata) + size,
                            PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (new_m==MAP_FAILED){
            /* code */
            return NULL;
        }
        //????????????????????????????
        nallocatedb++;
        nallocatedby+=size;
        nmeta_data_b+=sizeof(MallocMetadata);
        // numb++;
        // numby+=size;
        //???????????????????????????
        MallocMetadata* new_block=(MallocMetadata*) new_m;
        new_block->size=size;
        new_block->next=NULL;
        /// if the list is empty
        if(!mmapl){
            mmapl=new_block;
            new_block->prev=NULL;
        }
        else{
            MallocMetadata* curr_ptr=mmapl;
            curr_ptr=mmapl;
        while (curr_ptr->next){
            /* code */
            curr_ptr=curr_ptr->next;
        }
        new_block->prev=curr_ptr;
        curr_ptr->next=new_block;
        }
        return (char*) new_block+sizeof(MallocMetadata);

    }
    // int power_of_two;
    // /// get the closest power of two to our size including md
    // int new_size =ceil(((size + sizeof(MallocMetadata) )/128) + 1 ) ;
    // if(new_size == 0){
    //     power_of_two = 0;
    // }
    // else{
    //     power_of_two = log2(new_size);
    // }
    // if(power_of_two > 10){
    //     return NULL;
    // }

    //small allocations
    // check 
    MallocMetadata* found=NULL;
    int i;
       for( i=0; i<MAX_ORDER;i++)
        {
            if(orders[i] != NULL) {
                if(orders[i]->size >= size ){

                    orders[i]->is_free = false;
                    found=orders[i];
                    orders[i] = orders[i]->next;
                    if(orders[i] != NULL) {
                        orders[i]->prev = NULL;
                    }

                    break;
                 }
            } 
        } 
        if(i==11|| !found) return NULL;

        cutBlock(size,found, i);
        //update data like free bytes etc
        numb--;
        numby = numby - found->size;
        //nmeta_data_b+=found->size;
        
        //nallocatedb++;
        //nallocatedby+=
        //remove orders[i] from list
        /////////add tzo the used list
    if(global_used_list ==  NULL)
    {
       
        global_used_list = found;
        found->next = NULL;
        found->prev =NULL;
        //  found->size_without_meta = size;

        //global_used_list->next = NULL;
        //global_used_list->prev =NULL;

    }
    else{
        ///get to the tail of the used list
        MallocMetadata* temp =  global_used_list;
        while(temp->next != NULL)
        {
            
            temp = temp->next;
        }
       temp->next = found;
       found->prev = temp;
       found->next = NULL;
    }
    
    return (char*)found + sizeof(MallocMetadata);
    //now if size is less than 128kb , but there is not free blocks of required size , na3mal ae 
       
}
MallocMetadata* buddy;

//merge a block , if flg=-1 will merge the block up to the maximum level
//if flg = 0 , will merge untill ptr->size > size then will return
void* mergeBlock(MallocMetadata* the_block, size_t size, int flg,int *order_num)
{
    //MallocMetadata* ptr = the_block;
    // MallocMetadata* buddy =( (MallocMetadata*) ((size_t)ptr ^ (ptr->size+sizeof(MallocMetadata))) ); //xor between address and size , to get the buddy
    // if(buddy == NULL || the_block == NULL)
    //     return;
    //MallocMetadata* tmp = orders[0];
    while(*order_num< MAX_ORDER-1) //we do not want to merge order 10 blocks
    {
         buddy =( (MallocMetadata*) ((size_t)the_block ^ (the_block->size+sizeof(MallocMetadata))) ); //xor between address and size , to get the buddy
        // if(buddy == NULL || the_block == NULL) return;
        // if(buddy<the_block){
        //     MallocMetadata *tem=the_block;
        //     the_block=buddy;
        //     buddy=tem;

        // }
        MallocMetadata* current_list_in_order = orders[*order_num];
        while(current_list_in_order != NULL)
        {
            ///if we found the buddy
            if(current_list_in_order == buddy)
            {
                
                ///if the buddy is the first block
                if(current_list_in_order->prev == NULL)
                {
                    orders[*order_num] = current_list_in_order->next;
                    if(current_list_in_order->next != NULL)
                    {
                        current_list_in_order->next->prev = NULL;
                    }
                }
                else
                {
                   
                    MallocMetadata* before_temp= current_list_in_order->prev;
                    before_temp->next = current_list_in_order->next;
                    if(current_list_in_order->next != NULL)
                    {
                        current_list_in_order->next->prev = NULL;
                    }

                }
            if(buddy<the_block){
                MallocMetadata *tem=the_block;
                the_block=buddy;

                buddy=tem;


            }
                the_block->size = ((the_block->size+sizeof(MallocMetadata))*2) - sizeof(MallocMetadata);
                numb--;
                nallocatedb--;
                nallocatedby+=sizeof(MallocMetadata);
                numby += sizeof(MallocMetadata);
                nmeta_data_b-=sizeof(MallocMetadata);
                break;

            }
            else{
                current_list_in_order = current_list_in_order->next;
            }
        }
        if(current_list_in_order != NULL)
        {
            (*order_num)++;
        }
        else {
            // break;
            return the_block;
        }

        
    }
    //     MallocMetadata* buddy =( (MallocMetadata*) ((size_t)ptr ^ ptr->size) ); //xor between address and size , to get the buddy
    //     if(buddy == NULL || the_block == NULL) return;

    //     if(buddy->is_free == true && buddy->size == the_block->size)
    //     {
    //         //merge between two blocks
    //         numb--;
    //         numby = numby + sizeof(MallocMetadata);
    //         ptr->size = ptr->size * 2 + sizeof(MallocMetadata);
    //         if(buddy->next != NULL)
    //         {
    //             buddy->next->prev = buddy->prev;
    //         }
    //         if(buddy->prev != NULL)
    //         {
    //             buddy->prev->next = buddy->next;
    //         }
            

    //         nmeta_data_b -= sizeof(MallocMetadata);
    //         nallocatedb--;
    //         nallocatedby += sizeof(MallocMetadata);
    //         //remove buddy from orders  
    //         for(int i=0; i<MAX_ORDER - 1;i++) //not merge buddies of size MAX_ORDER
    //         {
    //             tmp = orders[i];
    //             while(tmp != NULL)
    //             {
    //                 if(tmp == buddy)//found buddy
    //                 {
    //                     //remove tmp from order , this one left
    //                     if(tmp->prev != NULL && tmp->next != NULL)
    //                     {
    //                         tmp->prev->next = tmp->next;
    //                         tmp->next->prev = tmp->prev;
    //                         break;
    //                     }
    //                     else if(tmp->prev != NULL && tmp->next == NULL)
    //                     {
    //                         tmp->prev->next = tmp->next;
    //                         break;
    //                     }
    //                     else if(tmp->prev == NULL && tmp->next != NULL)
    //                     {
    //                         tmp->next->prev = tmp->prev;
    //                         orders[i] = tmp->next;
    //                         break;
    //                     }
    //                     else if(tmp->prev == NULL && tmp->next == NULL)
    //                     {
    //                         orders[i] = tmp->next;
    //                         break;
    //                     }
    //                     //tmp = NULL;
    //                     //buddy =( (MallocMetadata*) ((size_t)ptr ^ the_block->size) ); //xor between address and size , to get the buddy
    //                 }
    //                 else
    //                 {
    //                     tmp = tmp->next;
    //                 }
    //             }
           
    //         }
    //         buddy =( (MallocMetadata*) ((size_t)ptr ^ ptr->size) ); //xor between address and size , to get the buddy
    
    //     }
    //     else
    //     {
    //         break;
    //     }
    // }
    //     if(flg == 0 && ptr->size > size)
    //     {
    //         return;
    //     }
    //     buddy =( (MallocMetadata*) ((size_t)ptr ^ ptr->size) ); //xor between address and size , to get the buddy
      return the_block;  
}




void* scalloc(size_t num, size_t size)
{
    if(size*num == 0 || size*num > 100000000){
        return NULL;
    }

    real_malloc=0;
    void* calloc_block = smalloc(num * size);
    real_malloc=1;
    if(calloc_block == NULL) {
        return NULL;
    }
    memset(calloc_block, 0, num*size);
    return calloc_block;
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
    size_t to_free_size = ptr->size+ sizeof(MallocMetadata);
    int without_128 = to_free_size / 128;
    int order_num;
    if(without_128 == 0)
    {
        order_num = 0;
    }
    else{
        order_num = log2(without_128);
    }
     if(ptr->size >= 1024 * 128 )
    {
        if(mmapl ==NULL)
        {
            return;/////should not get here
        }
        ///IF THE POINTER TO DELETE IS THE FIRST IN THE MMAP LIST
        if(mmapl == ptr)
        {
            mmapl = mmapl->next;
            ptr->is_free = true;
            ptr->next = NULL;
            ptr->prev = NULL;
            if(mmapl != NULL)
            {
                mmapl->prev = NULL;
            }
            nallocatedb--;
            nallocatedby-=ptr->size;
            nmeta_data_b-=sizeof(MallocMetadata);

            munmap(ptr, sizeof(MallocMetadata) + ptr->size);
           

            return;

        }
        ///here we search for the block in the mmap blocks

        MallocMetadata* temp_mmap = mmapl;
        while(temp_mmap->next != NULL)
        {
            
            if((char*)temp_mmap->next == (char*)ptr)
            {
                break;
            }
            temp_mmap = temp_mmap->next;
        }
        
        temp_mmap->next = temp_mmap->next->next;
        if(temp_mmap->next !=NULL){
            temp_mmap->next->prev = temp_mmap;
        }
        nallocatedb--;
        nallocatedby-=ptr->size;
        nmeta_data_b-=sizeof(MallocMetadata);

        munmap(ptr, sizeof(MallocMetadata) + ptr->size);
        return;
    }
    /// now we remove the block from the used list
    MallocMetadata* temp_used = global_used_list;

    ///if it was the first one in the used list
    if(temp_used == ptr){
        global_used_list = temp_used->next;
        if(temp_used->next != NULL)
        {
            temp_used->next->prev =NULL;
        }
        ptr->is_free = true;
        ptr->next = NULL;
        ptr->prev = NULL;
    }
    else{
        while(temp_used->next != NULL)
    {
        if(temp_used->next == ptr)
        {
            temp_used = temp_used->next;
            break;
        }
        temp_used = temp_used->next;
    }

    MallocMetadata* prev_used = temp_used->prev;
    if(prev_used!=NULL){
        prev_used->next= temp_used->next;
        if(temp_used->next != NULL)
        {
            temp_used->next->prev = prev_used;
        }
    }

    ptr->next = NULL;
    ptr->prev =NULL;
    }

    numb++;
    numby += ptr->size;
    ptr=(MallocMetadata*)mergeBlock(ptr, to_free_size, -1,&order_num);
    /// after we merge the blocks we insert the block to the current order
    MallocMetadata* head_to_insert = orders[order_num];
    if(head_to_insert == NULL)
    {
        orders[order_num] = ptr;
        ptr ->next = NULL;
        ptr->prev = NULL;
        ptr->is_free = true;
        return;
    }
    while(head_to_insert != NULL)
    {
        if(head_to_insert > ptr)
        {
            if(head_to_insert->prev == NULL)
            {
                orders[order_num] = ptr;
                ptr->next = head_to_insert;
                head_to_insert->prev = ptr;
                ptr->prev=NULL;
                return;

            }
            MallocMetadata* previos = head_to_insert->prev;
            previos->next = ptr;
            ptr->prev = previos;
            ptr->next = head_to_insert;
            head_to_insert->prev = ptr;
            return;
        }
        head_to_insert = head_to_insert->next;
    }
     MallocMetadata* head = orders[order_num];
     while (head->next)
     {
        head=head->next;
     }
     head->next=ptr;
     ptr->next=NULL;
     ptr->prev=head;
     
    return;



}

bool canBeMerged(void* oldp, size_t size)
{
     MallocMetadata* rptr = ((MallocMetadata*)oldp - 1);
     size_t merged_size = rptr->size;
     MallocMetadata* buddy = ( (MallocMetadata*) ((size_t)rptr ^( merged_size+sizeof(MallocMetadata))) );
     
    if(buddy != NULL)
    {
        if (!buddy->is_free){
            return false;
        }
        // if(buddy->size == rptr->size)
        // {
        //     merged_size = merged_size *  2 + sizeof(MallocMetadata);
        //     if(merged_size > size)
        //     {
        //         return true;
        //     }
        //     buddy = ( (MallocMetadata*) ((size_t)rptr ^ merged_size) );
        // }
        merged_size = merged_size *  2 + sizeof(MallocMetadata);
        if(merged_size > size){
            return true;
        }
       
        return false;
        
    }
    return false;
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


    if(size >= 128 * 1024){
        void* new_m= mmap(NULL, sizeof(MallocMetadata) + size,
                            PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (new_m==MAP_FAILED){
            /* code */
            return NULL;
        }
        //????????????????????????????
        nallocatedb++;
        nallocatedby+=size;
        nmeta_data_b+=sizeof(MallocMetadata);
        //???????????????????????????
        MallocMetadata* new_block=(MallocMetadata*) new_m;
        new_block->size=size;
        new_block->next=NULL;
        /// if the list is empty
        if(!mmapl){
            mmapl=new_block;
            new_block->prev=NULL;
        }
        else{
            MallocMetadata* curr_ptr=mmapl;
            curr_ptr=mmapl;
        while (!curr_ptr->next){
            /* code */
            curr_ptr=curr_ptr->next;
        }
        new_block->prev=curr_ptr;
        curr_ptr->next=new_block;
        }
        memmove(new_block,oldp,rptr->size);//rptr size and not size
        sfree(oldp);
        return (char*) new_block+sizeof(MallocMetadata);
    }

    // check if by merging we can get enough size
    //bool can_merge = canBeMerged(oldp, size);
    //size_t merge_size = rptr->size;

//     //????????????????????????
    //merge
size_t to_free_size = rptr->size+ sizeof(MallocMetadata);
    int without_128 = to_free_size / 128;
    int order_num;
    if(without_128 == 0)
    {
        order_num = 0;
    }
    else{
        order_num = log2(without_128);
    }
    rptr=(MallocMetadata*)mergeBlock(rptr, rptr->size, 0,&order_num);
    int power_of_two;
    /// get the closest power of two to our size including md
    int new_size =ceil(((rptr->size + sizeof(MallocMetadata) )/128) + 1 ) ;
    if(new_size == 0){
        power_of_two = 0;
    }
    else{
        power_of_two = log2(new_size);
    }
    if(rptr->size>=size){
        cutBlock(size,rptr,power_of_two);
        numby=numby-rptr->size+to_free_size-sizeof(MallocMetadata);
        return oldp;
    }
    // if megging iy there not bkfe msa7a feo
     cutBlock(to_free_size,rptr,power_of_two);


    // if(can_merge)
    //     {
    //     mergeBlock(rptr, rptr->size, 0,&order_num);
    //     return rptr;
    //     }
//     //????????????????????????
    

//     MallocMetadata* curr_ptr=orders[0];
//     for(int i=0; i<MAX_ORDER;i++){
//         while (curr_ptr){
//             /* code */
//             if(size <= curr_ptr->size && curr_ptr->is_free == true)
//             {
//                 curr_ptr->is_free = false;
//                 numb--;
//                 //numby-=min_ptr->size;
//                 numby = numby - curr_ptr->size;
//                 curr_ptr++;
//                 memmove(curr_ptr,oldp,rptr->size);//rptr size and not size
//                 sfree(oldp);
//                 return (void*) curr_ptr ;
//             }
            
//             curr_ptr=curr_ptr->next;
//         }
//     }
//     //void* ptr= sbrk(size + sizeof(MallocMetadata));


 // Find block if available
 void *newp = smalloc(size);
    if (newp == NULL)
        return NULL;
    memmove(newp, oldp, rptr->size);
    sfree(oldp);
    return newp;
   

//     // MallocMetadata* new_block =  (MallocMetadata*) ptr;
//     // new_block->size =size;
//     // new_block->is_free = false;
//     // new_block->next = NULL;
//     // curr_ptr=freel;
//     // while (!curr_ptr->next){
//     //     /* code */
//     //     curr_ptr=curr_ptr->next;
//     // }
//     // new_block->prev=curr_ptr;
//     // curr_ptr->next=new_block;
//     // new_block++;
//     // memmove(new_block,oldp,rptr->size);//rptr size and not size
//     // sfree(oldp);
//     // //new_block->next=NULL;
//     return (void*)(new_block);

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
///debug funs:

// int main(){
//     void *ptr1 = smalloc(64);
//     void* ptr2 = scalloc(1, 128 * pow(2, 1) - 64);
//     ptr1 = srealloc(ptr1, 128 * pow(2, 3) - 64);
//     //REQUIRE((end - start) == 128 * pow(2, 1));
// }
//240 
// int main(){
//     //void* ptr = scalloc(5, sizeof(int));
//     std::vector<void*> allocations;
//     void* ptr;
//     int i=1;

//     for (; i < 64; i++)
//     {
//          ptr = smalloc(128*1024+100);
//         allocations.push_back(ptr);
//     }
//     i--;
//     while (!allocations.empty())
//     //for(int j=0; j < 1; j++)
//     {
//          ptr = allocations.back();
//         allocations.pop_back();
//         sfree(ptr);
//         i--;
        
//     }


// }
//malloc3 test basic 190

// int main() {
//     void* ptr;
//     std::vector<void*> allocations;
//     // for(int i=0;i<64;i++){
//     //      ptr = smalloc(128 * pow(2, 9) - 64);
//     //      allocations.push_back(ptr);
//     // }
//     //  //ptr = smalloc(128 * pow(2, 9) - 64);
//     // // if(smalloc(40) == NULL){
//     // //     sfree(nullptr);
//     // // }
//     // while (!allocations.empty())
//     // {
//     //     void* ptr = allocations.back();
//     //     allocations.pop_back();
//     //     sfree(ptr);
//     // }
//     for(int i=0;i<64;i++){
//          ptr = smalloc(128 * pow(2, 9) - 64);
//          allocations.push_back(ptr);
//     }
//      //ptr = smalloc(128 * pow(2, 9) - 64);
//     // if(smalloc(40) == NULL){
//     //     sfree(nullptr);
//     // }
//     while (!allocations.empty())
//     {
//          void* ptr = allocations.front();
//         allocations.erase(allocations.begin());
//         sfree(ptr);
//     }


//     return 0;
// }


//malloc3 test basic 352 
// int main() {
//         void* ptr = smalloc(40);
//           void* ptr2 = srealloc(ptr, 128*pow(2,2) -64);

//         //void* ptr2 = srealloc(ptr, 128*4 -64);
//         void* ptr3 = srealloc(ptr2, 100);
//         void* ptr4 = srealloc(ptr3, 128*pow(2,8) -64);
//         sfree(ptr4);
//     return 0;
// }
// int main() {
//         void* ptr = smalloc(100000001);
//         sfree(ptr);

//     return 0;
// }
// int main() {
//     // // Print a string to the console
//     // void *base = sbrk(0);
//     // void*a= smalloc(100);
//     // //void*b= smalloc(100);
//     // void *after = sbrk(0);
//     // sfree(a);
//     //std::cout << "Hello, world!" << std::endl;
//     // void* ptr = smalloc(128*pow(2,10) -64);
//      // Allocate a small block
//     void* ptr1 = smalloc(4000000);//42
//         void* ptr2 = smalloc(4000000);//42

//     // void* ptr2 = srealloc(ptr1, 60);
//     // void* ptr3 = srealloc(ptr2, 30);
//     sfree(ptr2);
//     sfree(ptr1);
//     // Reallocate to a larger size
//     //void* ptr2 = srealloc(ptr1, 128*pow(2,2) -64); //40
//     //int* newArr = static_cast<int*>(ptr2);
//     return 0;
// }
// int main(){
//     void * ptr;
//     ptr = smalloc(128*pow(2,0) -64);
    
//     sfree(ptr);
    
//     ptr = smalloc(128*pow(2,1) -64);
    
//     sfree(ptr);

//     // ptr = smalloc(128*pow(2,2) -64);
//     // verify_block_by_order(0,0,0,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,31,0,0,0);
//     // sfree(ptr);
//     // verify_block_by_order(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0);
//     // ptr = smalloc(128*pow(2,3) -64);
//     // verify_block_by_order(0,0,0,0,0,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,31,0,0,0);
//     // sfree(ptr);
//     // verify_block_by_order(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0);
//     // ptr = smalloc(128*pow(2,4) -64);
//     // verify_block_by_order(0,0,0,0,0,0,0,0,1,1,1,0,1,0,1,0,1,0,1,0,31,0,0,0);
//     // sfree(ptr);
//     // verify_block_by_order(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0);
//     // ptr = smalloc(128*pow(2,5) -64);
//     // verify_block_by_order(0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,0,1,0,1,0,31,0,0,0);
//     // sfree(ptr);
//     // verify_block_by_order(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0);
//     // ptr = smalloc(128*pow(2,6) -64);
//     // verify_block_by_order(0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,0,1,0,31,0,0,0);
//     // sfree(ptr);
  
//     ptr = smalloc(128*pow(2,7) -64);
    
//     sfree(ptr);
 
//     ptr = smalloc(128*pow(2,8) -64);
 
//     sfree(ptr);
   
//     ptr = smalloc(128*pow(2,9) -64);
    
//     sfree(ptr);
  
//     ptr = smalloc(128*pow(2,10) -64);
    
//     sfree(ptr);
    
//     return 0; 
// }