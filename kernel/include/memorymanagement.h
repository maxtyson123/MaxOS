//
// Created by 98max on 10/20/2022.
//

#ifndef MAXOS_MEMORYMANAGEMENT_H
#define MAXOS_MEMORYMANAGEMENT_H

#include <common/types.h>

namespace maxOS{

    struct MemoryChunk{

        MemoryChunk* next;
        MemoryChunk* prev;

        bool allocated;
        common::size_t size;

    };

    class MemoryManager{

        protected:
            MemoryChunk* first;

        public:
            static MemoryManager* activeMemoryManager; //Simmilar to how we have the active interrupt manager

            MemoryManager(common::size_t start, common::size_t size);
            ~MemoryManager();

            void* malloc(common::size_t size);
            void free(void* pointer);

    };

}

#endif //MAXOS_MEMORYMANAGEMENT_H
