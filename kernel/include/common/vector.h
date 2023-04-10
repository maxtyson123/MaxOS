//
// Created by 98max on 12/1/2022.
//

#ifndef MAXOS_COMMON_VECTOR_H
#define MAXOS_COMMON_VECTOR_H

#include <common/types.h>

namespace maxOS{

    namespace common{

        template<class T> class VectorIterationHandler
        {
        public:
            VectorIterationHandler();
            ~VectorIterationHandler();
            virtual void OnRead(T);
            virtual void OnEndOfStream();
        };




        template<class T> class Vector
        {
        protected:
            T elements[100]; // fake - currently max 100 elements - but the interface should be used already
            uint32_t Size;
            uint32_t MaxSize;
        public:
            typedef T* iterator;

            Vector();
            Vector(int size, T element);
            ~Vector();

            T& operator[](int index);
            uint32_t size();
            iterator begin();
            iterator end();
            iterator find(T element);
            bool empty();

            iterator pushBack(T element);
            void popBack();
            iterator pushFront(T element);
            void popFront();
            void erase(T element);
            void erase(iterator position);
            void clear();

            void Iterate(VectorIterationHandler<T>* handler);
            void Iterate(void callback(T&));
        };



    }
}

#include "../../src/common/vector_templates.cpp"

#endif //MAXOS_COMMON_VECTOR_H
