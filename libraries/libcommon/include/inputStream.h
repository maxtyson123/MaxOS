/**
 * @file inputStream.h
 * @brief Defines a generic input stream and related classes for handling input data streams
 *
 * @date 12th April 2023
 * @author Max Tyson
 *
 * @todo Move tempalte implmentations to a cpp file
 */

#ifndef MAXOS_COMMON_INPUTSTREAM_H
#define MAXOS_COMMON_INPUTSTREAM_H

#include <cstdint>
#include <vector.h>
#include <string.h>

namespace MaxOS::common {

	// Forward declaration
	template<class Type> class GenericInputStream;

	/**
	 * @class InputStreamEventHandler
	 * @brief Handles read and end of stream events from a set of streams
	 *
	 * @tparam Type The type of data the stream is handling
	 */
	template<class Type> class InputStreamEventHandler {
			friend class GenericInputStream<Type>;

		protected:
			common::Vector<GenericInputStream<Type>*> m_generic_input_streams;        ///< List of streams being observed by this handler
		public:
			InputStreamEventHandler();
			~InputStreamEventHandler();

			virtual void on_stream_read(Type);
			virtual void on_end_of_stream(GenericInputStream<Type>*);
	};

	/**
	 * @class InputStreamProcessor
	 * @brief Converts one stream data type into another
	 *
	 * @tparam Type
	 * @tparam ProcessorType
	 */
	template<class Type, class ProcessorType> class InputStreamProcessor : public InputStreamEventHandler<Type>, public GenericInputStream<ProcessorType> {
		public:
			InputStreamProcessor();
			explicit InputStreamProcessor(InputStreamEventHandler<ProcessorType>* generic_stream_event_handler);
			~InputStreamProcessor();

			void on_end_of_stream(GenericInputStream<Type>* stream) override;
	};


	/**
	 * @class InputStreamSocket
	 * @brief Passes read events on to the handlers
	 *
	 * @tparam Type The type of data the stream is handling
	 */
	template<class Type> class InputStreamSocket : public InputStreamProcessor<Type, Type> {
		public:
			InputStreamSocket();
			explicit InputStreamSocket(InputStreamEventHandler<Type>*);
			~InputStreamSocket();

			void on_stream_read(Type) override;
	};

	/**
	 * @class InputStreamBuffer
	 * @brief Buffers data from a stream and fires an event when a certain element is read
	 *
	 * @tparam Type The type of data the stream is handling
	 */
	template<class Type> class InputStreamBuffer : protected InputStreamProcessor<Type, Type*> {
		protected:
			Type m_buffer[10240];               ///<  The buffer to store data in
			int m_offset { 0 };                 ///<  The current position in the buffer
			Type m_event_fire_element;          ///<  The element that will cause the buffer to flush and fire an event
			Type m_termination_element;         ///<  The element that will cause the buffer to flush and end the stream

		public:
			InputStreamBuffer(Type event_fire_element, Type termination_element);
			~InputStreamBuffer();

			void on_stream_read(Type) override;
			void on_end_of_stream(GenericInputStream<Type>*) override;
			void flush();
	};

	/**
	 * @class GenericInputStream
	 * @brief Manages the connection of a stream to handlers
	 *
	 * @tparam Type The type of data the stream is handling
	 */
	template<class Type> class GenericInputStream {

		protected:
			common::Vector<InputStreamEventHandler<Type>*> m_input_stream_event_handlers; ///< List of streams being observed by this handler

		public:
			GenericInputStream();
			explicit GenericInputStream(InputStreamEventHandler<Type>*);
			~GenericInputStream();

			void connect_input_stream_event_handler(InputStreamEventHandler<Type>*);
			void disconnect_input_stream_event_handler(InputStreamEventHandler<Type>*);

	};

	/**
	 * @brief The ">>" operator is used to read data from a stream, it takes a stream to read from and an event handler to handle the data
	 *
	 * @tparam Type The type of data the stream is handling
	 * @param source The stream to read from
	 * @param input_stream_event_handler The event handler to handle the data
	 */
	template<class Type> void operator >>(GenericInputStream<Type>& source, InputStreamEventHandler<Type>& input_stream_event_handler);

	/**
	 * @brief The ">>" operator is used to read data from a stream, it takes a stream to read from and an InputStreamProcessor to process the data
	 *
	 * @tparam Type The type of data the stream is handling
	 * @tparam ProcessorType The type of data to convert the data into via the processor
	 * @param source The stream to read from
	 * @param processor The processor to process the data
	 * @return The processed stream
	 */
	template<class Type, class ProcessorType> GenericInputStream<ProcessorType>& operator >>(GenericInputStream<Type>& source, InputStreamProcessor<Type, ProcessorType>& processor);

	/**
	 * @class InputStream
	 * @brief A stream that handles strings
	 */
	class InputStream : public GenericInputStream<string> {
		public:
			explicit InputStream(InputStreamEventHandler<string>*);
	};


	///_______________________________________________TEMPLATES_________________________________________________________________///

	/**
	 * @brief Creates a new InputStreamProcessor
	 *
	 * @tparam Type the type of data the stream is handling
	 */
	template<class Type> InputStreamEventHandler<Type>::InputStreamEventHandler() = default;

	/**
	 * @brief Destroys the InputStreamProcessor and disconnects it from all streams
	 *
	 * @tparam Type the type of data the stream is handling
	 */
	template<class Type> InputStreamEventHandler<Type>::~InputStreamEventHandler() {

		// Disconnect the handler from all streams
		while(!m_generic_input_streams.empty())
			(*m_generic_input_streams.begin())->disconnect_input_stream_event_handler(this);

	}

	/**
	 * @brief Called when data is read from a stream (overridden by subclasses)
	 *
	 * @tparam Type the type of data the stream is handling
	 */
	template<class Type> void InputStreamEventHandler<Type>::on_stream_read(Type) {

	}

	/**
	 * @brief Called when a stream has finished. Removes the stream from the array of streams
	 *
	 * @tparam Type The type of data the stream is handling
	 * @param stream The stream that has finished
	 */
	template<class Type> void InputStreamEventHandler<Type>::on_end_of_stream(GenericInputStream<Type>* stream) {

		// Remove the stream
		m_generic_input_streams.erase(stream);
	}


	/**
	 * @brief Creates a new InputStreamProcessor
	 *
	 * @tparam Type The type of data the stream is handling
	 * @tparam ProcessorType The type of data to convert the data into via the processor
	 */
	template<class Type, class ProcessorType> InputStreamProcessor<Type, ProcessorType>::InputStreamProcessor()
			: InputStreamEventHandler<Type>(),
			GenericInputStream<ProcessorType>() {

	}

	/**
	 * @brief Creates a new InputStreamProcessor
	 *
	 * @tparam Type The type of data the stream is handling
	 * @tparam ProcessorType The type of data to convert the data into via the processor
	 * @param generic_stream_event_handler The event handler to handle the data
	 */
	template<class Type, class ProcessorType> InputStreamProcessor<Type, ProcessorType>::InputStreamProcessor(InputStreamEventHandler<ProcessorType>* generic_stream_event_handler)
			: InputStreamEventHandler<Type>(),
			GenericInputStream<ProcessorType>(generic_stream_event_handler) {

	}

	/**
	 * @brief Destroys the InputStreamProcessor
	 *
	 * @tparam Type  The type of data the stream is handling
	 * @tparam ProcessorType The type of data to convert the data into via the processor
	 */
	template<class Type, class ProcessorType> InputStreamProcessor<Type, ProcessorType>::~InputStreamProcessor() = default;

	/**
	 * @brief Called when a stream has finished. Passes the event on to the handlers and then removes the stream from the array of streams
	 *
	 * @tparam Type The type of data the stream is handling
	 * @tparam ProcessorType The type of data to convert the data into via the processor
	 * @param stream The stream that has finished
	 */
	template<class Type, class ProcessorType> void InputStreamProcessor<Type, ProcessorType>::on_end_of_stream(GenericInputStream<Type>* stream) {

		// Pass the end of stream event on to the handlers
		for(auto& event_handler : this->m_input_stream_event_handlers)
			event_handler->on_end_of_stream(this);

		// Remove the stream
		InputStreamEventHandler<Type>::on_end_of_stream(stream);

	}

	/**
	 * @brief Creates a new InputStreamSocket
	 *
	 * @tparam Type The type of data the stream is handling
	 */
	template<class Type> InputStreamSocket<Type>::InputStreamSocket() = default;

	/**
	 * @brief Creates a new InputStreamSocket and connects it to the handler
	 *
	 * @tparam Type The type of data the stream is handling (and the sockets processor type)
	 * @param processor_handler The handler to pass to InputStreamProcessor
	 */
	template<class Type> InputStreamSocket<Type>::InputStreamSocket(InputStreamEventHandler<Type>* processor_handler)
			: InputStreamProcessor<Type, Type>(processor_handler) {

	}

	/**
	 * @brief Destroys the InputStreamSocket
	 *
	 * @tparam Type The type of data the stream is handling
	 */
	template<class Type> InputStreamSocket<Type>::~InputStreamSocket() = default;

	/**
	 * @brief Called when data is read from a stream. Passes the event on to the internetProtocolHandlers
	 *
	 * @tparam Type The type of data the stream is handling
	 * @param read_element The element that was read from the stream
	 */
	template<class Type> void InputStreamSocket<Type>::on_stream_read(Type read_element) {

		// Pass the read event on to the handlers
		for(auto& event_handler : this->m_input_stream_event_handlers)
			event_handler->on_stream_read(read_element);

	}

	/**
	 * @brief Creates a new InputStreamBuffer
	 *
	 * @tparam Type The type of data the stream is handling
	 * @param event_fire_element The element to fire the event on
	 * @param termination_element The element that signifies the end of the part of the stream to be buffered (e.g a newline)
	 */
	template<class Type> InputStreamBuffer<Type>::InputStreamBuffer(Type event_fire_element, Type termination_element)
			: m_event_fire_element(event_fire_element),
			m_termination_element(termination_element) {

	}

	/**
	 * @brief Destroys the InputStreamBuffer
	 *
	 * @tparam Type The type of data the stream is handling
	 */
	template<class Type> InputStreamBuffer<Type>::~InputStreamBuffer() = default;

	/**
	 * @brief Called when data is read from a stream. Adds the data to the buffer and checks if the event should be fired
	 *
	 * @tparam Type
	 * @param read_element
	 */
	template<class Type> void InputStreamBuffer<Type>::on_stream_read(Type read_element) {

		// flush the buffer if the event fire element is read
		if(read_element == m_event_fire_element) {
			flush();
			return;
		}

		// Ensure the buffer is not full
		if(m_offset >= 10238) {
			flush();
		}

		// Add the element
		m_buffer[m_offset++] = read_element;

	}

	/**
	 * @brief Called when a stream has finished. Flushes the buffer if there is any data in it
	 *
	 * @tparam Type The type of data the stream is handling
	 * @param stream The stream that has finished
	 */
	template<class Type> void InputStreamBuffer<Type>::on_end_of_stream(GenericInputStream<Type>* stream) {

		// flush the buffer if there is any data in it
		if(m_offset > 0)
			flush();

		// Pass the event on to the handlers and remove the stream
		InputStreamProcessor<Type, Type*>::on_end_of_stream(stream);
	}

	/**
	 * @brief Flushes the buffer by adding the termination element and firing an on read event (NOTE: The buffer is not cleared after this just overwritten)
	 *
	 * @tparam Type The type of data the stream is handling
	 */
	template<class Type> void InputStreamBuffer<Type>::flush() {

		// Ensure the buffer is not empty
		if(m_offset == 0)
			return;

		// Add the termination element to the buffer
		m_buffer[m_offset] = m_termination_element;

		// Fire the on read event
		InputStreamProcessor<Type, Type*>::on_stream_read(m_buffer);

		// Reset the offset
		m_offset = 0;

	}

	/**
	 * @brief Creates a new GenericInputStream
	 *
	 * @tparam Type The type of data the stream is handling
	 */
	template<class Type> GenericInputStream<Type>::GenericInputStream() = default;

	/**
	 * @brief Creates a new GenericInputStream and connects it to the handler
	 *
	 * @tparam Type The type of data the stream is handling
	 * @param input_stream_event_handler The handler to handle the data
	 */
	template<class Type> GenericInputStream<Type>::GenericInputStream(InputStreamEventHandler<Type>* input_stream_event_handler) {

		// Connect the handler
		connect_input_stream_event_handler(input_stream_event_handler);
	}

	/**
	 * @brief Destroys the GenericInputStream and disconnects all handlers
	 *
	 * @tparam Type The type of data the stream is handling
	 */
	template<class Type> GenericInputStream<Type>::~GenericInputStream() {

		// Disconnect all handlers
		while(!m_input_stream_event_handlers.empty())
			disconnect_input_stream_event_handler(*(m_input_stream_event_handlers.begin()));
	}

	/**
	 * @brief Adds a inputStreamEventHandler to the list of internetProtocolHandlers
	 * @tparam Type The type of data the stream is handling
	 * @param input_stream_event_handler The inputStreamEventHandler to add
	 */
	template<class Type> void GenericInputStream<Type>::connect_input_stream_event_handler(InputStreamEventHandler<Type>* input_stream_event_handler) {

		// Don't add the handler if it is already connected
		if(m_input_stream_event_handlers.find(input_stream_event_handler) != m_input_stream_event_handlers.end())
			return;

		// Add the handler
		m_input_stream_event_handlers.push_back(input_stream_event_handler);

	}

	/**
	 * @brief Removes a handler from the list of handlers
	 *
	 * @tparam Type The type of data the stream is handling
	 * @param input_stream_event_handler The handler to remove
	 */
	template<class Type> void GenericInputStream<Type>::disconnect_input_stream_event_handler(InputStreamEventHandler<Type>* input_stream_event_handler) {

		// Don't remove the handler if it is not connected
		if(m_input_stream_event_handlers.find(input_stream_event_handler) == m_input_stream_event_handlers.end())
			return;

		// Remove the handler
		m_input_stream_event_handlers.erase(input_stream_event_handler);

		// Fire the end of stream event
		input_stream_event_handler->on_end_of_stream(this);

	}
}

#endif //MAXOS_COMMON_INPUTSTREAM_H
