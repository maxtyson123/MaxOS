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

            template<class T> T* Instantiate(common::uint32_t numberOfElements=1){
                T* result = (T*)malloc(sizeof(T)*numberOfElements);
                new (result) T();
                return result;
            }

            template<class T, class P1> T* Instantiate(P1 p1){
                T* result = (T*)malloc(sizeof(T));
                new (result) T(p1);
                return result;
            }

            template<class T, class P1, class P2> T* Instantiate(P1 p1, P2 p2)     {
                T* result = (T*)malloc(sizeof(T));
                new (result) T(p1, p2);
                return result;
            }

            template<class T, class P1, class P2, class P3> T* Instantiate(P1 p1, P2 p2, P3 p3) {
                T* result = (T*)malloc(sizeof(T));
                new (result) T(p1, p2, p3);
                return result;
            }

            template<class T, class P1, class P2, class P3, class P4> T* Instantiate(P1 p1, P2 p2, P3 p3, P4 p4) {
                T* result = (T*)malloc(sizeof(T));
                new (result) T(p1, p2, p3, p4);
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
