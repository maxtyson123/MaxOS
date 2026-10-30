//
// Created by 98max on 19/04/2026.
//

#include <libcommon/math.h>

bool MaxOS::common::is_digit(char c) {
	return (c >= '0' && c <= '9') || c == '-';
}
