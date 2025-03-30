//
// Created by 98max on 15/01/2024.
//

#include <common/kprint.h>


using namespace MaxOS::drivers;
using namespace MaxOS::system;
using namespace MaxOS::common;

/**
 * @brief Converts integer to string (does not handle negative numbers currently as int64_t wont hold higher half addresses which are deemed to be more important
 *
 * @param buffer The buffer to store the converted string
 * @param base The base of the number (10 for decimal, 16 for hex)
 * @param number The number to convert
 */
char* itoa(int base, uint64_t  number)
{
    static char buffer[50] = {0};
    int i = 49;
    bool isNegative = false;

    if (number == 0)
    {
        buffer[i] = '0';
        return &buffer[i];
    }

    //TODO: Handle negatives but then using an int64_t means we cant print higher half addresses in hex

    for (; number && i; --i, number /= base)
        buffer[i] = "0123456789ABCDEF"[number % base];

//    if (isNegative)
//    {
//        buffer[i] = '-';
//        return &buffer[i];
//    }

    return &buffer[i + 1];
}

/**
 * @brief Gets the length of a string
 *
 * @param str The string to get the length of
 * @return The length of the string
 */
int strlen(const char* str)
{
   int len = 0;
   for (; str[len] != '\0'; len++);
   return len;
}

#include <drivers/console/console.h>
using namespace MaxOS::drivers::console;

// Pointer to the active stream
extern ConsoleStream* active_stream;


/**
 * @brief Prints a character to the serial output if it is initialized
 *
 * @param c The character to print
 */
static void putchar (int c, bool cout_enabled = false)
{
    // Check if the active serial console is null
    if (SerialConsole::s_active_serial_console == nullptr)
            return;

    // Put the character
    SerialConsole::s_active_serial_console->put_character(c);

    // If there is an active stream write the character to the stream
    if(active_stream != nullptr && cout_enabled)
            active_stream->write_char(c);
}

/**
 * @brief Prints a debug prefix to the serial output
 */
void pre_kprintf(const char* file, int line, const char* func, uint8_t type)
{

  // Print the  colour
  const char* colour = "---------";
  switch (type) {

    // Log (yellow)
    case 0:
      colour = "\033[1;33m";
      break;

    // Assert, Panic (red)
    case 2:
    case 3:
      colour = "\033[0;31m";
      break;

    default:
      break;
  }

  for (int i = 0; i < strlen(colour); i++)
    putchar(colour[i]);

  putchar('[', type == 2);

  // File Output
  if(type == 0){

    // Print the file (but not the path)
    const char* file_str = file;
    for (int i = strlen(file) - 1; i >= 0; i--)
    {
      if (file[i] == '/')
      {
        file_str = &file[i + 1];
        break;
      }
    }\
    for (int j = 0; j < strlen(file_str); j++)
      putchar(file_str[j], type == 2);
    putchar(':', type == 2);

    // Print the line
    const char* line_str = itoa(10, line);
    for (int i = 0; i < strlen(line_str); i++)
      putchar(line_str[i], type == 2);
  }else if(type == 3){

    // Print the text
    const char* text = "FATAL ERROR IN {";
    for (int i = 0; i < strlen(text); i++)
      putchar(text[i], type == 2);

    // Print the function
    for (int i = 0; i < strlen(func); i++)
      putchar(func[i], type == 2);

    putchar('}', type == 2);
  }


  // Print the kernel footer
  const char* footer = "] \033[0m";
  for (int i = 0; i < strlen(footer); i++)
    putchar(footer[i], type == 2);

}

Spinlock kprintf_lock;

/**
 * @brief Prints a formatted string to the serial output (blocks until the other threads have finished printing their full message)
 *
 * ARGUMENTS:
 *  - %d for decimal
 *  - %x for hex
 *  - %u for unsigned decimal
 *  - %s for string
 *
 * @param format The formatted string
 * @param ... The data to pass into the string
 */
void _kprintf_internal(uint8_t type, const char* file, int line, const char* func, const char* format, ...)
{

  // wait for the lock
  kprintf_lock.lock();

  // Create a pointer to the data
  va_list parameters;
  va_start(parameters, format);

  // Print the header if the first two are not %h
  if(*format != '%' && *(format + 1) != 'h')
    pre_kprintf(file, line,func, type);


  // Loop through the format string
  for (; *format != '\0'; format++)
  {

    // If it is not a %, print the character
    if (*format != '%')
    {
      putchar(*format, type == 2);
      continue;
    }

    // Move to the next character
    format++;

    switch (*format)
    {
      case 'd':
      {
        // Print a decimal
        int number = va_arg (parameters, int);
        char* str = itoa(10, number);
        for (int i = 0; i < strlen(str); i++)
          putchar(str[i], type == 2);
        break;
      }
      case 'x':
      {
        // Print a hex
        uint64_t  number = va_arg (parameters, uint64_t );
        char* str = itoa(16, number);
        for (int i = 0; i < strlen(str); i++)
          putchar(str[i], type == 2);
        break;
      }
      case 's':
      {
        // Print a string
        char* str = va_arg (parameters, char*);
        for (int i = 0; i < strlen(str); i++)
          putchar(str[i], type == 2);
        break;
      }
    }
  }

  kprintf_lock.unlock();

  // If it is type 3, panic
  if(type == 3)
     CPU::PANIC("Check the serial output for more information");
}
