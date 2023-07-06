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

    //______________________________________Implementation__________________________________________________
        /**
              * Base Template for VectorIterationHandler
              * @tparam Type Type of the Vector
              */
        template<class Type>
        Vector<Type>::Vector() {
            MaxSize = 100;
            Size = 0;
        }

        /**
         * Template for VectorIterationHandler
         * @tparam Type Type of the Vector
         * @param Size Size of the Vector
         * @param element Element to fill the Vector with
         */
        template<class Type>
        Vector<Type>::Vector(int Size, Type element) {
            MaxSize = 100;

            if (Size >= MaxSize)
                Size = MaxSize;

            this->Size = Size;
            for (iterator i = begin(); i != end(); i++)
                *i = element;
        }


        template<class Type>
        Vector<Type>::~Vector() {
        }

        /**
         * Operator [] for Vector
         * @tparam Type Type of the Vector
         * @param index The index of the element
         * @return the element at the index
         */
        template<class Type>
        Type &Vector<Type>::operator[](int index) {
            if (index <= Size)
                return elements[index];


        }

        /**
         * Returns the size of the Vector
         * @tparam Type    Type of the Vector
         * @return     Size of the Vector
         */
        template<class Type>
        uint32_t Vector<Type>::size() {
            return Size;
        }

        /**
         * Returns the begin of the Vector
         * @tparam Type   Type of the Vector
         * @return   The begin of the Vector
         */
        template<class Type>
        typename Vector<Type>::iterator Vector<Type>::begin() {
            return &elements[0];
        }

        /**
         * Returns the end of the Vector
         * @tparam Type   Type of the Vector
         * @return   The end of the Vector
         */
        template<class Type>
        typename Vector<Type>::iterator Vector<Type>::end() {
            return &elements[0] + Size;
        }

        /**
         * Finds an element in the Vector
         * @tparam Type Type of the Vector
         * @param element The element to find
         * @return The iterator of the element
         */
        template<class Type>
        typename Vector<Type>::iterator Vector<Type>::find(Type element) {
            for (iterator i = begin(); i != end(); ++i)      // for each element in the Vector
                if (*i ==
                    element)                           // if the element is equal to the element we are looking for
                    return i;                               // return the iterator of the element
            return end();                                   // else return the end of the Vector
        }

        /**
         * Empty the Vector
         * @tparam Type Type of the Vector
         * @return The iterator of the element
         */
        template<class Type>
        bool Vector<Type>::empty() {
            return begin() == end();
        }


        /**
         * Pushes an element to the Vector (at the end)
         * @tparam Type Type of the Vector
         * @param element The element to push

         * @return The iterator of the element
         */
        template<class Type>
        typename Vector<Type>::iterator Vector<Type>::pushBack(Type element) {
            if (Size >= MaxSize)                 // if the Vector is full
                return end();                   // return the end of the Vector

            elements[Size++] = element;        // else add the element to the Vector
            return end() - 1;                    // return the iterator of the element
        }

        /**
         * Removes an element from the Vector (at the end)
         * @tparam Type Type of the Vector
         */
        template<class Type>
        void Vector<Type>::popBack() {
            if (Size > 0)                // if the Vector is not empty
                --Size;                // remove the last element
        }

        /**
         * Removes an element from the Vector (at the index)
         * @tparam Type  Type of the Vector
         * @param element The element to remove

         * @return The iterator of the element
         */
        template<class Type>
        typename Vector<Type>::iterator Vector<Type>::pushFront(Type element) {
            if (Size >= MaxSize)                               // if the Vector is full
                return end();                                 // return the end of the Vector

            for (iterator i = end(); i > begin(); --i)       // for each element in the Vector
                *i = *(i - 1);                                // move the element one index to the right

            elements[0] = element;                          // add the element to front of the Vector
            ++Size;                                         // increase the size of the Vector

            return begin();                                 // return the iterator of the element
        }

        /**
         * Removes an element from the Vector (at the index)
         * @tparam Type  Type of the Vector

         */
        template<class Type>
        void Vector<Type>::popFront() {
            if (Size > 0)                                     //If the Vector is not empty
            {
                for (iterator i = begin() + 1; i != end(); ++i) // for each element in the Vector
                    *(i - 1) = *i;                             // move the element one index to the left
                --Size;                                      // decrease the size of the Vector
            }
        }

        /**
         * Removes an element from the Vector
         * @tparam Type The type of the Vector
         * @param element The element to remove
         */
        template<class Type>
        void Vector<Type>::erase(Type element) {
            int hits = 0;                                   // number of hits
            for (iterator i = begin(); i != end(); ++i)      // for each element in the Vector
            {
                if (*i ==
                    element)                           // if the element is equal to the element we are looking for
                {
                    ++hits;                                 // increase the number of hits
                } else {
                    if (hits > 0)                            // if we have hits
                        *(i - hits) = *i;                     // move the element one index to the left
                }

            }
            Size -= hits;                                   // decrease the size of the Vector
        }

        /**
         * Removes the element at the index
         * @tparam Type The type of the Vector
         * @param position The position of the element to remove
         */
        template<class Type>
        void Vector<Type>::erase(typename Vector<Type>::iterator position) {
            // element not in this vector
            if (position < begin() || position >= end())
                return;

            // move all elements one index to the left
            for (++position; position != end(); ++position)
                *(position - 1) = *position;

            // decrease the size of the Vector
            --Size;
        }

        /**
         * Clears the Vector
         * @tparam Type  Type of the Vector
         */
        template<class Type>
        void Vector<Type>::clear() {
            Size = 0;
        }


        /**
         * Iterator for Vector
         * @tparam Type  Type of the Vector

         * @param vectorIterationHandler  The handler of the Vector
         */
        template<class Type>
        void Vector<Type>::Iterate(VectorIterationHandler<Type> *vectorIterationHandler) {
            // for each element in the Vector
            for (iterator i = begin(); i != end(); ++i)
                vectorIterationHandler -> OnRead(*i);                    // call the OnRead function of the handler
            vectorIterationHandler -> OnEndOfStream();                   // call the OnEndOfStream function of the handler
        }

        /**
         * Iterates over the Vector
         * @tparam Type Type of the Vector
         * @param callback The callback function
         */
        template<class Type>
        void Vector<Type>::Iterate(void callback(Type &)) {
            for (iterator i = begin(); i != end(); ++i)              // for each element in the Vector
                callback(*i);                                       // call the callback function
        }
    }
}


#endif //MAXOS_COMMON_VECTOR_H
