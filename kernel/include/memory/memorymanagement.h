//
// Created by 98max on 10/20/2022.
//

#ifndef MAXOS_SYSTEM_MEMORYMANAGEMENT_H
#define MAXOS_SYSTEM_MEMORYMANAGEMENT_H

#include <common/types.h>

namespace maxOS{

    namespace memory{

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
            static MemoryManager* activeMemoryManager; //Similar to how we have the active interrupt manager

            MemoryManager(common::size_t start, common::size_t size);
            ~MemoryManager();

            void* malloc(common::size_t size);
            void free(void* pointer);

            template<class Type> Type* Instantiate(common::uint32_t numberOfElements=1){
                Type* result = (Type*)malloc(sizeof(Type)*numberOfElements);
                new (result) Type();
                return result;
            }

            template<class Type, class Parameter1> Type* Instantiate(Parameter1 p1){
                Type* result = (Type*)malloc(sizeof(Type));
                new (result) Type(p1);
                return result;
            }

            template<class Type, class Parameter1, class Parameter2> Type* Instantiate(Parameter1 p1, Parameter2 p2)     {
                Type* result = (Type*)malloc(sizeof(Type));
                new (result) Type(p1, p2);
                return result;
            }

            template<class Type, class Parameter1, class Parameter2, class Parameter3> Type* Instantiate(Parameter1 p1, Parameter2 p2, Parameter3 p3) {
                Type* result = (Type*)malloc(sizeof(Type));
                new (result) Type(p1, p2, p3);
                return result;
            }

            template<class Type, class Parameter1, class Parameter2, class Parameter3, class Parameter4> Type* Instantiate(Parameter1 p1, Parameter2 p2, Parameter3 p3, Parameter4 p4) {
                Type* result = (Type*)malloc(sizeof(Type));
                new (result) Type(p1, p2, p3, p4);
                return result;
            }






        };
    }


}




void* operator new(unsigned size);
void* operator new[](unsigned size);

//Placement New
void* operator new(unsigned size, void* pointer);
void* operator new[](unsigned size, void* pointer);

void operator delete(void* pointer);
void operator delete[](void* pointer);

#endif //MAXOS_SYSTEM_MEMORYMANAGEMENT_H
