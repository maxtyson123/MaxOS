//
// Created by 98max on 12/1/2022.
//

#ifndef MAXOS_COMMON_VECTOR_H
#define MAXOS_COMMON_VECTOR_H

#include <stdint.h>
#include <stddef.h>

namespace MaxOS{

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
            Type* m_elements;
            uint32_t m_size { 0 };
            uint32_t m_capacity { 1 };

        public:
            typedef Type* iterator;

            Vector();
            Vector(int Size, Type element);
	        Vector(const Vector<Type>& other);
	        Vector(Vector<Type>&& other);
            ~Vector();

            Type& operator[](uint32_t index) const;
	        Vector<Type>& operator=(const Vector<Type>& other);
	        Vector<Type>& operator=(Vector<Type>&& other);

            [[nodiscard]] bool empty() const;
			[[nodiscard]] uint32_t size() const;

            iterator begin() const;
            iterator end() const;
            iterator find(Type) const;

            iterator push_back(Type);
            Type pop_back();

            iterator push_front(Type);
            Type pop_front();

            void erase(Type);
            void erase(iterator position);
            void clear();

			void reserve(size_t amount);
	        void increase_size();

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

            // Allocate space for the array
            m_elements = new Type[m_capacity];

        }

        /**
        * @brief Constructor for Vector
         *
        * @tparam Type Type of the Vector
        * @param size Size of the Vector
        * @param element Element to fill the Vector with
        */
        template<class Type> Vector<Type>::Vector(int size, Type element) {

            // Allocate space for the array
            m_elements = new Type[size];
	        m_capacity = size > 0 ? size : 1;
			m_size     = 0;

            // Push all the elements to the Vector
            for (int i = 0; i < size; ++i)
                    push_back(element);
        }

		/**
		 * @brief Copy constructor for Vector
		 *
		 * @tparam Type The type of data to be stored
		 * @param other The vector to copy from
		 */
	    template<class Type> Vector<Type>::Vector(const Vector<Type>& other)
	    : m_size(other.m_size),
		  m_capacity(other.m_capacity)
		{
			// Copy each element into a new array
		    m_elements = new Type[m_capacity];
		    for (uint32_t i = 0; i < m_size; ++i)
			    m_elements[i] = other.m_elements[i];
	    }

		/**
		 * @brief Move constructor for Vector
		 *
	     * @tparam Type The type of data to be stored
		 * @param other The vector to copy from
		 */
	    template<class Type> Vector<Type>::Vector(Vector<Type> &&other)
		: m_elements(other.m_elements),
		  m_size(other.m_size),
		  m_capacity(other.m_capacity)
	    {

			// Clear the other Vector
			other.m_elements = nullptr;
			other.m_size = 0;
			other.m_capacity = 0;

	    }

        template<class Type> Vector<Type>::~Vector() {

            // De-allocate the array
            delete[] m_elements;

        }

        /**
         * @brief Increases the size of the Vector by doubling the capacity
         *
         * @tparam Type Type of the Vector
         */
        template <class Type> void Vector<Type>::increase_size() {
	        reserve(m_capacity * 2);
        }

	    template<class Type> void Vector<Type>::reserve(size_t amount) {


		    // Increase the capacity of the Vector
			if(m_capacity < amount)
				m_capacity = amount;

		    // Allocate more space for the array
		    Type* new_elements = new Type[amount];

		    // Copy the elements to the new array
		    for (uint32_t i = 0; i < m_size; ++i)
			    new_elements[i] = m_elements[i];

		    // De-allocate the old array
		    delete[] m_elements;

		    // Set the new array
		    m_elements = new_elements;
	    }


	    /**
         * @brief Overloads the [] operator to return the element at the index
         *
         * @tparam Type Type of the Vector
         * @param index The index of the element
         * @return the element at the index or the end of the Vector if the index is out of bounds
         */
        template<class Type> Type &Vector<Type>::operator[](uint32_t index) const{

            // If the index is in the Vector
            if (index < m_size)
                return m_elements[index];

            // Return the last element of the Vector
            return m_elements[m_size - 1];

        }

		/**
		 * @brief Assignment by copy, data is copied into a new buffer stored in this vector
		 *
		 * @tparam Type Type of the Vector
         * @param other The vector to copy from
		 * @return This vector, with the copied elements
		 */
	    template<class Type> Vector<Type>& Vector<Type>::operator=(const Vector<Type>& other) {

			// Setting to itself?
			if (this == &other)
				return *this;

			// Create a new buffer to store the elements
		    delete[] m_elements;
			m_elements = new Type[other.m_capacity];

			// Copy data
		    m_size = other.m_size;
		    m_capacity = other.m_capacity;
		    for (uint32_t i = 0; i < m_size; ++i)
			    m_elements[i] = other.m_elements[i];

		    return *this;
	    }

	    /**
		 * @brief Assignment by move, data is moved into the buffer stored in this vector and the other vector is cleared
		 *
		 * @tparam Type Type of the Vector
		 * @param other The vector to copy from
		 * @return This vector, with the copied elements
		 */
	    template<class Type> Vector<Type>& Vector<Type>::operator=(Vector<Type>&& other) noexcept {

		    // Moving to itself?
		    if (this == &other)
			    return *this;

		    // Move into this vector
		    delete[] m_elements;
		    m_elements = other.m_elements;
		    m_size = other.m_size;
		    m_capacity = other.m_capacity;

			// Remove from other vector
		    other.m_elements = nullptr;
		    other.m_size = 0;
		    other.m_capacity = 0;

		    return *this;
	    }

        /**
         * @brief Returns the number of elements in the Vector
         *
         * @tparam Type Type of the Vector
         * @return The size of the Vector
         */
        template<class Type> uint32_t Vector<Type>::size() const{
            return m_size;
        }

        /**
         * @brief Returns the first element of the Vector
         *
         * @tparam Type Type of the Vector
         * @return The first element of the Vector
         */
        template<class Type> typename Vector<Type>::iterator Vector<Type>::begin() const{
            return &m_elements[0];
        }

        /**
         * @brief Returns the last element of the Vector
         *
         * @tparam Type Type of the Vector
         * @return The last element of the Vector
         */
         template<class Type> typename Vector<Type>::iterator Vector<Type>::end() const{
            return &m_elements[0] + m_size;
         }

        /**
         * @brief Finds an element in the Vector and returns the iterator of the element
         *
         * @tparam Type Type of the Vector
         * @param element The element to find
         * @return The iterator of the element or the end of the Vector if the element is not found
         */
        template<class Type> typename Vector<Type>::iterator Vector<Type>::find(Type element) const{

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
        template<class Type> bool Vector<Type>::empty() const{
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

            // Check if we need to allocate more space for the array
            if(m_size == m_capacity)
                increase_size();

            // Add the element to the Vector and return the iterator of the element
            m_elements[m_size++] = element;
            return end() - 1;
        }

        /**
         * @brief Removes the last element from the Vector
         * @tparam Type Type of the Vector
         */
        template<class Type> Type Vector<Type>::pop_back() {

            // Remove the last element from the Vector
            if (m_size > 0)
                    --m_size;

	        return m_elements[m_size];
        }

        /**
         * @brief Adds an element to the front of the Vector and returns the iterator of the element
         *
         * @tparam Type Type of the Vector
         * @param element The element to add
         * @return The iterator of the element, if the Vector is full it returns the end of the Vector
         */
        template<class Type> typename Vector<Type>::iterator Vector<Type>::push_front(Type element) {

            // Check if we need to allocate more space for the array
            if(m_size == m_capacity)
                    increase_size();

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
         * @return The element that was removed, or a default constructed element if the Vector is empty
         */
        template<class Type> Type Vector<Type>::pop_front() {

	        // Make sure the Vector is not empty
	        if (m_size == 0)
	          return Type();

			// Store the element to return

			Type element = m_elements[0];

            // Move all elements one index to the left
	        for (uint32_t i = 0; i < m_size - 1; ++i)
		        m_elements[i] = m_elements[i + 1];

            // Decrease the size of the Vector
            --m_size;
	        return element;
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
        template<class Type> void Vector<Type>::erase(typename Vector<Type>::iterator position) {

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

        template<class Type> VectorIterationHandler<Type>::VectorIterationHandler() = default;

        template<class Type> VectorIterationHandler<Type>::~VectorIterationHandler() = default;

        template<class Type> void VectorIterationHandler<Type>::on_end_of_stream() {

        }

        template<class Type> void VectorIterationHandler<Type>::on_read(Type) {

        }
    }
}


#endif //MAXOS_COMMON_VECTOR_H
