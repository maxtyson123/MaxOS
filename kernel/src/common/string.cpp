//
// Created by 98max on 1/7/2024.
//
#include <common/string.h>

using namespace MaxOS;

String::String() {

	// String that only contains the null terminator
	allocate_self();
	m_string[0] = '\0';
	m_length = 0;

}

String::String(char c) {

	// Create the memory
	m_length = 1;
	allocate_self();

	// Store the char
	m_string[0] = c;
	m_string[m_length] = '\0';


}

String::String(char const *string) {

	// Get the length of the string, prevent longer than 10000 because this should mean something's gone wrong
	m_length = 0;
	while (string[m_length] != '\0' && m_length <= 10000)
		m_length++;
	allocate_self();

	// Copy the string
	for (int i = 0; i < m_length; i++)
		m_string[i] = string[i];

	// If the length is more than 10,000 Replace the end with a warning incase future use actually requires that
	const char *warning = "MAXOS: String length exceeded 10000 - might be a bug";
	if (m_length > 10000)
		for (int i = 0; i < 52; i++)
			m_string[m_length - 52 + i] = warning[i];

	m_string[m_length] = '\0';
}

String::String(uint8_t const *string, int length) {
	// Allocate memory for the string (and null terminator)
	m_length = length;
	allocate_self();

	// Copy the string
	for (int i = 0; i < length; i++)
		m_string[i] = string[i];

	// Write the null terminator
	m_string[length] = '\0';
}

String::String(int value) {

	// Convert to a string
	const char *str = itoa(10, value);
	m_length = strlen(str);

	// Create space to store
	allocate_self();

	// Store the string
	for (int i = 0; i < m_length; i++)
		m_string[i] = str[i];
	m_string[m_length] = '\0';

}

/**
 * @brief Constructs a string from a hex value (Excludes 0x____)
 *
 * @param value
 */
String::String(uint64_t value) {

	// Convert to a string
	const char *str = htoa(value);
	m_length = strlen(str);

	// Create space to store
	allocate_self();

	// Store the string
	for (int i = 0; i < m_length; i++)
		m_string[i] = str[i];
	m_string[m_length] = '\0';
}

String::String(float value) {

	// Convert to a string
	const char *str = ftoa(value);
	m_length = strlen(str);

	// Create space to store
	allocate_self();

	// Store the string
	for (int i = 0; i < m_length; i++)
		m_string[i] = str[i];
	m_string[m_length] = '\0';

}

/**
 * @brief Copy constructor for the string
 *
 * @param other String to copy from
 */
String::String(String const &other) {
	copy(other);
}


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
void String::copy(String const &other) {

	// Allocate memory for the string (and null terminator)
	m_length = other.length();
	allocate_self();

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

	// Sum the ascii values of the characters in the string
	int sum = 0;
	for (int i = 0; i < string.length(); i++)
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
String &String::operator=(String const &other) {

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
char *String::c_str() {

	return m_string;
}

/**
 * @brief Returns the string as a c string
 *
 * @return The string as an array of characters
 */
const char *String::c_str() const {

	return m_string;
}

/**
 * @brief Checks if the string starts with the other string (must contain the same characters in the same order)
 *
 * @param other The other string
 * @return True if the string starts with the other string, false otherwise
 */
bool String::starts_with(String const &other) {

	// Must at least be able to fit the other string
	if (m_length < other.length())
		return false;

	// Check if the string starts with the other string
	for (int i = 0; i < other.length(); i++)
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
String String::substring(int start, int length) const {

	// Ensure the start is within bounds
	if (start < 0 || start >= m_length)
		return {};

	// Ensure the length is within bounds
	if (length < 0 || start + length > m_length)
		return {};

	// Allocate memory for the substring (and null terminator)
	String substring;
	substring.m_length = length;
	substring.allocate_self();

	// Copy the substring
	for (int i = 0; i < length; i++)
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
common::Vector<String> String::split(String const &delimiter) const {
	common::Vector<String> strings;

	// Go through the string and split it by the delimiter
	int start = 0;
	for (int i = 0; i <= m_length - delimiter.length(); i++) {

		// Check if matches at this position
		bool matches = true;
		for (int j = 0; j < delimiter.length(); j++)
			if (m_string[i + j] != delimiter[j]) {
				matches = false;
				break;
			}

		if(!matches)
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
int String::length(bool count_ansi) const {

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
bool String::operator==(String const &other) const {

	// Check if the strings are equal
	return equals(other);
}

/**
 * @brief Checks if one string is not equal to another
 *
 * @param other The other string
 * @return True if the strings are not equal, false otherwise
 */
bool String::operator!=(String const &other) const {

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
bool String::operator<(String const &other) const {

	return lex_value(*this) < lex_value(other);

}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is greater than the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is greater than the other, false otherwise
 */
bool String::operator>(String const &other) const {

	return lex_value(*this) > lex_value(other);

}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is less than or equal to the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is less than or equal to the other, false otherwise
 */
bool String::operator<=(String const &other) const {

	return lex_value(*this) <= lex_value(other);

}

/**
 * @brief Checks if the sum of the ascii values of the characters in the string is greater than or equal to the sum of the ascii values of the characters in the other string
 *
 * @param other The other string
 * @return True if the string is greater than or equal to the other, false otherwise
 */
bool String::operator>=(String const &other) const {

	return lex_value(*this) >= lex_value(other);

}

/**
 * @brief Adds the other string to the string
 *
 * @param other The other string
 * @return The concatenated string
 */
String String::operator+(String const &other) const {

	// The concatenated string
	String concatenated;
	concatenated.m_length = m_length + other.length();
	concatenated.allocate_self();

	// Copy the first string
	for (int i = 0; i < m_length; i++)
		concatenated.m_string[i] = m_string[i];

	// Copy the second string
	for (int i = 0; i < other.length(); i++)
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
String &String::operator+=(String const &other) {

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
char &String::operator[](int index) {
	return m_string[index];
}


/**
 * @brief Returns the character at the specified index
 *
 * @param index The index of the character
 * @return The character at the specified index
 */
char &String::operator[](int index) const {
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
	repeated.m_length = m_length * times;
	repeated.allocate_self();

	// Copy the string
	for (int i = 0; i < times; i++)
		for (int j = 0; j < m_length; j++)
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
String String::center(int width, char fill) const {

	// The number of characters to add
	int add = (width - m_length) / 2;

	// The centered string
	String centered;
	centered.m_length = width;
	centered.allocate_self();

	// Fill the right side (before)
	for (int i = 0; i < add; i++)
		centered.m_string[i] = fill;

	// Copy the string (middle)
	for (int i = 0; i < m_length; i++)
		centered.m_string[add + i] = m_string[i];

	// Fill the left side (after)
	for (int i = add + m_length; i < width; i++)
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
	int end = m_length - 1;
	while (end >= 0 && (m_string[end] == strip_char || m_string[end] == '\n' || m_string[end] == '\t'))
		end--;

	// Make sure there is something to strip
	if (end < 0)
		return stripped;

	// Split the string to remove the end
	return stripped.substring(0, end + 1);
}

String::String(bool value) {

	if(value)
		*this = string("true");
	else
		*this = string("false");

}

String String::formatted(char const* format, ...) {

	// Create a pointer to the data
	va_list parameters;
	va_start(parameters, format);

	return formatted(format, parameters);

}

String String::formatted(char const* format, va_list parameters) {

	String out;

	// Loop through the format string
	for (; *format != '\0'; format++) {

		// If it is not a %, print the character
		if (*format != '%') {
			out += (string)(char)(*format);
			continue;
		}

		// Move to the next character
		format++;
		switch (*format) {
			case 'd': {
				// Print a decimal
				int number = va_arg (parameters, int);
				out += (string)(number);
				break;
			}
			case 'x': {
				// Print a hex
				uint64_t number = va_arg (parameters, uint64_t);
				out += (string)(number);
				break;
			}
			case 's': {
				// Print a string
				char *str = va_arg (parameters, char*);
				out += (string)(str);
				break;
			}
		}
	}

	return out;

}

/**
 * @brief Gets the length of a string
 *
 * @param str The string to get the length of
 * @return The length of the string
 */
int strlen(const char *str) {
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
char *itoa(int base, int64_t number) {

	// If there is no buffer use a default buffer
	static char buffer[50] = {0};

	int i = 49;
	bool isNegative = number < 0;

	if (number == 0) {
		buffer[i] = '0';
		return &buffer[i];
	}


	for (; number && i; --i, number /= base)
		buffer[i] = "0123456789ABCDEF"[number % base];

	if (isNegative) {
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
char *htoa(uint64_t number) {
	// If there is no buffer use a default buffer
	static char buffer[50] = {0};
	int i = 49;

	if (number == 0) {
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
char *ftoa(float number) {

	static char buffer[50];
	char *ptr = buffer;

	// Handle negative numbers.
	if (number < 0) {
		*ptr++ = '-';
		number = -number;
	}

	// Separate integer and fractional parts.
	int64_t intPart = (int64_t) number;
	float fraction = number - (float) intPart;

	// Convert integer part to string using itoa.
	char *intStr = itoa(10, intPart);
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
	auto fracInt = (int64_t) (fracValue + 0.5f);

	// Convert the fractional part to string.
	char fracBuffer[50];
	char *fracStr = itoa(10, fracInt);

	// Ensure we have leading zeros if the fractional part doesn't produce enough digits.
	// Calculate length of the converted fractional string.
	int len = 0;
	for (char *p = fracStr; *p; p++) {
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
