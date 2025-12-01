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

}

#endif //SYSCORE_COMMON_H
