//
// Created by 98max on 10/20/2022.
//

#include <memorymanagement.h>

using namespace maxOS;
using namespace maxOS::common;

MemoryManager* MemoryManager::activeMemoryManager = 0;

MemoryManager::MemoryManager(common::size_t start, common::size_t size) {

    activeMemoryManager = this;

    //Prevent wirting outside the area that is allowed to write
    if(size < sizeof(MemoryChunk)){

        this -> first = 0;

    }else{

        this -> first = (MemoryChunk*)start;
        first -> allocated = false;
        first -> prev = 0;
        first -> next = 0;
        first -> size = size - sizeof(MemoryChunk);

    }



}

MemoryManager::~MemoryManager() {
    if(activeMemoryManager == this){
        activeMemoryManager = 0;
    }
}

void* MemoryManager::malloc(common::size_t size) {

    MemoryChunk* result = 0;

    //Common way of iterating through a linked list
    for (MemoryChunk* chunk = first; chunk != 0 && result == 0; chunk = chunk->next) { //Iterate through the list of chunks


        if(chunk -> size > size && !chunk -> allocated){    //If the chunk is big enough and not being used
            result = chunk;
        }

    }

    if(result == 0){
        return 0;
    }

    //The result must have been set to a memory chunk because it did not return

    if(result -> size < size + sizeof(MemoryChunk) + 1){    //Check if there is space for the new data and another memory chunk (for splitting). The + 1 is there so that the split chunk will actually have space for data, not just the header

        //If there isn't space to split then there will just be a few bytes left over, not a problem
        result -> allocated = true;

    }else{
        //If there is space then split the chunk

        MemoryChunk* temp = (MemoryChunk*)((size_t)result + sizeof(MemoryChunk) + size); //Add size memory chunk (header) and size of data to get to get the position of the next chunk

        temp -> allocated = false;                                          //Just created so wont be allocated
        temp -> size =  result->size - size - sizeof(MemoryChunk);          //Get old size of the free chunk, take away the requested data and the header size
        temp -> prev = result;                                              //The previous is now the newly allocated chunk
        temp -> next = result -> next;                                      //The next pointer from the old chunk

        if(temp -> next != 0){                                              //If there was something in front of this chunk
           temp -> next -> prev = temp;                                     //The chunk in front's previous pointer must be set to the newly split chunk
        }

        result->size = size;                                                //The results new size is the requested data (excess has now been split off)
        result->next = temp;                                                //Point it to the split chunk
        result -> allocated = true;

    }

    return (void*)(((size_t)result) + sizeof(MemoryChunk));                 //Return the pointer to the newly allocated memory chunk
    return result;

}

void MemoryManager::free(void *pointer) {


    MemoryChunk* chunk = (MemoryChunk*)((size_t)pointer - sizeof(MemoryChunk));     //Subtract size of MemoryChunk as the pointer is seprate from the header

    chunk -> allocated = false;                                                     //Chunk is now free


    //To merge you pretty much tell the chunk that is free that it has all this extra data and then move this pointer to the free chunk

    //Merge the chunk behind
    if(chunk -> prev != 0 && !chunk -> prev -> allocated){                          //If there is a chunk behind this chunk and its free

        chunk -> prev -> next = chunk -> next;                                      //Set the previous chunk next to the chunk in front of this (remove it from the backward linked list)

        chunk->prev->size += chunk->size + sizeof(MemoryChunk);                     //Add the size that has been freed and its header

        if(chunk -> next != 0){                                                     //If there is a chunk infront of this chunk

            chunk -> next -> prev = chunk->prev;                                    //Set the chunk in front of this chunk  to the chunk behind this this (remove it from the forward linked list)

        }

        chunk = chunk -> prev;                                                      //Move the chunk pointer to the previous chunk

    }

    //Merge the chunk infront
    if(chunk -> next != 0 && !chunk -> next -> allocated){                          //If there is a chunk infront this chunk and its free

        chunk -> size += chunk -> next -> size + sizeof(MemoryChunk);             //Add the size that has been freed and its header

        chunk -> next = chunk -> next -> next;

        if(chunk -> next != 0){                                                     //If there is a chunk infront of this new chunk

            chunk -> next -> prev = chunk;                                         //Set the chunk in front of this chunk  to the chunk behind this this (remove it from the forward linked list)

        }



    }

}


//Redefine the default object functions with memory orientated ones (defaults disabled in makefile)


void* operator new(unsigned size){

    if(maxOS::MemoryManager::activeMemoryManager != 0){     //Check if there is a memory manager

        return maxOS::MemoryManager::activeMemoryManager -> malloc(size);

    }

    return 0;

}
void* operator new[](unsigned size){

    if(maxOS::MemoryManager::activeMemoryManager != 0){     //Check if there is a memory manager

        return maxOS::MemoryManager::activeMemoryManager -> malloc(size);

    }

    return 0;

}

//Placement New (see placement new operator)

void* operator new(unsigned size, void* pointer){

    return pointer;

}
void* operator new[](unsigned size, void* pointer){

    return pointer;

}

void operator delete(void* pointer){

    if(maxOS::MemoryManager::activeMemoryManager != 0){     //Check if there is a memory manager

        return maxOS::MemoryManager::activeMemoryManager -> free(pointer);

    }

}

void operator delete[](void* pointer){

    if(maxOS::MemoryManager::activeMemoryManager != 0){     //Check if there is a memory manager

        return maxOS::MemoryManager::activeMemoryManager -> free(pointer);

    }

}