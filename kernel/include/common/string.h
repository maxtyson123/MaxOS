//
// Created by 98max on 4/12/2023.
//

#ifndef MAXOS_STRING_H
#define MAXOS_STRING_H

#include <stdint.h>

#include <common/vector.h>
#include <stdarg.h>

namespace MaxOS {

    /**
     * @class String
     * @brief Dynamically sized string with various operations
     */
    typedef class String {
        private:
          char* m_string = nullptr;
          int m_length = 0;           // Does not include the null terminator

		  const static uint8_t s_small_storage = 0x99;
		  char m_small_string[s_small_storage] = {0};
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

    } string;

	class StringBuilder {
		public:
			String out;
			operator String() const { return out; }

			StringBuilder& operator <<(char const* str) { out += string(str); return *this; }
			StringBuilder& operator <<(String const &other) { out += other; return *this; }
			StringBuilder& operator <<(int value) { out += string(value); return *this; }
			StringBuilder& operator <<(uint64_t value) { out += string(value); return *this; }
			StringBuilder& operator <<(float value) { out += string(value); return *this; }
			StringBuilder& operator <<(bool value) { out += string(value); return *this; }

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
