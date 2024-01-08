//
// Created by 98max on 4/12/2023.
//

#ifndef MAXOS_STRING_H
#define MAXOS_STRING_H

#include <stdint.h>

namespace maxOS {

    class String {
        private:
          char* m_string;
          int m_length;

          void copy(String const &other);
          int lex_value(String const &other) const;

        public:

          String();
          String(char const *string);
          String(String const &other);
          String(int value);
          ~String();

          int length() const;
          const char* c_str() const;

          String &operator = (String const &other);
          String operator + (String const &other) const;
          String &operator += (String const &other);

          bool operator == (String const &other) const;
          bool operator != (String const &other) const;

          bool operator < (String const &other) const;
          bool operator > (String const &other) const;

          bool operator <= (String const &other) const;
          bool operator >= (String const &other) const;

          char operator [] (int index);
          char &operator [] (int index) const;
    };

    typedef String              string;

}

#endif //MAXOS_STRING_H
