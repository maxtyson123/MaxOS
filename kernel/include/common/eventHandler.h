//
// Created by 98max on 9/10/2023.
//

#ifndef MAXOS_COMMON_EVENTHANDLER_H
#define MAXOS_COMMON_EVENTHANDLER_H

#include <stdint.h>
#include <common/vector.h>

namespace maxOS{

    namespace common{


        template <typename EventType> class Event
        {
            public:
                Event(EventType type);
                ~Event();

                EventType type;
                union {
                    uint8_t* bufferValue;
                    uint32_t intValue;
                    bool boolValue;
                } returnValue;
        };

        template <typename EventType> class EventHandler
        {
            public:
                EventHandler();
                ~EventHandler();
                virtual Event<EventType>* onEvent(Event<EventType>* event);
        };

        template <typename EventType> class EventManager
        {
            protected:
                Vector<EventHandler<EventType>*> handlers;

            public:
                EventManager();
                ~EventManager();
                void connectEventHandler(EventHandler<EventType>* handler);
                void disconnectEventHandler(EventHandler<EventType>* handler);
                Vector<Event<EventType>*> raiseEvent(Event<EventType>* event);
        };


        /// ___________________________ Template Implementation ___________________________
        template<typename EventType> Event<EventType>::Event(EventType type) {
            this->type = type;
        }


        template<typename EventType> Event<EventType>::~Event() {

        }

        template<typename EventType> EventHandler<EventType>::EventHandler() {

        }


        template<typename EventType> EventHandler<EventType>::~EventHandler() {

        }

        /**
         * This function is called when an event is raised
         * @tparam EventType The type of event
         * @param event The event that was raised
         */
        template<typename EventType> Event<EventType>* EventHandler<EventType>::onEvent(Event<EventType> *event) {
            return event;
        }

        template<typename EventType> EventManager<EventType>::EventManager() {

        }

        template<typename EventType> EventManager<EventType>::~EventManager() {

        }


        /**
         * Connect an event handler to the event manager
         * @tparam EventType The type of event
         * @param handler The event handler to connect
         */
        template<typename EventType> void EventManager<EventType>::connectEventHandler(EventHandler<EventType> *handler) {
            // If the handler is already connected, return
            if(handlers.find(handler) != handlers.end()) {
                return;
            }

            handlers.pushBack(handler);

        }

        /**
         * Disconnect an event handler from the event manager
         * @tparam EventType The type of event
         * @param handler The event handler to disconnect
         */
        template<typename EventType> void EventManager<EventType>::disconnectEventHandler(EventHandler<EventType> *handler) {
            // If the handler is not connected, return
            if(handlers.find(handler) == handlers.end()) {
                return;
            }

            handlers.erase(handler);
        }

        /**
         * Raise an event
         * @tparam EventType The type of event
         * @param event The event to raise
         */
        template<typename EventType>  Vector<Event<EventType>*> EventManager<EventType>::raiseEvent(Event<EventType> *event) {


            // Store a list of the results of the event handlers
            Vector<Event<EventType>*> results;
            for(typename Vector<EventHandler<EventType>*>::iterator handler = handlers.begin(); handler != handlers.end(); ++handler) {
                results.pushBack((*handler)->onEvent(event));
            }
            return results;
        }
    }
}

#endif //MAXOS_COMMON_EVENTHANDLER_H
