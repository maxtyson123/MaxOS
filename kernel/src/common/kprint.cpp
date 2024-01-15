//
// Created by 98max on 15/01/2024.
//

#include <common/kprint.h>

using namespace MaxOS::drivers;

/**
 * @brief Prints a formatted string to the serial output
 *
 * ARGUMENTS:
 *  - %d for decimal
 *  - %x for hex
 *  - %u for unsigned decimal
 *  - %s for string
 *  - %0 to pad with 0s
 *  - %<number> how many characters to pad with
 *
 *
 * @param format The formatted string
 * @param ... The data to pass into the string
 */
void MaxOS::_kprintf(const char *format, ...){

    // Get the arguments
    char** arg = (char**) &format;

    // Current character and buffer
    int curr_char;
    char buffer[20];

    // Move to the first argument
    arg++;

    // Iterate through the format string
    while ((curr_char = *format++) != 0) {

        // If the current character is not a %, print it
        if (curr_char != '%') {
            SerialConsole::s_active_serial_console->put_character(curr_char);
        } else {

            // Pointers for the characters for the argument
            char* string_1, *string_2;
            int zero_padding = 0, feild_width = 0;

            // Get the next character
            curr_char = *format++;

            // Should the string be padded with 0s?
            if (curr_char == '0') {
                zero_padding = 1;
                curr_char = *format++;
            }

            // How many characters should the string be padded with?
            if (curr_char >= '0' && curr_char <= '9') {
                    feild_width = curr_char - '0';
                    curr_char = *format++;
            }

            // Handle the different types of arguments
            switch (curr_char) {

                case 'd':
                case 'u':
                case 'x':
                    _kitoa(buffer, curr_char, *((int*) arg++));
                    string_1 = buffer;
                    goto string;
                    break;

                // If the argument is a string set p to it
                case 's':
                  string_1 = *arg++;
                  if (!string_1)
                    string_1 = "(null)";

                string:
                    // Copy the string to the second string
                    for (string_2 = string_1; *string_2; string_2++);

                    // Print the string with padding
                    for (; string_2 < string_1 + feild_width; string_2++)
                      SerialConsole::s_active_serial_console->put_character(zero_padding ? '0' : ' ');

                    // Print the string
                    while (*string_1)
                      SerialConsole::s_active_serial_console->put_character(*string_1++);

                    break;

                default:
                    // Print the character
                    SerialConsole::s_active_serial_console->put_character(*((int*) arg++));
                    break;
            }
        }
    }

}

/**
 * @brief Converts integer to string
 *
 * @param buffer The buffer to store the converted string
 * @param base The base (d for decimal, x gor hex)
 * @param number The number to convert
 */
void MaxOS::_kitoa(char *buffer, int base, int number){

    // Pointer to the start of the buffer
    char* p = buffer;

    // Buffer to store the converted number
    char* p1, *p2;

    // Temporary variable to store the converted number
    unsigned long unsigned_number = number;

    // Divide the number by the base
    int divisor = 10;

    // If it is a negative decimal, add a minus sign
    if (base == 'd' && number < 0) {
            *p++ = '-';
            buffer++;
            unsigned_number = -number;
    }else if (base == 'x') {
        divisor = 16;
    }

    // Divide the number by the base until it is 0
    do {
            int remainder = unsigned_number % divisor;
            *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    } while (unsigned_number /= divisor);

    // Add a null terminator
    *p = 0;

    // Reverse the string
    p1 = buffer;
    p2 = p - 1;
    while (p1 < p2) {
            char tmp = *p1;
            *p1++ = *p2;
            *p2-- = tmp;
    }
}
