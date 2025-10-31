/**
 * @file string.h
 * @brief Defines a String class for dynamically sized strings with various operations
 *
 * @date 12th April 2023
 * @author Max Tyson
 */

#ifndef MAXOS_STRING_H
#define MAXOS_STRING_H

#include <stdint.h>

#include <common/vector.h>
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
          int m_length = 0;           // Does not include the null terminator

		  char m_small_string[MAX_STRING_SMALL_STORAGE] = {0};
		  bool m_using_small = true;

          [[nodiscard]] static int lex_value(String const &other) ;
          void allocate_self();

        public:

          String();
          explicit String(char c);
          String(char const* string);
          String(uint8_t const* string, int length);
          String(String const &other);
          explicit String(int value);
          explicit String(uint64_t value);
          explicit String(float value);
		  explicit String(bool value);
          ~String();

          void copy(String const &other);

		  static String formatted(char const *format, ...);
		  static String formatted(char const *format, va_list parameters);

          [[nodiscard]] int length(bool count_ansi = true) const;
          char* c_str();
          const char* c_str() const;

          bool starts_with(String const &other);
          [[nodiscard]] String substring(int start, int length) const;

          [[nodiscard]] common::Vector<String> split(String const &delimiter) const;
          [[nodiscard]] String strip(char strip_char = ' ') const;


          [[nodiscard]] String center(int width, char fill = ' ') const;

          // Operator functions
          [[nodiscard]] bool equals(String const &other) const;

          // Operators
          String &operator = (String const &other);
          String operator + (String const &other) const;
          String &operator += (String const &other);

          String operator * (int times) const;

          bool operator == (String const &other) const;
          bool operator != (String const &other) const;

          bool operator < (String const &other) const;
          bool operator > (String const &other) const;

          bool operator <= (String const &other) const;
          bool operator >= (String const &other) const;

          char& operator [] (int index);
          char& operator [] (int index) const;

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
			operator String() const { return out; } 	///< Convert to string

			StringBuilder& operator << (char const* str) { out += string(str); return *this; }   	///< Append C-string
			StringBuilder& operator << (String const &other) { out += other; return *this; }           	///< Append String
			StringBuilder& operator << (int value) { out += string(value); return *this; }              ///< Append int
			StringBuilder& operator << (uint64_t value) { out += string(value); return *this; }         ///< Append hex
			StringBuilder& operator << (float value) { out += string(value); return *this; }            ///< Append decimal
			StringBuilder& operator << (bool value) { out += string(value); return *this; }             ///< Append bool

	};
}

// Convert functions
int strlen(const char* str);
char* itoa(int base, int64_t  number);
char* htoa(uint64_t number);
char* ftoa(float number);

// Compare functions
bool strcmp(char const *str1, char const *str2);
bool strcmp(char const *str1, MaxOS::String const &str2);
bool strcmp(MaxOS::String const &str1, char const *str2);
bool strcmp(MaxOS::String const &str1, MaxOS::String const &str2);

// Compare limited functions
bool strncmp(char const *str1, char const *str2, int length);
bool strncmp(char const *str1, MaxOS::String const &str2, int length);
bool strncmp(MaxOS::String const &str1, char const *str2, int length);
bool strncmp(MaxOS::String const &str1, MaxOS::String const &str2, int length);

#endif //MAXOS_STRING_H
