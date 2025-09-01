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
		int len = 0;
		for (; str[len] != '\0'; len++);
		return len;
	}

}

#endif //SYSCORE_COMMON_H
