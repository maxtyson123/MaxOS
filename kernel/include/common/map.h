//
// Created by 98max on 23/10/2023.
//

#ifndef MAXOS_COMMON_MAP_H
#define MAXOS_COMMON_MAP_H

#include <common/vector.h>
#include <common/pair.h>

namespace MaxOS{

    namespace common{

        /**
         * @class MapIterationHandler
         * @brief Handles iteration of a map
         *
         * @tparam Key The key type
         * @tparam Value The value type
         */
        template<class Key, class Value> class MapIterationHandler
        {
        public:
            MapIterationHandler();
            ~MapIterationHandler();

            virtual void on_read(Key, Value);
            virtual void on_end_of_stream();
        };

        /**
         * @class Map
         * @brief A list of key-value pairs
         *
         * @tparam Key The key type
         * @tparam Value The value type
         */
        template<class Key, class Value> class Map
        {
            protected:
                Vector<Pair<Key, Value>> m_elements;

            public:
                typedef typename Vector<Pair<Key, Value>>::iterator iterator;

                Map();
                ~Map();

	            Value& operator[](Key);

		        bool empty();
		        int size();

                iterator begin();
                iterator end();
                iterator find(Key);

		        iterator push_back(Key, Value);
	            Pair<Key, Value> pop_back();

		        iterator push_front(Key, Value);
	            Pair<Key, Value> pop_front();

	            void insert(Key, Value);

		        void erase(Key);
		        void erase(iterator position);
		        void clear();

                void iterate(MapIterationHandler<Key, Value>* handler);
                void iterate(void callback(Key&, Value&));

        };


	    /// ______________ TEMPLATE IMPLEMENTATION ______________
        template<class Key, class Value> MapIterationHandler<Key, Value>::MapIterationHandler() = default;

        template<class Key, class Value> MapIterationHandler<Key, Value>::~MapIterationHandler() = default;

        /**
         * @brief Called when the end of the stream is reached
         *
         * @tparam Key The key type
         * @tparam Value The value type
         */
        template<class Key, class Value> void MapIterationHandler<Key, Value>::on_end_of_stream() {

        }

        /**
         * @brief Called when a key-value pair is read
         *
         * @tparam Key The key type
         * @tparam Value The value type
         */
        template<class Key, class Value> void MapIterationHandler<Key, Value>::on_read(Key, Value) {

        }

        template<class Key, class Value> Map<Key, Value>::Map() = default;

        template<class Key, class Value> Map<Key, Value>::~Map() = default;

        /**
         * @brief Overloads the [] operator to return the value of the key
         *
         * @tparam Key The key type
         * @tparam Value The value type
         * @param key The key to search for
         * @return The value of the key
         */
        template<class Key, class Value> Value &Map<Key, Value>::operator[](Key key) {

            // Return the value of the key (second item in the pair)
            return find(key) -> second;
        }

        /**
         * @brief Returns the beginning of the map
         *
         * @tparam Key The key type
         * @tparam Value The value type
         * @return The m_first_memory_chunk element in the map
         */
        template<class Key, class Value> typename Map<Key, Value>::iterator Map<Key, Value>::begin() {
            return m_elements.begin();
        }

        /**
         * @brief Returns the end of the map
         *
         * @tparam Key The key type
         * @tparam Value The value type
         * @return The last element in the map
         */
        template<class Key, class Value> typename Map<Key, Value>::iterator Map<Key, Value>::end() {
            return m_elements.end();
        }

        /**
         * @brief Finds an element in the map based on the key
         *
         * @tparam Key The key type
         * @tparam Value The value type
         * @param element The key to search for
         * @return The iterator of the element, or the end iterator if not found
         */
        template<class Key, class Value> typename Map<Key, Value>::iterator Map<Key, Value>::find(Key element) {

            // Search for the element
            for (iterator it = begin(); it != end(); it++)
                if (it -> first == element)
                    return it;

            // Item not found
            return end();

        }

	    template<class Key, class Value> Map<Key, Value>::iterator Map<Key, Value>::push_back(Key key, Value value) {
		    return m_elements.push_back(Pair<Key, Value>(key, value));
	    }

	    template<class Key, class Value> Pair<Key, Value> Map<Key, Value>::pop_back() {
		    return m_elements.pop_back();
	    }

	    template<class Key, class Value> Map<Key, Value>::iterator Map<Key, Value>::push_front(Key key, Value value) {
		    return m_elements.push_front({key, value});
	    }

	    template<class Key, class Value> Pair<Key, Value> Map<Key, Value>::pop_front() {
		    return m_elements.pop_front();
	    }


        /**
         * @brief Returns whether the map is empty
         *
         * @tparam Key The key type
         * @tparam Value The value type
         * @return Whether the map is empty
         */
        template<class Key, class Value> bool Map<Key, Value>::empty() {
            return m_elements.empty();
        }

        /**
         * @brief The number of elements in the map
         *
         * @return The number of elements in the Map
         */
        template <class Key, class Value> int Map<Key, Value>::size()
        {
            // Return the size of the vector
            return m_elements.size();

        }

        /**
         * @brief Removes all elements from the map
         *
         * @tparam Key The key type
         * @tparam Value The value type
         */
        template<class Key, class Value> void Map<Key, Value>::clear() {
          m_elements.clear();
        }

        /**
         * @brief Updates the value of an element, or adds a new element if it does not exist
         *
         * @tparam Key The key type
         * @tparam Value The value type
         * @param key The key of the new element
         * @param value The value of the new element
         */
        template<class Key, class Value> void Map<Key, Value>::insert(Key key, Value value) {

            // Find the element
            iterator it = find(key);

            // If the element is found then update the value
            if (it != end()) {
                it -> second = value;
            } else {
                // Otherwise, add a new element
                m_elements.push_back(Pair<Key, Value>(key, value));
            }
        }

        /**
         * @brief Removes an element from the map
         *
         * @tparam Key The key type
         * @tparam Value The value type
         * @param key The key of the element to remove
         */
        template<class Key, class Value> void Map<Key, Value>::erase(Key key) {

            // Find the element
            iterator it = find(key);

            // If the element is found then remove it
            if (it != end()) {
                m_elements.erase(it);
            }

        }

	    template<class Key, class Value> void Map<Key, Value>::erase(Map::iterator position) {
			m_elements.erase(position);

	    }

        /**
         * @brief Iterates through the map and calls the handler
         *
         * @tparam Key The key type
         * @tparam Value The value type
         * @param handler The handler to call
         */
        template<class Key, class Value> void Map<Key, Value>::iterate(MapIterationHandler<Key, Value> *handler) {

            // Loop through the elements
            for(auto& it : m_elements) {

                // Call the handler
                handler->on_read(it.first, it.second);
            }

            // Call the handler
            handler->on_end_of_stream();

        }

        /**
         * @brief Iterates through the map and calls the callback
         *
         * @tparam Key The key type
         * @tparam Value The value type
         * @param callback The callback to call
         */
        template<class Key, class Value> void Map<Key, Value>::iterate(void (*callback)(Key &, Value &)) {

            // Call the callback for each element
            for(auto& it : m_elements) {
                    callback(it.first, it.second);
            }
        }
    }
}

#endif //MAXOS_COMMON_MAP_H
