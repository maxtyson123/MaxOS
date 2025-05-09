//
// Created by 98max on 4/12/2023.
//

#ifndef MAXOS_STRING_H
#define MAXOS_STRING_H

#include <stdint.h>

namespace MaxOS {

    /**
     * @class String
     * @brief Dynamically sized string with various operations
     */
    class String {
        private:
          char* m_string = nullptr;
          int m_length = 0;

          [[nodiscard]] static int lex_value(String const &other) ;

        public:

          String();
          String(char const* string);
          String(String const &other);
          String(int value);
          String(uint64_t value);
          String(float value);
          ~String();

          void copy(String const &other);

          [[nodiscard]] int length(bool count_ansi = true) const;
          char* c_str();
          [[nodiscard]] const char* c_str() const;

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

    };

    typedef String              string;
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
