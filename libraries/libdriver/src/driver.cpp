/**
 * @file driver.cpp
 * @brief Implementation of a generic Driver class
 *
 * @date 24th November 2025
 * @author Max Tyson
*/

#include <driver.h>

using namespace LibDriver;
using namespace MaxOS;
using namespace MaxOS::KPI::ipc;

Driver::Driver() = default;

Driver::~Driver() = default;

/**
 * @brief Reset the driver
 *
 * @return How long in milliseconds it took to reset the driver
 */
uint32_t Driver::reset() {
	return 0;
}