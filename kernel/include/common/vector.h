//
// Created by 98max on 12/1/2022.
//

#ifndef MAXOS_COMMON_VECTOR_H
#define MAXOS_COMMON_VECTOR_H

#include <stdint.h>

namespace maxOS{

    namespace common{

        /**
         * @class VectorIterationHandler
         * @brief Handles the iteration of a Vector providing read and end of stream functions
         *
         * @tparam Type Type of the Vector
         */
        template<class Type> class VectorIterationHandler
        {
        public:
            VectorIterationHandler();
            ~VectorIterationHandler();

            virtual void on_read(Type);
            virtual void on_end_of_stream();
        };

        /**
         * @class Vector
         * @brief Dynamically stores an array of elements
         *
         * @tparam Type Type of the Vector
         */
        template<class Type> class Vector
        {
        protected:
            Type m_elements[100]; //Todo: make this dynamic
            uint32_t m_size { 0 };
            uint32_t m_max_size { 100 };
        public:
            typedef Type* iterator;

            Vector();
            Vector(int size, Type element);
            ~Vector();

            Type& operator[](uint32_t index);

            bool empty();
            uint32_t size();

            iterator begin();
            iterator end();
            iterator find(Type);

            iterator push_back(Type);
            void pop_back();

            iterator push_front(Type);
            void pop_front();

            void erase(Type);
            void erase(iterator position);
            void clear();

            void iterate(VectorIterationHandler<Type>*);
            void Iterate(void callback(Type&));
        };

    ///______________________________________Implementation__________________________________________________
        /**
        * @brief Constructor for Vector
        *
        * @tparam Type Type of the Vector
        */
        template<class Type> Vector<Type>::Vector() {

        }

        /**
        * @brief Constructor for Vector
         *
        * @tparam Type Type of the Vector
        * @param Size Size of the Vector
        * @param element Element to fill the Vector with
        */
        template<class Type> Vector<Type>::Vector(int Size, Type element) {

          // Make sure the size is not bigger than the max size
          if (Size > m_max_size)
          Size = m_max_size;

          // Fill the Vector with the element
          for (int i = 0; i < Size; ++i)
                  m_elements[i] = element;

          // Set the size of the Vector
          m_size = Size;
        }


        template<class Type> Vector<Type>::~Vector() {
        }

        /**
         * @brief Overloads the [] operator to return the element at the index
         *
         * @tparam Type Type of the Vector
         * @param index The index of the element
         * @return the element at the index or the end of the Vector if the index is out of bounds
         */
        template<class Type> Type &Vector<Type>::operator[](uint32_t index) {

            // If the index is in the Vector
            if (index <= m_size)
                return m_elements[index];

            // Return the last element of the Vector
            return m_elements[m_size - 1];

        }

        /**
         * @brief Returns the number of elements in the Vector
         *
         * @tparam Type Type of the Vector
         * @return The size of the Vector
         */
        template<class Type> uint32_t Vector<Type>::size() {
            return m_size;
        }

        /**
         * @brief Returns the m_first_memory_chunk element of the Vector
         *
         * @tparam Type Type of the Vector
         * @return The m_first_memory_chunk element of the Vector
         */
        template<class Type> typename Vector<Type>::iterator Vector<Type>::begin() {
            return &m_elements[0];
        }

        /**
         * @brief Returns the last element of the Vector
         *
         * @tparam Type Type of the Vector
         * @return The last element of the Vector
         */
         template<class Type> typename Vector<Type>::iterator Vector<Type>::end() {
            return &m_elements[0] + m_size;
         }

        /**
         * @brief Finds an element in the Vector and returns the iterator of the element
         *
         * @tparam Type Type of the Vector
         * @param element The element to find
         * @return The iterator of the element or the end of the Vector if the element is not found
         */
        template<class Type> typename Vector<Type>::iterator Vector<Type>::find(Type element) {

            // Find the element
            for (iterator i = begin(); i != end(); ++i)
            if (*i == element)
                    return i;

            // The element must not be in the Vector
            return end();
        }

        /**
         * @brief Checks if the Vector is empty
         *
         * @tparam Type Type of the Vector
         * @return True if the Vector is empty, false otherwise
         */
        template<class Type> bool Vector<Type>::empty() {
            return begin() == end();
        }


        /**
         * @brief Adds an element to the end of the vector and returns the iterator of the element
         *
         * @tparam Type Type of the Vector
         * @param element The element to add
         * @return The iterator of the element, if the Vector is full it returns the end of the Vector
         */
        template<class Type> typename Vector<Type>::iterator Vector<Type>::push_back(Type element) {
            // Return the end of the Vector if it is full
            if (m_size >= m_max_size)
                    return end();

            // Add the element to the Vector and return the iterator of the element
            m_elements[m_size++] = element;
            return end() - 1;
        }

        /**
         * @brief Removes the last element from the Vector
         * @tparam Type Type of the Vector
         */
        template<class Type> void Vector<Type>::pop_back() {
            // Remove the last element from the Vector
            if (m_size > 0)
                    --m_size;
        }

        /**
         * @brief Adds an element to the front of the Vector and returns the iterator of the element
         *
         * @tparam Type Type of the Vector
         * @param element The element to add
         * @return The iterator of the element, if the Vector is full it returns the end of the Vector
         */
        template<class Type> typename Vector<Type>::iterator Vector<Type>::push_front(Type element) {

            // Make sure the Vector is not full
            if (m_size >= m_max_size)
                return end();

            // Move all elements one index to the right
            for (iterator i = end(); i > begin(); --i)
                *i = *(i - 1);

            // Add the element to the front of the Vector
            m_elements[0] = element;
            ++m_size;

            // Return the iterator of the element
            return begin();
        }

        /**
         * @brief Removes the m_first_memory_chunk element from the Vector
         *
         * @tparam Type Type of the Vector
         */
        template<class Type> void Vector<Type>::pop_front() {

          // Make sure the Vector is not empty
          if (m_size == 0)
            return;

          // Move all elements one index to the left
          for (iterator i = begin(); i != end(); ++i)
              *i = *(i + 1);

          // Decrease the size of the Vector
          --m_size;
        }

        /**
         * @brief Removes all elements from the Vector that are equal to the element
         *
         * @tparam Type Type of the Vector
         * @param element The element to remove
         */
        template<class Type> void Vector<Type>::erase(Type element) {

            // Search for the element
            int hits = 0;
            for (iterator i = begin(); i != end(); ++i)
            {
                // If it is the element we are looking for
                if (*i == element)
                {
                    ++hits;
                } else {

                    // If there are hits move the element to the left
                    if (hits > 0)
                        *(i - hits) = *i;
                }

            }

            // Decrease the size of the Vector
            m_size -= hits;
        }

        /**
         * @brief Removes the element at the m_position
         *
         * @tparam Type The type of the Vector
         * @param position The m_position of the element to remove
         */
        template<class Type>
        void Vector<Type>::erase(typename Vector<Type>::iterator position) {

            // If the m_position is not in the Vector
            if (position < begin() || position >= end())
                return;

            // Move all elements one index to the left
            for (++position; position != end(); ++position)
                *(position - 1) = *position;

            // Decrease the size of the Vector
            --m_size;
        }

        /**
         * @brief Removes all elements from the Vector
         *
         * @tparam Type Type of the Vector
         */
        template<class Type> void Vector<Type>::clear() {
          m_size = 0;
        }

        /**
         * @brief Iterates over the Vector and calls the OnRead function of the handler for each element
         *
         * @tparam Type Type of the Vector
         * @param vector_iteration_handler The handler
         */
        template<class Type> void Vector<Type>::iterate(VectorIterationHandler<Type> *vector_iteration_handler) {

          // Call the OnRead function of the handler for each element
          for(auto& element : m_elements)
            vector_iteration_handler->on_read(element);

          // Call the OnEndOfStream function of the handler
          vector_iteration_handler->on_end_of_stream();
        }


        /**
         * @brief Iterates over the Vector and calls the callback function for each element
         *
         * @tparam Type Type of the Vector
         * @param callback The callback function
         */
        template<class Type> void Vector<Type>::Iterate(void callback(Type &)) {

           // Call the callback function for each element
           for(auto& element : m_elements)
              callback(element);
        }

        template<class Type> VectorIterationHandler<Type>::VectorIterationHandler() {

        }

        template<class Type> VectorIterationHandler<Type>::~VectorIterationHandler() {

        }

        template<class Type> void VectorIterationHandler<Type>::on_end_of_stream() {

        }

        template<class Type> void VectorIterationHandler<Type>::on_read(Type) {

        }
    }
}


#endif //MAXOS_COMMON_VECTOR_H
