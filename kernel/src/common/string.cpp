//
// Created by 98max on 1/7/2024.
//
#include <common/string.h>

using namespace MaxOS;

String::String()
{

  // String that only contains the null terminator
  m_string = new char[1];
  m_string[0] = '\0';
  m_length = 1;

}

String::String(char const *string)
{

  // Get the length of the string, prevent longer than 10000 because this should mean somethings gone wrong
  m_length = 0;
  while (string[m_length] != '\0' && m_length <= 10000)
          m_length++;


  // Allocate memory for the string (and null terminator)
  m_string = new char[m_length + 1];

  // Copy the string
  for (int i = 0; i < m_length; i++)
          m_string[i] = string[i];

  // If the length is more than 10,000 Replace the end with a warning incase future use actually requires that
  const char* warning = "MAXOS: String length exceeded 10000 - might be a bug";
  if(m_length > 10000)
    for (int i = 0; i < 52; i++)
      m_string[m_length - 52 + i] = warning[i];


  // Write the null terminator
  m_string[m_length] = '\0';
}


String::String(int value) {

}


String::String(uint64_t value) {

}

String::String(float value) {



}


String::String(String const &other) {
  // Copy the other string
  copy(other);
}


String::~String() {

  // Free the memory
  delete[] m_string;

}

/**
 * @brief Copies the other string
 *
 * @param other The other string
 */
void String::copy(String const &other) {

  // Get the length of the string
  m_length = other.length();

  // Allocate memory for the string (and null terminator)
  m_string = new char[m_length + 1];

  // Copy the string
  for (int i = 0; i < m_length; i++)
    m_string[i] = other[i];

  // Write the null terminator
  m_string[m_length] = '\0';

}

/**
 * @brief Returns the sum of the ascii values of the characters in the string
 *
 * @param string The string
 * @return The sum of the ascii values of the characters in the string
 */
int String::lex_value(String const &string) {

    // The sum of the ascii values of the characters in the string
    int sum = 0;

    // Add the ascii values of the characters in the string
    for (int i = 0; i < string.length(); i++)
      sum += string[i];

    // Return the sum
    return sum;

}



/**
 * @brief Sets the string to the other string
 *
 * @param other The string for this one to be updated to
 * @return String& The string
 */
String &String::operator = (String const &other) {

    // Self assignment check
    if (this == &other)
        return *this;

    // Free the old memory
    delete[] m_string;

    // Copy the other string
    copy(other);

    // Return the string
    return *this;

}

/**
 * @brief The char pointer representation of the current string
 *
 * @return The char* string
 */
char* String::c_str() {

    // Return the string
    return m_string;

}

/**
 * @brief Returns the string as a c string
 *
 * @return The string as an array of characters
 */
const char* String::c_str() const {

    // Return the string
    return m_string;

}

/**
 * @brief Returns the length of the string
 *
 * @param count_ansi Whether to count the ansi characters (default true)
 * @return The length of the string
 */
int String::length(bool count_ansi) const {

    // If ansi characters are not to be counted
    if (count_ansi)
      return m_length;


    // Calculate the length of the string without ansi characters
    int total_length = 0;
    int clean_length = 0;
    while (m_string[total_length] != '\0'){

      // If the character is an ansi character, skip it
      if (m_string[total_length] == '\033'){
          while (m_string[total_length] != 'm')
            total_length++;
      }

      // Increment the length
      clean_length++;
      total_length++;
    }

    // Return the length
    return clean_length;
}

/**
 * @brief Checks if one string is equal to another
 *
 * @param other The other string
 * @return True if the strings are equal, false otherwise
 */
bool String::equals(String const &other) const {

  // Check if the lengths are equal
  if (m_length != other.length())
    return false;

  // Check if the characters are equal
  for (int i = 0; i < m_length; i++)
    if (m_string[i] != other[i])
      return false;

  // The strings are equal
  return true;

}

/**
 * @brief Checks if one string is equal to another
 *
 * @param other The other string
 * @return True if the strings are equal, false otherwise
 */
bool String::operator == (String const &other) const {

    // Check if the strings are equal
    return equals(other);

}

/**
 * @brief Checks if one string is not equal to another
 *
 * @param other The other string
 * @return True if the strings are not equal, false otherwise
 */
bool String::operator != (String const &other) const {

    // If the strings are equal, return false
    if (*this == other)
      return false;

    // The strings are not equal
    return true;

}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is less than the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is less than the other, false otherwise
 */
bool String::operator < (String const &other) const {

  // If the sum of this is less than the sum of the other, return true
  return lex_value(*this) < lex_value(other);

}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is greater than the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is greater than the other, false otherwise
 */
bool String::operator > (String const &other) const {

  // If the sum of this is greater than the sum of the other, return true
  return lex_value(*this) > lex_value(other);

}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is less than or equal to the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is less than or equal to the other, false otherwise
 */
bool String::operator <= (String const &other) const {

  // If the sum of this is less than or equal to the sum of the other, return true
  return lex_value(*this) <= lex_value(other);

}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is greater than or equal to the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is greater than or equal to the other, false otherwise
 */
bool String::operator >= (String const &other) const {

    // If the sum of this is greater than or equal to the sum of the other, return true
    return lex_value(*this) >= lex_value(other);

}

/**
 * @brief Adds the other string to the string
 *
 * @param other The other string
 * @return The concatenated string
 */
String String::operator + (String const &other) const {

  // The concatenated string
  String concatenated;

  // The length of the concatenated string
  int length = m_length + other.length();
  concatenated.m_length = length;

  // Allocate memory for the concatenated string (and null terminator)
  concatenated.m_string = new char[length + 1];

  // Copy the first string
  for (int i = 0; i < m_length; i++)
    concatenated.m_string[i] = m_string[i];

  // Copy the second string
  for(int i = 0; i < other.length(); i++)
    concatenated.m_string[m_length + i] = other[i];

  // Write the null terminator
  concatenated.m_string[length] = '\0';

  // Return the concatenated string
  return concatenated;
}

/**
 * @brief Adds the other string to the string
 *
 * @param other The other string
 * @return The concatenated string
 */
String &String::operator += (String const &other) {

    // The concatenated string
    String concatenated;

    // The length of the concatenated string
    int length = m_length + other.length();
    concatenated.m_length = length;

    // Allocate memory for the concatenated string (and null terminator)
    concatenated.m_string = new char[length + 1];

    // Copy the first string
    for (int i = 0; i < m_length; i++)
      concatenated.m_string[i] = m_string[i];

    // Copy the second string
    for (int i = 0; i < other.length(); i++)
      concatenated.m_string[m_length + i] = other[i];

    // Write the null terminator
    concatenated.m_string[length] = '\0';

    // Free the old memory
    delete[] m_string;

    // Copy the concatenated string
    copy(concatenated);

    // Return the concatenated string
    return *this;
}


/**
 * @brief Returns the character at the specified index
 *
 * @param index The index of the character
 * @return The character at the specified index
 */
char& String::operator[](int index) {
  return m_string[index];
}


/**
 * @brief Returns the character at the specified index
 *
 * @param index The index of the character
 * @return The character at the specified index
 */
char& String::operator[](int index) const {
    return m_string[index];
}

/**
 * @brief Returns the string repeated a number of times
 *
 * @param times The number of times to repeat the string
 * @return The string repeated a number of times
 */
String String::operator*(int times) const {

    // The repeated string
    String repeated;

    // The length of the repeated string
    int length = m_length * times;
    repeated.m_length = length;

    // Allocate memory for the repeated string (and null terminator)
    repeated.m_string = new char[length + 1];

    // Copy the string
    for (int i = 0; i < times; i++)
      for (int j = 0; j < m_length; j++)
        repeated.m_string[i * m_length + j] = m_string[j];

    // Write the null terminator
    repeated.m_string[length] = '\0';

    // Return the repeated string
    return repeated;

}

/**
 * @brief Centers the string in a specified width
 *
 * @param width The width of the string
 * @param fill The character to fill the string with
 * @return  The centered string
 */
String String::center(int width, char fill) const {

    // The centered string
    String centered;

    // The length of the string
    int length = m_length;

    // The number of characters to add
    int add = (width - length) / 2;

    // The length of the centered string
    centered.m_length = width;

    // Allocate memory for the centered string (and null terminator)
    centered.m_string = new char[width + 1];

    // Fill the string with the fill character
    for (int i = 0; i < add; i++)
        centered.m_string[i] = fill;

    // Copy the string
    for (int i = 0; i < length; i++)
        centered.m_string[add + i] = m_string[i];

    // Fill the string with the fill character
    for (int i = add + length; i < width; i++)
        centered.m_string[i] = fill;

    // Write the null terminator
    centered.m_string[width] = '\0';

    // Return the centered string
    return centered;

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
 * @brief Converts integer to string
 *
 * @param base The base of the number (10 for decimal, 16 for hex)
 * @param number The number to convert
 * @param buffer The buffer to store the converted string
 *
 * @return The converted string
 */
char* itoa(int base, int64_t number)
{

  // If there is no buffer use a default buffer
  static char buffer[50] = {0};

  int i = 49;
  bool isNegative = number < 0;

  if (number == 0)
  {
    buffer[i] = '0';
    return &buffer[i];
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
 * @brief Converts hex to string
 *
 * @param number The number to convert
 * @param buffer The buffer to store the converted string
 * @return The converted string
 */
char* htoa(uint64_t number)
{
  // If there is no buffer use a default buffer
  static char buffer[50] = {0};
  int i = 49;

  if (number == 0)
  {
    buffer[i] = '0';
    return &buffer[i];
  }

  for (; number && i; --i, number /= 16)
    buffer[i] = "0123456789ABCDEF"[number % 16];

  return &buffer[i + 1];
}

/**
 * @brief Converts a float to a string
 *
 * @param number The number to convert
 * @param buffer The buffer to store the converted string
 * @return The converted string
 */
char* ftoa(float number) {

  static char buffer[50];
  char* ptr = buffer;

  // Handle negative numbers.
  if (number < 0) {
    *ptr++ = '-';
    number = -number;
  }

  // Separate integer and fractional parts.
  int64_t intPart = (int64_t)number;
  float fraction = number - (float)intPart;

  // Convert integer part to string using itoa.
  char* intStr = itoa(10, intPart);
  while (*intStr) {
    *ptr++ = *intStr++;
  }

  // Add the decimal point.
  *ptr++ = '.';

  // Define the desired precision for the fractional part.
  const int precision = 6;

  // Multiply the fraction to shift the decimal digits into integer range.
  float fracValue = fraction;
  for (int i = 0; i < precision; i++) {
    fracValue *= 10.0f;
  }

  // Optionally, round the value.
  auto fracInt = (int64_t)(fracValue + 0.5f);

  // Convert the fractional part to string.
  char fracBuffer[50];
  char* fracStr = itoa(10, fracInt);

  // Ensure we have leading zeros if the fractional part doesn't produce enough digits.
  // Calculate length of the converted fractional string.
  int len = 0;
  for (char* p = fracStr; *p; p++) {
    len++;
  }
  for (int i = 0; i < precision - len; i++) {
    *ptr++ = '0';
  }

  // Copy the fractional digits.
  while (*fracStr) {
    *ptr++ = *fracStr++;
  }

  // Null-terminate the string.
  *ptr = '\0';

  return buffer;
}

/**
 * @brief Checks if one string pointer is equal to another string pointer
 *
 * @param str1 The first string
 * @param str2 The second string
 * @return True if the strings are equal, false otherwise
 */
bool strcmp(char const *str1, char const *str2) {

    // Check if the strings are equal
    for (int i = 0; str1[i] != '\0' || str2[i] != '\0'; i++)
      if (str1[i] != str2[i])
            return false;

    // The strings are equal
        return true;

}

/**
 * @brief Checks if one string pointer is equal to a String
 *
 * @param str1 The first string
 * @param str2 The second string
 * @return True if the strings are equal, false otherwise
 */
bool strcmp(char const *str1, String const &str2) {

  // Use the other strcmp function
  return strcmp(str1, str2.c_str());

}

/**
 * @brief Checks if one String is equal to a string pointer
 *
 * @param str1 The first string
 * @param str2 The second string
 * @return True if the strings are equal, false otherwise
 */
bool strcmp(String const &str1, char const *str2) {

    // Use the other strcmp function
    return strcmp(str1.c_str(), str2);
}

/**
 * @brief Checks if one String is equal to another String (better use is of "==")
 *
 * @param str1 The first string
 * @param str2 The second string
 * @return True if the strings are equal, false otherwise
 */
bool strcmp(String const &str1, String const &str2) {

  // Use the other strcmp function
  return strcmp(str1.c_str(), str2.c_str());

}

/**
 * @brief Checks if one string pointer is equal to another string pointer up to a specified length (each must be >= this length)
 *
 * @param str1 The first string
 * @param str2 The second string
 * @param length The length of the string to compare
 * @return True if the strings are equal, false otherwise
 */
bool strncmp(char const *str1, char const *str2, int length) {

  // Check if the strings are equal
  for (int i = 0; i < length; i++)
    if (str1[i] != str2[i])
        return false;

  // Strings are equal
  return true;

}

/**
 * @brief Checks if one string pointer is equal to another String up to a specified length (each must be >= this length)
 *
 * @param str1 The first string
 * @param str2 The second string
 * @param length The length of the string to compare
 * @return True if the strings are equal, false otherwise
 */
bool strncmp(char const *str1, String const &str2, int length) {

  // Use the other strncmp function
  return strncmp(str1, str2.c_str(), length);

}

/**
 * @brief Checks if one String is equal to another string pointer up to a specified length (each must be >= this length)
 *
 * @param str1 The first string
 * @param str2 The second string
 * @param length The length of the string to compare
 * @return True if the strings are equal, false otherwise
 */
bool strncmp(String const &str1, char const *str2, int length) {

  // Use the other strncmp function
  return strncmp(str1.c_str(), str2, length);

}

/**
 * @brief Checks if one String is equal to another String up to a specified length (each must be >= this length)
 *
 * @param str1 The first string
 * @param str2 The second string
 * @param length The length of the string to compare
 * @return True if the strings are equal, false otherwise
 */
bool strncmp(String const &str1, String const &str2, int length) {

  // Use the other strncmp function
  return strncmp(str1.c_str(), str2.c_str(), length);
}
