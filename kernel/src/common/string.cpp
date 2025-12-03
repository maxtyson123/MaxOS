/**
 * @file string.cpp
 * @brief Implementation of a simple String class for handling text of varying types and lengths
 *
 * @date 7th January 2024
 * @author Max Tyson
 */

#include <common/string.h>

using namespace MaxOS;

/**
 * @brief Construct a String, 0 length and only contains the null terminator
 */
String::String() {

	// String that only contains the null terminator
	m_length = 0;
	allocate_self();
	m_string[0] = '\0';


}

/**
 * @brief Constructs a String from a single character
 * @param c The character
 */
String::String(char c) {

	// Create the memory
	m_length = 1;
	allocate_self();

	// Store the char
	m_string[0] = c;
	m_string[m_length] = '\0';


}

/**
 * @brief Constructs a String from a pointer to an array of chars
 * @param string An array of chars, must be null terminated and of length less than 10,000
 */
String::String(char const* string) {

	// Get the length of the string, prevent longer than 10000 because this should mean something's gone wrong
	m_length = 0;
	while (string[m_length] != '\0' && m_length <= 10000)
		m_length++;
	allocate_self();

	// Copy the string
	for (size_t i = 0; i < m_length; i++)
		m_string[i] = string[i];

	// If the length is more than 10,000 Replace the end with a warning incase future use actually requires that
	const char* warning = "MAXOS: String length exceeded 10000 - might be a bug";
	if (m_length > 10000)
		for (int i = 0; i < 52; i++)
			m_string[m_length - 52 + i] = warning[i];

	m_string[m_length] = '\0';
}

/**
 * @brief Constructs a string from a an array of bytes (doesnt have to be null terminated)
 *
 * @param string The string bytes
 * @param length How large the string byte buffer is
 */
String::String(uint8_t const* string, int length) {
	// Allocate memory for the string (and null terminator)
	m_length = length;
	allocate_self();

	// Copy the string
	for (int i = 0; i < length; i++)
		m_string[i] = string[i];

	// Write the null terminator
	m_string[length] = '\0';
}

/**
 * @brief Constructs a string from an integer (must be base 10)
 *
 * @param value The integer value
 */
String::String(int value) {

	// Convert to a string
	const char* str = itoa(10, value);
	m_length = strlen(str);

	// Create space to store
	allocate_self();

	// Store the string
	for (size_t i = 0; i < m_length; i++)
		m_string[i] = str[i];
	m_string[m_length] = '\0';

}

/**
 * @brief Constructs a string from a hex value (Excludes 0x)
 *
 * @param value The hex value
 */
String::String(uint64_t value) {

	// Convert to a string
	const char* str = htoa(value);
	m_length = strlen(str);

	// Create space to store
	allocate_self();

	// Store the string
	for (size_t i = 0; i < m_length; i++)
		m_string[i] = str[i];
	m_string[m_length] = '\0';
}

/**
 * @brief Constructs a String from a boolean as "true" or "false"
 * @param value The bool value
 */
String::String(bool value) {

	if (value)
		*this = string("true");
	else
		*this = string("false");

}

/**
 * @brief Copy constructor for the string
 *
 * @param other String to copy from
 */
String::String(String const& other) {
	copy(other);
}

/**
 * @brief Destructor for the string, cleans up memory if needed
 */
String::~String() {

	// Free the memory
	if (!m_using_small)
		delete[] m_string;

}

/**
 * @brief Copies the other string
 *
 * @param other The other string
 */
void String::copy(String const& other) {

	// Allocate memory for the string (and null terminator)
	m_length = other.length();
	allocate_self();

	// Copy the string
	for (size_t i = 0; i < m_length; i++)
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
int String::lex_value(String const& string) {

	// Sum the ascii values of the characters in the string
	int sum = 0;
	for (size_t i = 0; i < string.length(); i++)
		sum += string[i];

	return sum;
}

/**
 * @brief Allocates memory for the string
 */
void String::allocate_self() {

	// Clear the old buffer if in use
	if (m_string && !m_using_small)
		delete[] m_string;

	// Try to use the small string buffer
	m_using_small = m_length + 1 <= MAX_STRING_SMALL_STORAGE;
	m_string = m_using_small ? m_small_string : new char[m_length + 1];

}

/**
 * @brief Sets the string to the other string
 *
 * @param other The string for this one to be updated to
 * @return String The string
 */
String& String::operator =(String const& other) {

	// Self assignment check
	if (this == &other)
		return *this;

	// Copy the other string
	copy(other);
	return *this;
}

/**
 * @brief The char pointer representation of the current string
 *
 * @return The char* string
 */
char* String::c_str() {

	return m_string;
}

/**
 * @brief Returns the string as a c string
 *
 * @return The string as an array of characters
 */
const char* String::c_str() const {

	return m_string;
}

/**
 * @brief Checks if the string starts with the other string (must contain the same characters in the same order)
 *
 * @param other The other string
 * @return True if the string starts with the other string, false otherwise
 */
bool String::starts_with(String const& other) {

	// Must at least be able to fit the other string
	if (m_length < other.length())
		return false;

	// Check if the string starts with the other string
	for (size_t i = 0; i < other.length(); i++)
		if (m_string[i] != other[i])
			return false;

	// No string left over to check so it must contain other
	return true;
}

/**
 * @brief Get a section of the string
 *
 * @param start The start of the substring
 * @param length The length of the substring
 * @return The substring or empty string if out of bounds
 */
String String::substring(size_t start, size_t length) const {

	// Ensure the start is within bounds
	if (start >= m_length)
		return { };

	// Ensure the length is within bounds
	if (start + length > m_length)
		return { };

	// Allocate memory for the substring (and null terminator)
	String substring;
	substring.m_length = length;
	substring.allocate_self();

	// Copy the substring
	for (size_t i = 0; i < length; i++)
		substring.m_string[i] = m_string[start + i];

	// Write the null terminator
	substring.m_string[length] = '\0';

	return substring;
}

/**
 * @brief Splits the string by the delimiter
 *
 * @param delimiter What to split the string by
 * @return A vector of strings that were split by the delimiter
 */
common::Vector<String> String::split(String const& delimiter) const {
	common::Vector<String> strings;

	// Go through the string and split it by the delimiter
	size_t start = 0;
	for (size_t i = 0; i <= m_length - delimiter.length(); i++) {

		// Check if matches at this position
		bool matches = true;
		for (size_t j = 0; j < delimiter.length(); j++)
			if (m_string[i + j] != delimiter[j]) {
				matches = false;
				break;
			}

		if (!matches)
			continue;

		// Add the splice of the string
		strings.push_back(substring(start, i - start));
		start = i + delimiter.length();
		i += delimiter.length() - 1;
	}

	// Add the last string to the vector
	strings.push_back(substring(start, m_length - start));

	return strings;
}

/**
 * @brief Returns the length of the string
 *
 * @param count_ansi Whether to count the ansi characters (default true)
 * @return The length of the string
 */
size_t String::length(bool count_ansi) const {

	// If ansi characters are not to be counted
	if (count_ansi)
		return m_length;

	// Calculate the length of the string without ansi characters
	int total_length = 0;
	int clean_length = 0;
	while (m_string[total_length] != '\0') {

		// If the character is an ansi character, skip it
		if (m_string[total_length] == '\033')
			while (m_string[total_length] != 'm')
				total_length++;

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
bool String::equals(String const& other) const {

	// Check if the lengths are equal
	if (m_length != other.length())
		return false;

	// Check if the characters are equal
	for (size_t i = 0; i < m_length; i++)
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
bool String::operator ==(String const& other) const {

	// Check if the strings are equal
	return equals(other);
}

/**
 * @brief Checks if one string is not equal to another
 *
 * @param other The other string
 * @return True if the strings are not equal, false otherwise
 */
bool String::operator !=(String const& other) const {

	// Self assignment check
	if (*this == other)
		return false;

	return !equals(other);
}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is less than the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is less than the other, false otherwise
 */
bool String::operator <(String const& other) const {

	return lex_value(*this) < lex_value(other);

}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is greater than the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is greater than the other, false otherwise
 */
bool String::operator >(String const& other) const {

	return lex_value(*this) > lex_value(other);

}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is less than or equal to the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is less than or equal to the other, false otherwise
 */
bool String::operator <=(String const& other) const {

	return lex_value(*this) <= lex_value(other);

}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is greater than or equal to the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is greater than or equal to the other, false otherwise
 */
bool String::operator >=(String const& other) const {

	return lex_value(*this) >= lex_value(other);

}

/**
 * @brief Adds the other string to the string
 *
 * @param other The other string
 * @return The concatenated string
 */
String String::operator +(String const& other) const {

	// The concatenated string
	String concatenated;
	concatenated.m_length = m_length + other.length();
	concatenated.allocate_self();

	// Copy the first string
	for (size_t i = 0; i < m_length; i++)
		concatenated.m_string[i] = m_string[i];

	// Copy the second string
	for (size_t i = 0; i < other.length(); i++)
		concatenated.m_string[m_length + i] = other[i];

	// Write the null terminator
	concatenated.m_string[concatenated.m_length] = '\0';

	// Return the concatenated string
	return concatenated;
}

/**
 * @brief Adds the other string to the string
 *
 * @param other The other string
 * @return The concatenated string
 */
String& String::operator +=(String const& other) {

	// Add the other string to this string
	String concatenated = *this + other;
	copy(concatenated);
	return *this;
}


/**
 * @brief Returns the character at the specified index
 *
 * @param index The index of the character
 * @return The character at the specified index
 */
char& String::operator [](size_t index) {
	return m_string[index];
}


/**
 * @brief Returns the character at the specified index
 *
 * @param index The index of the character
 * @return The character at the specified index
 */
char& String::operator [](size_t index) const {
	return m_string[index];
}

/**
 * @brief Returns the string repeated a number of times
 *
 * @param times The number of times to repeat the string
 * @return The string repeated a number of times
 */
String String::operator *(int times) const {

	// The repeated string
	String repeated;
	repeated.m_length = m_length * times;
	repeated.allocate_self();

	// Copy the string
	for (int i = 0; i < times; i++)
		for (size_t j = 0; j < m_length; j++)
			repeated.m_string[i * m_length + j] = m_string[j];

	// Write the null terminator
	repeated.m_string[repeated.m_length] = '\0';

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
String String::center(size_t width, char fill) const {

	// The number of characters to add
	size_t add = (width - m_length) / 2;

	// The centered string
	String centered;
	centered.m_length = width;
	centered.allocate_self();

	// Fill the right side (before)
	for (size_t i = 0; i < add; i++)
		centered.m_string[i] = fill;

	// Copy the string (middle)
	for (size_t i = 0; i < m_length; i++)
		centered.m_string[add + i] = m_string[i];

	// Fill the left side (after)
	for (size_t i = add + m_length; i < width; i++)
		centered.m_string[i] = fill;

	// Write the null terminator
	centered.m_string[width] = '\0';

	return centered;
}

/**
 * @brief Strips the string of whitespace
 *
 * @param strip_char The character to strip (default = ' ')
 * @return The stripped string (new string)
 */
String String::strip(char strip_char) const {

	// The stripped string
	String stripped;
	stripped.copy(*this);

	// Search from the back for the earliest non-whitespace character
	size_t end = m_length - 1;
	while (end >= 0 && (m_string[end] == strip_char || m_string[end] == '\n' || m_string[end] == '\t'))
		end--;

	// Make sure there is something to strip
	if (end < 0)
		return stripped;

	// Split the string to remove the end
	return stripped.substring(0, end + 1);
}

/**
 * @brief Creates a formated string. %s = string, %x = hex, %d = decimal
 *
 * @param format The string containing the format
 * @param ... The arguments to format into the string
 *
 * @return The string with the arguments formated inside
 */
String String::formatted(char const* format, ...) {

	// Create a pointer to the data
	va_list parameters;
	va_start(parameters, format);

	return formatted(format, parameters);

}

/**
 * @brief Creates a formated string. %s = string, %x = hex, %d = decimal
 *
 * @param format The string containing the format
 * @param parameters The arguments to format into the string
 *
 * @return The string with the arguments formated inside
 */
String String::formatted(char const* format, va_list parameters) {

	String out;

	// Loop through the format string
	for (; *format != '\0'; format++) {

		// If it is not a %, print the character
		if (*format != '%') {
			out += (string) (char) (*format);
			continue;
		}

		// Move to the next character
		format++;
		switch (*format) {
			case 'd': {
				// Print a decimal
				int number = va_arg (parameters, int);
				out += (string) (number);
				break;
			}
			case 'x': {
				// Print a hex
				uint64_t number = va_arg (parameters, uint64_t);
				out += (string) (number);
				break;
			}
			case 's': {
				// Print a string
				char* str = va_arg (parameters, char*);
				out += (string) (str);
				break;
			}
		}
	}

	return out;

}

/**
 * @brief Converts the string to an integer
 *
 * @return The integer value of the string or 0 if not a valid integer
 */
int String::to_int() const {

	int result = 0;
	bool is_negative = false;

	for (size_t i = 0; i < m_length; i++) {
		char c = m_string[i];

		// Check for negative sign
		if (i == 0 && c == '-') {
			is_negative = true;
			continue;
		}

		// Check if the character is a digit
		if (c < '0' || c > '9')
			return 0;

		result = result * 10 + (c - '0');
	}

}

/**
 * @brief Converts the string to an unsigned 64 bit integer
 *
 * @return The uint or 0 if not a valid uint
 */
uint64_t String::to_uint64() const {

	uint64_t result = 0;

	for (size_t i = 0; i < m_length; i++) {
		char c = m_string[i];

		// Check if the character is a digit
		if (c < '0' || c > '9')
			return 0;

		result = result * 10 + (c - '0');
	}

	return result;

}

/**
 * @brief Gets the length of a string
 *
 * @param str The string to get the length of
 * @return The length of the string
 */
int strlen(const char* str) {
	int len = 0;
	for (; str[len] != '\0'; len++);
	return len;
}

/**
 * @brief Converts integer to string
 *
 * @param base The base of the number (10 for decimal, 16 for hex)
 * @param number The number to convert
 *
 * @return The converted string
 */
char* itoa(int base, int64_t number) {

	// If there is no buffer use a default buffer
	static char buffer[50] = { 0 };

	int i = 49;
	bool is_negative = number < 0;

	// Null terminate the string
	buffer[i] = '\0';
	--i;

	if (number == 0) {
		buffer[i] = '0';
		return &buffer[i];
	}


	for (; number && i; --i, number /= base)
		buffer[i] = "0123456789ABCDEF"[number % base];

	if (is_negative) {
		buffer[i] = '-';
		return &buffer[i];
	}

	return &buffer[i + 1];
}

/**
 * @brief Converts hex to string
 *
 * @param number The number to convert
 * @return The converted string
 */
char* htoa(uint64_t number) {
	// If there is no buffer use a default buffer
	static char buffer[50] = { 0 };
	int i = 49;

	// Null terminate the string
	buffer[i] = '\0';
	--i;

	if (number == 0) {
		buffer[i] = '0';
		return &buffer[i];
	}

	for (; number && i; --i, number /= 16)
		buffer[i] = "0123456789ABCDEF"[number % 16];

	return &buffer[i + 1];
}

/**
 * @brief Checks if one string pointer is equal to another string pointer
 *
 * @param str1 The first string
 * @param str2 The second string
 * @return True if the strings are equal, false otherwise
 */
bool strcmp(char const* str1, char const* str2) {

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
bool strcmp(char const* str1, String const& str2) {

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
bool strcmp(String const& str1, char const* str2) {

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
bool strcmp(String const& str1, String const& str2) {

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
bool strncmp(char const* str1, char const* str2, int length) {

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
bool strncmp(char const* str1, String const& str2, int length) {

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
bool strncmp(String const& str1, char const* str2, int length) {

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
bool strncmp(String const& str1, String const& str2, int length) {

	// Use the other strncmp function
	return strncmp(str1.c_str(), str2.c_str(), length);
}

/**
 * @brief Append C-string to the StringBuilder
 *
 * @param str The C-string to append
 * @return The StringBuilder reference
 */
StringBuilder& StringBuilder::operator <<(char const* str) {
	out += string(str);
	return *this;
}

/**
 * @brief Append String to the StringBuilder
 *
 * @param other The String to append
 * @return The StringBuilder reference
 */
StringBuilder& StringBuilder::operator <<(String const& other) {
	out += other;
	return *this;
}

/**
 * @brief Append int to the StringBuilder
 *
 * @param value The int value to append
 * @return The StringBuilder reference
 */
StringBuilder& StringBuilder::operator <<(int value) {
	out += string(value);
	return *this;
}

/**
 * @brief Append a hex to the StringBuilder
 *
 * @param value The hex value to append
 * @return The StringBuilder reference
 */
StringBuilder& StringBuilder::operator <<(uint64_t value) {
	out += string(value);
	return *this;
}

/**
 * @brief Append bool to the StringBuilder
 *
 * @param value The bool value to append
 * @return The StringBuilder reference
 */
StringBuilder& StringBuilder::operator <<(bool value) {
	out += string(value);
	return *this;
}