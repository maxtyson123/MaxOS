//
// Created by 98max on 12/1/2022.
//

#ifndef MAXOS_COMMON_VECTOR_H
#define MAXOS_COMMON_VECTOR_H

#include <common/types.h>

namespace maxOS{

    namespace common{

        template<class Type> class VectorIterationHandler
        {
        public:
            VectorIterationHandler();
            ~VectorIterationHandler();
            virtual void OnRead(Type);
            virtual void OnEndOfStream();
        };




        template<class Type> class Vector
        {
        protected:
            Type elements[100]; // fake - currently max 100 elements - but the interface should be used already
            uint32_t Size;
            uint32_t MaxSize;
        public:
            typedef Type* iterator;

            Vector();
            Vector(int size, Type element);
            ~Vector();

            Type& operator[](int index);
            uint32_t size();
            iterator begin();
            iterator end();
            iterator find(Type element);
            bool empty();

            iterator pushBack(Type element);
            void popBack();
            iterator pushFront(Type element);
            void popFront();
            void erase(Type element);
            void erase(iterator position);
            void clear();

            void Iterate(VectorIterationHandler<Type>* vectorIterationHandler);
            void Iterate(void callback(Type&));
        };



    }
}

#include "../../src/common/vector_templates.cpp"

#endif //MAXOS_COMMON_VECTOR_H
