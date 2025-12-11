/**
 * @file manager.h
 * @brief Implements the Manager class for managing drivers
 *
 * @date 11th October 2022
 * @author Max Tyson
 */

#include <core/manager.h>

using namespace DriverManager;
using namespace DriverManager::core;
using namespace LibDriver;
using namespace MaxOS::common;

/**
 * @brief Constructs the Driver Manager, adding any default driver selectors
 */
Manager::Manager() {

	// TODO: libc support for this
	// add_driver_selector(new PCIController);
	// add_driver_selector(new UniversalSerialBusController);
}

/**
 * @brief Destructor for the Driver Manager, removes all drivers and driver selectors
 */
Manager::~Manager() {

	// Remove any drivers that are still attached
	while(!m_drivers.empty())
		remove_driver(*m_drivers.begin());

	// Free the driver selectors
	for(auto& driver_selector : m_driver_selectors)
		delete driver_selector;

}

/**
 * @brief Adds a driver to the manager
 *
 * @param driver The driver to add
 */
void Manager::add_driver(Driver* driver) {
	m_drivers.push_back(driver);
}

/**
 * @brief Removes a driver from the driver vector
 *
 * @param driver The driver to remove
 */
void Manager::remove_driver(Driver* driver) {

	driver->deactivate();
	m_drivers.erase(driver);

}

/**
 * @brief When a driver is selected add it to the manager
 *
 * @param driver The driver that was selected
 */
void Manager::on_driver_selected(Driver* driver) {
	add_driver(driver);
}

/**
 * @brief Add a driver selector to the manager
 *
 * @param driver_selector The driver selector to add
 */
void Manager::add_driver_selector(Selector* driver_selector) {

	m_driver_selectors.push_back(driver_selector);

}

/**
 * @brief Remove a driver selector from the manager
 *
 * @param driver_selector The driver selector to remove
 */
void Manager::remove_driver_selector(Selector* driver_selector) {

	m_driver_selectors.erase(driver_selector);

}

/**
 * @brief Find the drivers
 */
void Manager::find_drivers() {

	// Select the drivers
	for(auto& driver_selector : m_driver_selectors)
		driver_selector->select_drivers(this);
}

/**
 * @brief Reset all the devices
 *
 * @return The longest time it takes to reset a device
 */
uint32_t Manager::reset_devices() {

	uint32_t reset_wait_time = 0;
	for(auto& driver : m_drivers) {
		// Reset the driver
		uint32_t wait_time = driver->reset();

		// If the wait time is longer than the current longest wait time, set it as the new longest wait time
		if(wait_time > reset_wait_time)
			reset_wait_time = wait_time;
	}

	return reset_wait_time;
}

/**
 * @brief Initialise the drivers
 */
void Manager::initialise_drivers() {

	for(auto& driver : m_drivers)
		driver->initialise();

}

/**
 * @brief Deactivate the drivers
 */
void Manager::deactivate_drivers() {

	for(auto& driver : m_drivers)
		driver->deactivate();

}

/**
 * @brief Activate the drivers
 */
void Manager::activate_drivers() {

	for(auto& driver : m_drivers)
		driver->activate();

}