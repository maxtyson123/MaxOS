//
// Created by 98max on 15/01/2024.
//

#include <common/kprint.h>


using namespace MaxOS::drivers;

/**
 * @brief Converts integer to string
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

    if (number < 0 && base == 10)
    {
        isNegative = true;
        number = -number;
    }

    for (; number && i; --i, number /= base)
        buffer[i] = "0123456789ABCDEF"[number % base];

    if (isNegative)
    {
        buffer[i] = '-';
        return &buffer[i];
    }

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

/**
 * @brief Prints a character to the serial output if it is initialized
 * @param c The character to print
 */
static void putchar (int c)
{
    // Check if the active serial console is null
    if (SerialConsole::s_active_serial_console == 0)
            return;

    // Put the character
    SerialConsole::s_active_serial_console->put_character(c);
}

/**
 * @ brief Prints a debug prefix (in yellow) to the serial output
 */
void pre_kprintf(const char* file, int line, const char* func, uint8_t type)
{

  // Print the  colour
  char* colour = "---------";
  switch (type) {

    // Log (yellow)
    case 0:
      colour = "\033[1;33m";
      break;

    // Assert (red)
    case 3:
      colour = "\033[1;31m";
      break;
  }

  for (int i = 0; i < strlen(colour); i++)
    putchar(colour[i]);

  putchar('[');

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
      putchar(file_str[j]);
    putchar(':');

    // Print the line
    const char* line_str = itoa(10, line);
    for (int i = 0; i < strlen(line_str); i++)
      putchar(line_str[i]);
  }else{

    // Print the text
    const char* text = "FATAL ERROR IN {";
    for (int i = 0; i < strlen(text); i++)
      putchar(text[i]);

    // Print the function
    for (int i = 0; i < strlen(func); i++)
      putchar(func[i]);

    putchar('}');
  }


  // Print the kernel footer
  const char* footer = "] \033[0m";
  for (int i = 0; i < strlen(footer); i++)
    putchar(footer[i]);

}

/**
 * @brief Prints a formatted string to the serial output
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
  // Create a pointer to the data
  va_list parameters;
  va_start(parameters, format);

  // Print the header
  if(*format != '\h')
    pre_kprintf(file, line,func, type);
  else
    format++;


  // Loop through the format string
  for (; *format != '\0'; format++)
  {

    // If it is not a %, print the character
    if (*format != '%')
    {
      putchar(*format);
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
          putchar(str[i]);
        break;
      }
      case 'x':
      {
        // Print a hex
        uint64_t  number = va_arg (parameters, uint64_t );
        char* str = itoa(16, number);
        for (int i = 0; i < strlen(str); i++)
          putchar(str[i]);
        break;
      }
      case 's':
      {
        // Print a string
        char* str = va_arg (parameters, char*);
        for (int i = 0; i < strlen(str); i++)
          putchar(str[i]);
        break;
      }
    }
  }
}