//
// Created by 98max on 11/10/2022.
//

#include <drivers/driver.h>
#include <hardwarecommunication/pci.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::memory;
using namespace MaxOS::hardwarecommunication;

Driver::Driver() = default;

Driver::~Driver() = default;

/**
 * @brief activate the driver
 */
void Driver::activate() {

}

/**
 * @brief deactivate the driver
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

DriverSelectorEventHandler::DriverSelectorEventHandler() = default;

DriverSelectorEventHandler::~DriverSelectorEventHandler() = default;

/**
 * @brief This function is called when a driver is selected
 *
 * @param driver The driver that was selected
 */
void DriverSelectorEventHandler::on_driver_selected(Driver *) {
}

DriverSelector::DriverSelector() = default;

DriverSelector::~DriverSelector() = default;

/**
 * @brief Select the drivers
 */
void DriverSelector::select_drivers(DriverSelectorEventHandler *) {
}


DriverManager::DriverManager() {

	Logger::INFO() << "Setting up Driver Manager \n";
	add_driver_selector(new PeripheralComponentInterconnectController);
	// add_driver_selector(new UniversalSerialBusController);
}

DriverManager::~DriverManager() {

	// Remove any drivers that are still attached
	while (!m_drivers.empty())
		remove_driver(*m_drivers.begin());

	// Free the driver selectors
	for (auto &driver_selector: m_driver_selectors)
		delete driver_selector;

}

/**
 * @brief Adds a driver to the manager
 *
 * @param driver The driver to add
 */
void DriverManager::add_driver(Driver *driver) {
	m_drivers.push_back(driver);
}

/**
 * @brief Removes a driver from the driver vector
 *
 * @param driver The driver to remove
 */
void DriverManager::remove_driver(Driver *driver) {

	driver->deactivate();
	m_drivers.erase(driver);

}

/**
 * @brief When a driver is selected add it to the manager
 */
void DriverManager::on_driver_selected(Driver *driver) {
	add_driver(driver);
}

/**
 * @brief Add a driver selector to the manager
 */
void DriverManager::add_driver_selector(DriverSelector *driver_selector) {

	m_driver_selectors.push_back(driver_selector);

}

/**
 * @brief Remove a driver selector from the manager
 */
void DriverManager::remove_driver_selector(DriverSelector *driver_selector) {

	m_driver_selectors.erase(driver_selector);

}

/**
 * @brief Find the drivers
 */
void DriverManager::find_drivers() {

	Logger::INFO() << "Finding Drivers \n";

	// Select the drivers
	for (auto &driver_selector: m_driver_selectors)
		driver_selector->select_drivers(this);
}

/**
 * @brief Reset all the devices
 *
 * @return The longest time it takes to reset a device
 */
uint32_t DriverManager::reset_devices() {

	Logger::INFO() << "Resetting Devices \n";

	uint32_t resetWaitTime = 0;
	for (auto &driver: m_drivers) {
		// Reset the driver
		uint32_t waitTime = driver->reset();

		// If the wait time is longer than the current longest wait time, set it as the new longest wait time
		if (waitTime > resetWaitTime)
			resetWaitTime = waitTime;
	}

	return resetWaitTime;
}

/**
 * @brief Initialise the drivers
 */
void DriverManager::initialise_drivers() {

	Logger::INFO() << "Initialising Drivers \n";

	// Initialise the drivers
	for (auto &driver: m_drivers)
		driver->initialise();

}

/**
 * @brief Deactivate the drivers
 */
void DriverManager::deactivate_drivers() {

	// Deactivate the drivers
	for (auto &driver: m_drivers)
		driver->deactivate();

}

/**
 * @brief Activate the drivers
 */
void DriverManager::activate_drivers() {


	Logger::INFO() << "Activating Drivers \n";

	// Activate the drivers
	for (auto &driver: m_drivers)
		driver->activate();

}
