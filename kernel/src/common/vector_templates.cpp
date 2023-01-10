//
// Created by 98max on 12/1/2022.
//



namespace maxOS
{
    namespace common {
        /**
         * Base Template for VectorIterationHandler
         * @tparam T Type of the Vector
         */
        template<class T>
        Vector<T>::Vector() {
            MaxSize = 100;
            Size = 0;
        }

        /**
         * Template for VectorIterationHandler
         * @tparam T Type of the Vector
         * @param Size Size of the Vector
         * @param element Element to fill the Vector with
         */
        template<class T>
        Vector<T>::Vector(int Size, T element) {
            MaxSize = 100;

            if (Size >= MaxSize)
                Size = MaxSize;

            this->Size = Size;
            for (iterator i = begin(); i != end(); i++)
                *i = element;
        }


        template<class T>
        Vector<T>::~Vector() {
        }

        /**
         * Operator [] for Vector
         * @tparam T Type of the Vector
         * @param index The index of the element
         * @return the element at the index
         */
        template<class T>
        T &Vector<T>::operator[](int index) {
            if (index <= Size)
                return elements[index];


        }

        /**
         * Returns the size of the Vector
         * @tparam T    Type of the Vector
         * @return     Size of the Vector
         */
        template<class T>
        uint32_t Vector<T>::size() {
            return Size;
        }

        /**
         * Returns the begin of the Vector
         * @tparam T   Type of the Vector
         * @return   The begin of the Vector
         */
        template<class T>
        typename Vector<T>::iterator Vector<T>::begin() {
            return &elements[0];
        }

        /**
         * Returns the end of the Vector
         * @tparam T   Type of the Vector
         * @return   The end of the Vector
         */
        template<class T>
        typename Vector<T>::iterator Vector<T>::end() {
            return &elements[0] + Size;
        }

        /**
         * Finds an element in the Vector
         * @tparam T Type of the Vector
         * @param element The element to find
         * @return The iterator of the element
         */
        template<class T>
        typename Vector<T>::iterator Vector<T>::find(T element) {
            for (iterator i = begin(); i != end(); ++i)      // for each element in the Vector
                if (*i ==
                    element)                           // if the element is equal to the element we are looking for
                    return i;                               // return the iterator of the element
            return end();                                   // else return the end of the Vector
        }

        /**
         * Emptys the Vector
         * @tparam T Type of the Vector
         * @return The iterator of the element
         */
        template<class T>
        bool Vector<T>::empty() {
            return begin() == end();
        }


        /**
         * Pushes an element to the Vector (at the end)
         * @tparam T Type of the Vector
         * @param element The element to push

         * @return The iterator of the element
         */
        template<class T>
        typename Vector<T>::iterator Vector<T>::push_back(T element) {
            if (Size >= MaxSize)                 // if the Vector is full
                return end();                   // return the end of the Vector

            elements[Size++] = element;        // else add the element to the Vector
            return end() - 1;                    // return the iterator of the element
        }

        /**
         * Removes an element from the Vector (at the end)
         * @tparam T Type of the Vector
         */
        template<class T>
        void Vector<T>::pop_back() {
            if (Size > 0)                // if the Vector is not empty
                --Size;                // remove the last element
        }

        /**
         * Removes an element from the Vector (at the index)
         * @tparam T  Type of the Vector
         * @param element The element to remove

         * @return The iterator of the element
         */
        template<class T>
        typename Vector<T>::iterator Vector<T>::push_front(T element) {
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
         * @tparam T  Type of the Vector

         */
        template<class T>
        void Vector<T>::pop_front() {
            if (Size > 0)                                     //If the Vector is not empty
            {
                for (iterator i = begin() + 1; i != end(); ++i) // for each element in the Vector
                    *(i - 1) = *i;                             // move the element one index to the left
                --Size;                                      // decrease the size of the Vector
            }
        }

        /**
         * Removes an element from the Vector
         * @tparam T The type of the Vector
         * @param element The element to remove
         */
        template<class T>
        void Vector<T>::erase(T element) {
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
         * @tparam T The type of the Vector
         * @param position The position of the element to remove
         */
        template<class T>
        void Vector<T>::erase(typename Vector<T>::iterator position) {
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
         * @tparam T  Type of the Vector
         */
        template<class T>
        void Vector<T>::clear() {
            Size = 0;
        }


        /**
         * Iterator for Vector
         * @tparam T  Type of the Vector

         * @param handler  The handler of the Vector
         */
        template<class T>
        void Vector<T>::Iterate(VectorIterationHandler<T> *handler) {
            // for each element in the Vector
            for (iterator i = begin(); i != end(); ++i)
                handler->OnRead(*i);                    // call the OnRead function of the handler
            handler->OnEndOfStream();                   // call the OnEndOfStream function of the handler
        }

        /**
         * Iterates over the Vector
         * @tparam T Type of the Vector
         * @param callback The callback function
         */
        template<class T>
        void Vector<T>::Iterate(void callback(T &)) {
            for (iterator i = begin(); i != end(); ++i)              // for each element in the Vector
                callback(*i);                                       // call the callback function
        }
    }
}