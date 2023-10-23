//
// Created by 98max on 23/10/2023.
//

#ifndef MAXOS_COMMON_MAP_H
#define MAXOS_COMMON_MAP_H

#include <common/vector.h>
#include <common/pair.h>

namespace maxOS{

    namespace common{

        template<class Key, class Value> class MapIterationHandler
        {
        public:
            MapIterationHandler();
            ~MapIterationHandler();
            virtual void onRead(Key, Value);
            virtual void onEndOfStream();
        };

        template<class Key, class Value> class Map
        {
            protected:
                Vector<Pair<Key, Value> > elements;

            public:
                typedef typename Vector<Pair<Key, Value> >::iterator iterator;

                Map();
                ~Map();

                Value& operator[](Key key);
                iterator begin();
                iterator end();
                iterator find(Key element);

                bool empty();
                void clear();
                void insert(Key key, Value value);
                void erase(Key key);

                void iterate(MapIterationHandler<Key, Value>* handler);
                void iterate(void callback(Key&, Value&));

        };

        /// ______________ TEMPLATE IMPLEMENTATION ______________
        template<class Key, class Value> void MapIterationHandler<Key, Value>::onEndOfStream() {

        }

        template<class Key, class Value> void MapIterationHandler<Key, Value>::onRead(Key, Value) {

        }

        template<class Key, class Value> MapIterationHandler<Key, Value>::~MapIterationHandler() {

        }

        template<class Key, class Value> MapIterationHandler<Key, Value>::MapIterationHandler() {

        }


        template<class Key, class Value> Map<Key, Value>::Map() {

        }

        template<class Key, class Value> Map<Key, Value>::~Map() {

        }

        template<class Key, class Value> Value &Map<Key, Value>::operator[](Key key) {

            // Return the value of the key (second item in the pair)
            return find(key) -> second;
        }

        template<class Key, class Value> typename Map<Key, Value>::iterator Map<Key, Value>::begin() {
            return elements.begin();
        }

        template<class Key, class Value> typename Map<Key, Value>::iterator Map<Key, Value>::end() {
            return elements.end();
        }

        template<class Key, class Value> typename Map<Key, Value>::iterator Map<Key, Value>::find(Key element) {

            // Loop through the elements
            for (iterator it = begin(); it != end(); it++) {

                // If the key of the current element is equal to the key we are looking for
                if (it -> first == element) {
                    // Return the iterator
                    return it;
                }
            }

            // If it is not found, return the end iterator
            return end();

        }

        template<class Key, class Value> bool Map<Key, Value>::empty() {
            return elements.empty();
        }

        template<class Key, class Value> void Map<Key, Value>::clear() {
            elements.clear();
        }

        template<class Key, class Value> void Map<Key, Value>::insert(Key key, Value value) {

            // Find the element
            iterator it = find(key);

            // If the element is found
            if (it != end()) {
                // Set the value of the element to the new value
                it -> second = value;
            } else {
                // Otherwise, add a new element
                elements.pushBack(Pair<Key, Value>(key, value));
            }
        }

        template<class Key, class Value> void Map<Key, Value>::erase(Key key) {

            // Find the element
            iterator it = find(key);

            // If the element is found
            if (it != end()) {
                // Erase the element
                elements.erase(it);
            }

        }

        template<class Key, class Value> void Map<Key, Value>::iterate(MapIterationHandler<Key, Value> *handler) {

            // Loop through the elements
            for (iterator it = begin(); it != end(); it++) {
                // Call the handler
                handler -> onRead(it -> first, it -> second);
            }

            // Call the handler
            handler -> onEndOfStream();

        }

        template<class Key, class Value> void Map<Key, Value>::iterate(void (*callback)(Key &, Value &)) {

            // Loop through the elements
            for (iterator it = begin(); it != end(); it++) {

                // Call the callback
                callback(it -> first, it -> second);
            }
        }
    }
}

#endif //MAXOS_COMMON_MAP_H
