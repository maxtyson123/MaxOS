/**
 * @file outputStream.cpp
 * @brief Implements an output stream for writing strings, characters, integers, and hex values to an output device
 *
 * @date 13th April 2023
 * @author Max Tyson
 */

#include <common/outputStream.h>

using namespace MaxOS;
using namespace MaxOS::common;

OutputStream::OutputStream()
: GenericOutputStream<string>::GenericOutputStream()
{

}

OutputStream::~OutputStream() = default;

/**
 * @brief Writes a newline to the output stream.
 */
void OutputStream::line_feed() {

	write_char('\n');

}

/**
 * @brief Writes a carriage return to the output stream.
 */
void OutputStream::carriageReturn() {

	// write the text representation of a carriage return to the output stream.
	write_char('\r');

}

/**
 * @brief Clears the output stream.
 */
void OutputStream::clear() {

}

/**
 * @brief Writes a string to the output stream.
 *
 * @param string_to_write The string to write to the output stream.
 */
void OutputStream::write(string string_to_write) {

	write(string_to_write.c_str());
}

/**
 * @brief Writes a string to the output stream.
 *
 * @param string_to_write The string to write to the output stream.
 */
void OutputStream::write(const char *string_to_write) {

	int i = 0;
	while (string_to_write[i] != '\0') {

		switch (string_to_write[i]) {

			case '\n':
				line_feed();
				break;

			case '\r':
				carriageReturn();
				break;

			case '\0':
				return;

			default:
				write_char(string_to_write[i]);
				break;

		}
		i++;
	}
}

/**
 * @brief Writes a character to the output stream.
 *
 * @param char_to_write The character to write to the output stream.
 */
void OutputStream::write_char(char) {

}

/**
 * @brief Writes an integer to the output stream.
 *
 * @param int_to_write The integer to write to the output stream.
 */
void OutputStream::write_int(int int_to_write) {

	write(itoa(10, int_to_write));

}

/**
 * @brief Writes a hex to the output stream.
 *
 * @param hex_to_write The hex to write to the output stream.
 */
void OutputStream::write_hex(uint64_t hex_to_write) {

	write(htoa(hex_to_write));

}

/**
 * @brief Writes a integer to the output stream.
 *
 * @param int_to_write The integer to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator<<(int int_to_write) {

	write_int(int_to_write);
	return *this;
}

/**
 * @brief Writes a hexadecimal to the output stream.
 *
 * @param hex_to_write The hex to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator<<(uint64_t hex_to_write) {

	write_hex(hex_to_write);
	return *this;
}

/**
 * @brief Writes a string to the output stream.
 *
 * @param string_to_write The string to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator<<(string string_to_write) {

	write(string_to_write);
	return *this;
}

/**
 * @brief Writes a character to the output stream.
 *
 * @param char_to_write The character to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator<<(char char_to_write) {

	write_char(char_to_write);
	return *this;
}

/**
 * @brief Writes a string to the output stream.
 *
 * @param string_to_write The string to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator<<(char const *string_to_write) {

	write(string_to_write);
	return *this;
}