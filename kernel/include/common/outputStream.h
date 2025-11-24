/**
 * @file outputStream.h
 * @brief Defines OutputStream and GenericOutputStream classes for writing data to streams
 *
 * @date 13th April 2023
 * @author Max Tyson
 */

#ifndef MAXOS_COMMON_OUTPUTSTREAM_H
#define MAXOS_COMMON_OUTPUTSTREAM_H

#include <cstdint>
#include <common/inputStream.h>
#include <common/string.h>


namespace MaxOS::common {

	/**
	 * @class GenericOutputStream
	 * @brief A stream that can be written to.
	 *
	 * @tparam Type The type of the elements that will be written to the stream.
	 */
	template<class Type> class GenericOutputStream : public InputStreamProcessor<Type, Type> {
		public:
			GenericOutputStream();
			~GenericOutputStream();

			void on_stream_read(Type) override;
			void on_end_of_stream(GenericInputStream<Type>*) override;

			virtual void write(Type);
			virtual void close();

			virtual GenericOutputStream<Type>& operator <<(Type);
	};

	/**
	 * @class OutputStream
	 * @brief A stream that strings can be written to.
	 */
	class OutputStream : public GenericOutputStream<string> {

		public:
			OutputStream();
			~OutputStream();

			virtual void line_feed();
			virtual void carriage_return();
			virtual void clear();

			void write(string string_to_write) override;
			void write(const char* string_to_write);
			virtual void write_char(char char_to_write);
			virtual void write_int(int int_to_write);
			virtual void write_hex(uint64_t hex_to_write);

			OutputStream& operator <<(string string_to_write) override;
			OutputStream& operator <<(const char* string_to_write);
			OutputStream& operator <<(int int_to_write);
			OutputStream& operator <<(uint64_t hex_to_write);
			OutputStream& operator <<(char char_to_write);
	};



	///__________________________________________Templates__________________________________________________///

	/**
	 * @brief Constructor of the GenericOutputStream class.
	 *
	 * @tparam Type The type of the elements that will be written to the stream.
	 */
	template<class Type> GenericOutputStream<Type>::GenericOutputStream() = default;

	/**
	 * @brief Destructor of the GenericOutputStream class.
	 *
	 * @tparam Type The type of the elements that will be written to the stream.
	 */
	template<class Type> GenericOutputStream<Type>::~GenericOutputStream() = default;

	/**
	 * @brief Writes the date that was read from the input stream to the output stream.
	 *
	 * @tparam Type The type of the elements that will be written to the stream.
	 * @param read_element The element that was read from the stream.
	 */
	template<class Type> void GenericOutputStream<Type>::on_stream_read(Type read_element) {

		// When something is read from the input stream, it is written to the output stream.
		write(read_element);

		// Pass the element to any handlers
		for(auto& input_stream_event_handler : this->m_input_stream_event_handlers)
			input_stream_event_handler->on_stream_read(read_element);


	}

	/**
	 * @brief Close the stream and remove it from the list of streams when the end of the stream is reached.
	 *
	 * @tparam Type The type of the elements that will be written to the stream.
	 * @param stream The stream that reached the end.
	 */
	template<class Type> void GenericOutputStream<Type>::on_end_of_stream(GenericInputStream<Type>* stream) {

		// Close the stream.
		close();

		// Pass the event to any handlers
		for(auto& input_stream_event_handler : this->m_input_stream_event_handlers)
			input_stream_event_handler->on_end_of_stream(stream);

		// Remove the stream from the list of streams.
		InputStreamProcessor<Type, Type>::on_end_of_stream(stream);

	}

	/**
	 * @brief write an element to the stream.
	 *
	 * @tparam Type The type of the elements that will be written to the stream.
	 */
	template<class Type> void GenericOutputStream<Type>::write(Type) {

	}

	/**
	 * @brief Close the stream.
	 *
	 * @tparam Type The type of the elements that will be written to the stream.
	 */
	template<class Type> void GenericOutputStream<Type>::close() {

	}

	/**
	 * @brief Overload the << operator to write an element to the stream.
	 *
	 * @tparam Type The type of the elements that will be written to the stream.
	 * @param write_element The element that will be written to the stream.
	 * @return The stream
	 */
	template<class Type> GenericOutputStream<Type>& GenericOutputStream<Type>::operator <<(Type write_element) {

		// write the element to the stream.
		write(write_element);

		// Return the stream.
		return *this;
	}
}

#endif //MAXOS_COMMON_OUTPUTSTREAM_H
