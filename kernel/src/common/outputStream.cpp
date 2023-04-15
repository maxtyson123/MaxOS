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
 * @details Writes a newline to the output stream.
 */
void OutputStream::lineFeed() {

    // Write the text representation of a newline to the output stream.
    writeChar('\n');

}

/**
 * @details Writes a carriage return to the output stream.
 */
void OutputStream::carriageReturn() {

    // Write the text representation of a carriage return to the output stream.
    writeChar('\r');

}

/**
 * @details Clears the output stream.
 */
void OutputStream::clear() {

}

/**
 * @details Writes a string to the output stream.
 * @param stringToWrite The string to write to the output stream.
 */
void OutputStream::write(string stringToWrite) {

    // Loop until broken
    while (true) {

        // Switch on the current character
        switch (*stringToWrite) {

            // If the current character is a newline
            case '\n':

                // Write a newline to the output stream
                lineFeed();
                break;

            // If the current character is a carriage return
            case '\r':

                // Write a carriage return to the output stream
                carriageReturn();
                break;

            // If the current character is a null terminator
            case '\0':
                return;

            // If the current character is any other character
            default:

                // Write the current character to the output stream
                writeChar(*stringToWrite);
                break;

        }

        // Increment the pointer to the next character
        stringToWrite++;

    }

}

/**
 * @details Writes a character to the output stream.
 * @param charToWrite The character to write to the output stream.
 */
void OutputStream::writeChar(char charToWrite) {

}

/**
 * @details Writes an integer to the output stream.
 * @param intToWrite The integer to write to the output stream.
 */
void OutputStream::writeInt(int intToWrite) {

    // If the integer is 0 then write a 0 to the output stream and return as no more calculations are needed
    if (intToWrite == 0) {
        writeChar('0');
        return;
    }

    // Store the int to write in a temporary variable
    int tempWriteInt = intToWrite;

    // If the integer is negative then write a minus sign to the output stream and make the integer positive
    bool isNegative = intToWrite <= 0;
    if (isNegative) {
        writeChar('-');
        tempWriteInt = -tempWriteInt;
    }

    // Check how many digits the integer has by dividing it by 10 until it is 0 (rounded automatically)
    int digits = 0;
    for (int i = tempWriteInt; i > 0; i /= 10) {
        digits++;
    }

    // Loop through each digit of the integer
    for (int currentDigit = digits; currentDigit > 0; --currentDigit) {  // (de-increment as writing from left to right)

        // Loop through the digits of the integer until the current digit is reached, dividing the integer by 10 each time to get the next digit
        for(int digit = 1; digit < currentDigit; digit++)
            tempWriteInt /= 10;

        // Write the current digit to the output stream (divisor by 10 to get the digit in the ones place)
        writeChar('0' + tempWriteInt % 10);

    }


}

/**
 * @details Writes a hex to the output stream.
 * @param writeHex The hex to write to the output stream.
 */
void OutputStream::writeHex(uint32_t hexToWrite) {

    // Write the hex prefix to the output stream
    write("0x");

    // If the hex is 0 then write a 0 to the output stream and return as no more calculations are needed
    if (hexToWrite == 0) {
        write("0");
        return;
    }

    // Store the hex to write in a temporary variable
    uint32_t tempWriteHex = hexToWrite;

    // Check how many digits the hex has by dividing it by 16 until it is 0 (rounded automatically)
    int digits = 0;
    for (uint32_t i = tempWriteHex; i > 0; i /= 16) {
        digits++;
    }

    // Loop through each digit of the hex
    for (int currentDigit = tempWriteHex; currentDigit > 0; --currentDigit) {

        // Loop through the digits of the integer until the current digit is reached, dividing the integer by 10 each time to get the next digit
        for(int digit = 1; digit < currentDigit; digit++)
            tempWriteHex /= 10;

        // Calculate the current digit
        uint8_t currentHexDigit = tempWriteHex % 16;

        // Write the current digit to the output stream (pass the digit into the hex character array to get the character representation)
        writeChar("0123456789ABCDEF"[currentHexDigit]);
    }

}

/**
 * @details Writes a interger to the output stream.
 * @param intToWrite The integer to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator << (int intToWrite) {

    // Call the writeInt function to write the integer to the output stream
    writeInt(intToWrite);

    // Return the output stream
    return *this;
}

/**
 * @details Writes a hexadecimal to the output stream.
 * @param hexToWrite The hex to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator << (uint32_t hexToWrite) {

    // Call the writeHex function to write the hex to the output stream
    writeHex(hexToWrite);

    // Return the output stream
    return *this;
}

/**
 * @details Writes a string to the output stream.
 * @param stringToWrite The string to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator << (string stringToWrite) {

    // Call the write function to write the string to the output stream
    write(stringToWrite);

    // Return the output stream
    return *this;
}

/**
 * @details Writes a character to the output stream.
 * @param charToWrite The character to write to the output stream.
 * @return The output stream.
 */
OutputStream &OutputStream::operator<<(char charToWrite) {

    // Call the writeChar function to write the character to the output stream
    writeChar(charToWrite);

    // Return the output stream
    return *this;

}