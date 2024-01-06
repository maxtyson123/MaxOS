//
// Created by 98max on 13/04/2023.
//

#include <common/outputStream.h>

using namespace maxOS;
using namespace maxOS::common;

OutputStream::OutputStream()
: GenericOutputStream<string>::GenericOutputStream()
{

}

OutputStream::~OutputStream() {

}

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
 * @param string_to_write The string to write to the output stream.
 */
void OutputStream::write(string string_to_write) {

    // Loop until broken
    while (true) {

        // Switch on the current character
        switch (*string_to_write) {

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
                write_char(*string_to_write);
                break;

        }

        // Increment the pointer to the next character
        string_to_write++;

    }

}

/**
 * @brief Writes a character to the output stream.
 * @param char_to_write The character to write to the output stream.
 */
void OutputStream::write_char(char) {

}

/**
 * @brief Writes an integer to the output stream.
 * @param int_to_write The integer to write to the output stream.
 */
void OutputStream::write_int(int int_to_write) {

    // If the integer is 0 then write a 0 to the output stream and return as no more calculations are needed
    if (int_to_write == 0) {
      write_char('0');
        return;
    }

    // Store the int to write in a temporary variable
    int temp_write_int = int_to_write;

    // If the integer is negative then write a minus sign to the output stream and make the integer positive
    bool is_negative = int_to_write <= 0;
    if (is_negative) {
      write_char('-');
      temp_write_int = -temp_write_int;
    }

// Check how many digits the integer has by dividing it by 10 until it is 0 (rounded automatically)
    int digits = 0;
    int temp = temp_write_int; // Copy the value to avoid modifying the original
    do {
        temp /= 10;
        digits++;
    } while (temp != 0);

// Loop through each digit of the integer
    for (int current_digit = digits; current_digit > 0; --current_digit) {

        // Calculate the divisor to extract the current digit
        int divisor = 1;
        for (int i = 1; i < current_digit; i++) {
            divisor *= 10;
        }

        // Calculate the current digit
        int current_digit_value = (temp_write_int / divisor) % 10;

        // write the current digit to the output stream
        write_char('0' + current_digit_value);
    }

}

/**
 * @brief Writes a hex to the output stream.
 * @param writeHex The hex to write to the output stream.
 */
void OutputStream::write_hex(uint32_t hex_to_write) {

    // write the hex prefix to the output stream
    write("0x");

    // If the hex is 0 then write a 0 to the output stream and return as no more calculations are needed
    if (hex_to_write == 0) {
        write("0");
        return;
    }

    int length = 0;
    unsigned int ibak = hex_to_write;
    while(ibak > 0)
    {
        ibak /= 16;
        length++;
    }
    for(; length > 0; length--)
    {
        ibak = hex_to_write;
        for(int j = 1; j < length; j++)
            ibak /= 16;
        const string Hex = "0123456789ABCDEF";
        write_char(Hex[ibak % 16]);
    }

}

/**
 * @brief Writes a interger to the output stream.
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
 * @param hex_to_write The hex to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator << (uint32_t hex_to_write) {

    // Call the write_hex function to write the hex to the output stream
    write_hex(hex_to_write);

    // Return the output stream
    return *this;
}

/**
 * @brief Writes a string to the output stream.
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
 * @param char_to_write The character to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator<<(char char_to_write) {

    // Call the writeChar function to write the character to the output stream
    write_char(char_to_write);

    // Return the output stream
    return *this;

}
