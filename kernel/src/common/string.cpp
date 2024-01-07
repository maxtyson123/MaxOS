//
// Created by 98max on 1/7/2024.
//
#include <common/string.h>

maxOS::String::String()
: m_string(nullptr),
  m_length(0)
{

}

maxOS::String::String(char const *string)
{

  // Get the length of the string
  m_length = 0;
  while (string[m_length] != '\0')
          m_length++;

  // Allocate memory for the string (and null terminator)
  m_string = new char[m_length + 1];

  // Copy the string
  for (int i = 0; i < m_length; i++)
          m_string[i] = string[i];

  // Write the null terminator
  m_string[m_length] = '\0';
}

maxOS::String::String(maxOS::String const &other) {
  // Copy the other string
  copy(other);
}


maxOS::String::~String() {

  // Free the memory
  delete[] m_string;

}

/**
 * @brief Copies the other string
 *
 * @param other The other string
 */
void maxOS::String::copy(maxOS::String const &other) {

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
int maxOS::String::lex_value(maxOS::String const &string) const {

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
 * @param other
 * @return String& The string
 */
maxOS::String &maxOS::String::operator = (maxOS::String const &other) {
    // Copy the other string
    copy(other);

    // Return the string
    return *this;

}

/**
 * @brief Returns the string as a c string
 *
 * @return The string as an array of characters
 */
const char* maxOS::String::c_str() const {

    // Return the string
    return m_string;

}

/**
 * @brief Returns the length of the string
 *
 * @return The length of the string
 */
int maxOS::String::length() const {

    // Return the length
    return m_length;

}

/**
 * @brief Checks if one string is equal to another
 *
 * @param other The other string
 * @return True if the strings are equal, false otherwise
 */
bool maxOS::String::operator == (maxOS::String const &other) const {

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
 * @brief Checks if one string is not equal to another
 * @param other The other string
 * @return True if the strings are not equal, false otherwise
 */
bool maxOS::String::operator != (maxOS::String const &other) const {

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
bool maxOS::String::operator < (maxOS::String const &other) const {

  // If the sum of this is less than the sum of the other, return true
  return lex_value(*this) < lex_value(other);

}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is greater than the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is greater than the other, false otherwise
 */
bool maxOS::String::operator > (maxOS::String const &other) const {

  // If the sum of this is greater than the sum of the other, return true
  return lex_value(*this) > lex_value(other);

}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is less than or equal to the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is less than or equal to the other, false otherwise
 */
bool maxOS::String::operator <= (maxOS::String const &other) const {

  // If the sum of this is less than or equal to the sum of the other, return true
  return lex_value(*this) <= lex_value(other);

}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is greater than or equal to the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is greater than or equal to the other, false otherwise
 */
bool maxOS::String::operator >= (maxOS::String const &other) const {

    // If the sum of this is greater than or equal to the sum of the other, return true
    return lex_value(*this) >= lex_value(other);

}

/**
 * @brief Adds the other string to the string
 *
 * @param other The other string
 * @return The concatenated string
 */
maxOS::String maxOS::String::operator + (maxOS::String const &other) const {

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
maxOS::String &maxOS::String::operator += (maxOS::String const &other) {

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
char maxOS::String::operator[](int index){

    return m_string[index];
}

/**
 * @brief Returns the character at the specified index
 *
 * @param index the index of the character
 * @return The character at the specified index
 */
char& maxOS::String::operator[](int index) const {

    return m_string[index];
}
maxOS::String::String(int value) {

    // The length of the string
    m_length = 0;

    // The value of the string
    int temp = value;

    // Get the length of the string
    while (temp != 0) {
       temp /= 10;
       m_length++;
    }

    // Allocate memory for the string (and null terminator)
    m_string = new char[m_length + 1];

    // Copy the string
    for (int i = m_length - 1; i >= 0; i--) {
       m_string[i] = (value % 10) + '0';
       value /= 10;
    }

    // Write the null terminator
    m_string[m_length] = '\0';
}