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
 * @brief Activate the driver
 */
void Driver::activate() {

}

/**
 * @brief Deactivate the driver
 */
void Driver::deactivate() {

}

/**
 * @brief Initialise the driver
 */
void Driver::initialise() {

}

/**
 * @brief Reset the driver
 *
 * @return How long in milliseconds it took to reset the driver
 */
uint32_t Driver::reset() {
	return 0;
}

/**
 * @brief Get who created the device
 *
 * @return The vendor name of the driver
 */
string Driver::vendor_name() {
	return "Generic";
}

/**
 * @brief Get the device name of the driver
 *
 * @return The device name of the driver
 */
string Driver::device_name() {
	return "Unknown Driver";
}