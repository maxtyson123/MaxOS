//
// Created by 98max on 9/10/2023.
//

#ifndef MAXOS_COMMON_EVENTHANDLER_H
#define MAXOS_COMMON_EVENTHANDLER_H

#include <stdint.h>
#include <common/vector.h>

namespace MaxOS{

    namespace common{


        /**
         * @class Event
         * @brief Used to store information about an event, has a type and a return value
         *
         * @tparam EventType The type of event
         */
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
                } return_value;
        };

        /**
         * @class EventHandler
         * @brief Used to handle an event
         *
         * @tparam EventType The type of event
         */
        template <typename EventType> class EventHandler
        {
            public:
                EventHandler();
                ~EventHandler();
                virtual Event<EventType>* on_event(Event<EventType>* event);
        };

        /**
         * @class EventManager
         * @brief Manages the m_handlers for a type of event, raises events and calls the m_handlers
         *
         * @tparam EventType The type of event
         */
        template <typename EventType> class EventManager
        {
            protected:
                Vector<EventHandler<EventType>*> m_handlers;

            public:
                EventManager();
                ~EventManager();
                void connect_event_handler(EventHandler<EventType>* handler);
                void disconnect_event_handler(EventHandler<EventType>* handler);
                Vector<Event<EventType>*> raise_event(Event<EventType>* event);
        };


        /// ___________________________ Template Implementation ___________________________
        template<typename EventType> Event<EventType>::Event(EventType type) {
            this->type = type;
        }


        template<typename EventType> Event<EventType>::~Event() = default;

        template<typename EventType> EventHandler<EventType>::EventHandler() = default;


        template<typename EventType> EventHandler<EventType>::~EventHandler() = default;

        /**
         * @brief This function is called when an event is raised
         *
         * @tparam EventType The type of event
         * @param event The event that was raised
         */
        template<typename EventType> Event<EventType>* EventHandler<EventType>::on_event(Event<EventType>* event) {
            return event;
        }

        template<typename EventType> EventManager<EventType>::EventManager() = default;

        template<typename EventType> EventManager<EventType>::~EventManager() = default;


        /**
         * @brief Connect an event handler to the event manager if it is not already connected
         *
         * @tparam EventType The type of event
         * @param handler The event handler to connect
         */
        template<typename EventType> void EventManager<EventType>::connect_event_handler(EventHandler<EventType>* handler) {
            // If the handler is already connected, return
            if(m_handlers.find(handler) != m_handlers.end()) {
                return;
            }

            m_handlers.push_back(handler);

        }

        /**
         * @brief Disconnect an event handler from the event manager if it is connected
         *
         * @tparam EventType The type of event
         * @param handler The event handler to disconnect
         */
        template<typename EventType> void EventManager<EventType>::disconnect_event_handler(EventHandler<EventType>* handler) {
            // If the handler is not connected, return
            if(m_handlers.find(handler) == m_handlers.end()) {
                return;
            }

            m_handlers.erase(handler);
        }

        /**
         * @brief Calls the on_event function of all the event m_handlers connected to the event manager and returns a list of the results
         *
         * @tparam EventType The type of event
         * @param event The event to raise
         * @return A list of the results of the event m_handlers
         */
        template<typename EventType>  Vector<Event<EventType>*> EventManager<EventType>::raise_event(Event<EventType>* event) {


            // Store a list of the results of the event handlers
            Vector<Event<EventType>*> results;
            for(auto& handler : m_handlers) {
                results.push_back(handler->on_event(event));
            }

            // Free the memory used by the event
            delete event;

            return results;
        }
    }
}

#endif //MAXOS_COMMON_EVENTHANDLER_H
