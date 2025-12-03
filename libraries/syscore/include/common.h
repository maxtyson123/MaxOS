//
// Created by 98max on 9/1/2025.
//

#ifndef SYSCORE_COMMON_H
#define SYSCORE_COMMON_H

namespace syscore{

	/**
	* @brief Gets the length of a string
	*
	* @param str The string to get the length of
	* @return The length of the string
	*/
	inline int strlen(const char *str) {

		if(str == nullptr)
			return 0;

		int len = 0;
		for (; str[len] != '\0'; len++);
		return len;
	}

	/**
	 * @brief Copies memory from source to destination
	 *
	 * @param dest The destination pointer
	 * @param src The source pointer
	 * @param n The number of bytes to copy
	 * @return The destination pointer
	 */
	inline void* memcpy(void* dest, const void* src, size_t n) {
		auto* d = static_cast<uint8_t*>(dest);
		auto* s = static_cast<const uint8_t*>(src);
		for (size_t i = 0; i < n; ++i) {
			d[i] = s[i];
		}
		return dest;
	}

	/**
	 * @brief Compares two strings
	 *
	 * @param str1 The first string
	 * @param str2 The second string
	 * @return 0 if the strings are equal, a negative value if str1 is less than str2, or a positive value if str1 is greater than str2
	 */
	inline int strcmp(const char* str1, const char* str2) {
		while (*str1 && (*str1 == *str2)) {
			str1++;
			str2++;
		}
		return *(unsigned char*)str1 - *(unsigned char*)str2;
	}


	/**
	 * @brief Converts integer to string
	 *
	 * @param base The base of the number (10 for decimal, 16 for hex)
	 * @param number The number to convert
	 *
	 * @return The converted string
	 */
	inline char* itoa(int base, int64_t number) {

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
	inline char* htoa(uint64_t number) {
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

}

#endif //SYSCORE_COMMON_H
