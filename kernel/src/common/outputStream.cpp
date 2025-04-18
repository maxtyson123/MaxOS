//
// Created by 98max on 13/04/2023.
//

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
void OutputStream::lineFeed() {

    // write the text representation of a newline to the output stream.
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
void OutputStream::write(const char* string_to_write){

  // Loop through the string
  int i = 0;
  while (string_to_write[i] != '\0') {

    // Switch on the current character
    switch (string_to_write[i]) {

      // If the current character is a newline
      case '\n':

        // write a newline to the output stream
        lineFeed();
        break;

      // If the current character is a carriage return
      case '\r':

        // write a carriage return to the output stream
        carriageReturn();
        break;


      // If the current character is a null terminator
      case '\0':
        return;

      // If the current character is any other character
      default:

        // write the current character to the output stream
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
 * @param writeHex The hex to write to the output stream.
 */
void OutputStream::write_hex(uint64_t hex_to_write) {

    write(htoa(hex_to_write));

}

/**
 * @brief Writes a interger to the output stream.
 *
 * @param int_to_write The integer to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator << (int int_to_write) {

    // Call the writeInt function to write the integer to the output stream
    write_int(int_to_write);

    // Return the output stream
    return *this;
}

/**
 * @brief Writes a hexadecimal to the output stream.
 *
 * @param hex_to_write The hex to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator << (uint64_t hex_to_write) {

    // Call the write_hex function to write the hex to the output stream
    write_hex(hex_to_write);

    // Return the output stream
    return *this;
}

/**
 * @brief Writes a string to the output stream.
 *
 * @param string_to_write The string to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator << (string string_to_write) {

    // Call the write function to write the string to the output stream
    write(string_to_write);

    // Return the output stream
    return *this;
}

/**
 * @brief Writes a character to the output stream.
 *
 * @param char_to_write The character to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator<<(char char_to_write) {

    // Call the writeChar function to write the character to the output stream
    write_char(char_to_write);

    // Return the output stream
    return *this;
}

/**
 * @brief Writes a string to the output stream.
 *
 * @param string_to_write The string to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator<<(char const *string_to_write) {

  // Call the write function to write the string to the output stream
  write(string_to_write);

  // Return the output stream
  return *this;
}

