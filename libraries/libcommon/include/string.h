/**
 * @file string.h
 * @brief Defines a String class for dynamically sized strings with various operations
 *
 * @date 12th April 2023
 * @author Max Tyson
 */

#ifndef MAXOS_STRING_H
#define MAXOS_STRING_H

#include <cstdint>

#include <vector.h>
#include <stdarg.h>

namespace MaxOS {

	/// How many characters can be stored in the small string optimization array
	constexpr int MAX_STRING_SMALL_STORAGE = 0x99;

	/**
	 * @class String
	 * @brief Dynamically sized string with various operations
	 */
	typedef class String {
		private:
			char* m_string = nullptr;
			size_t m_length = 0;           ///< Length of the string (not including null terminator)

			char m_small_string[MAX_STRING_SMALL_STORAGE] = { 0 };
			bool m_using_small = true;

			[[nodiscard]] static int lex_value(String const& other);
			void allocate_self();

		public:

			String();
			String(char c);
			String(char const* string);
			String(uint8_t const* string, int length);
			String(String const& other);
			String(int value);
			String(uint64_t value);
			String(bool value);
			~String();

			void copy(String const& other);

			static String formatted(char const* format, ...);
			static String formatted(char const* format, va_list parameters);

			[[nodiscard]] size_t length(bool count_ansi = true) const;
			[[nodiscard]] char* c_str();
			[[nodiscard]] const char* c_str() const;

			bool starts_with(String const& other);
			[[nodiscard]] String substring(size_t start, size_t length) const;

			[[nodiscard]] common::Vector<String> split(String const& delimiter) const;
			[[nodiscard]] String strip(char strip_char = ' ') const;


			[[nodiscard]] String center(size_t width, char fill = ' ') const;

			// Operator functions
			[[nodiscard]] bool equals(String const& other) const;

			// Number conversion
			[[nodiscard]] int to_int() const;
			[[nodiscard]] uint64_t to_uint64() const;

			// Hex conversion
//			[[nodiscard]] uint64_t hex_to_uint64() const; TODO

			// Operators
			String& operator =(String const& other);
			String operator +(String const& other) const;
			String& operator +=(String const& other);

			String operator *(int times) const;

			bool operator ==(String const& other) const;
			bool operator !=(String const& other) const;

			bool operator <(String const& other) const;
			bool operator >(String const& other) const;

			bool operator <=(String const& other) const;
			bool operator >=(String const& other) const;

			char& operator [](size_t index);
			char& operator [](size_t index) const;

	} string;   ///< Typedef for String

	/**
	 * @class StringBuilder
	 * @brief Creates a string using a using a combination of parts with the '<<' operator. Simmilar to the logger.
	 *
	 * @see Logger
	 */
	class StringBuilder {
		public:
			String out;                                 ///< The output string
			operator String() const { return out; }     ///< Convert to string

			StringBuilder& operator <<(char const* str);
			StringBuilder& operator <<(String const& other);
			StringBuilder& operator <<(int value);
			StringBuilder& operator <<(uint64_t value);
			StringBuilder& operator <<(bool value);

	};
}

// Convert functions
int strlen(const char* str);
char* itoa(int base, int64_t number);
char* htoa(uint64_t number);

// Compare functions
bool strcmp(char const* str1, char const* str2);
bool strcmp(char const* str1, MaxOS::String const& str2);
bool strcmp(MaxOS::String const& str1, char const* str2);
bool strcmp(MaxOS::String const& str1, MaxOS::String const& str2);

// Compare limited functions
bool strncmp(char const* str1, char const* str2, int length);
bool strncmp(char const* str1, MaxOS::String const& str2, int length);
bool strncmp(MaxOS::String const& str1, char const* str2, int length);
bool strncmp(MaxOS::String const& str1, MaxOS::String const& str2, int length);

#endif //MAXOS_STRING_H
