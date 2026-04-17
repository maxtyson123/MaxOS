/**
 * @file manager.h
 * @brief Implements the Manager class for managing drivers
 *
 * @date 11th October 2022
 * @author Max Tyson
 */

#include <core/manager.h>

#include <core/pci.h>

using namespace DriverManager;
using namespace DriverManager::core;
using namespace LibDriver;
using namespace MaxOS::common;

/**
 * @brief Constructs the Driver Manager, adding any default driver selectors
 */
Manager::Manager() {

	add_device_enumerator(new PCIController);
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
	for(auto& enumerator : m_device_enumerators)
		delete enumerator;

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
void Manager::on_device_enumerated(Device* driver) {
	m_devices.push_back(driver);
}

/**
 * @brief  Check if all the found drivers have been started
 *
 * @return True if all the drivers found by the manager have been started
 */
bool Manager::all_drivers_started() {

	for (const auto& device : m_devices)
		if (!device->driver_started())
			return false;

	return true;

}

/**
 * @brief Add a device enumerator to the driver manager
 *
 * @param device_enumerator The driver selector to add
 */
void Manager::add_device_enumerator(DeviceEnumerator* device_enumerator) {

	m_device_enumerators.push_back(device_enumerator);

}

/**
 * @brief Remove a device enumerator from the manager
 *
* @param device_enumerator The driver selector to add
 */
void Manager::remove_device_enumerator(DeviceEnumerator* device_enumerator) {

	m_device_enumerators.erase(device_enumerator);

}

/**
 * @brief Find the devices
 */
void Manager::find_devices() {

	// Select the drivers
	for(auto& device_enumerator : m_device_enumerators)
		device_enumerator->enumerate_devices(this);
}

/**
 * @brief Start the drivers any devices that haven't been started
 */
void Manager::start_drivers() {

	for (const auto& device : m_devices)
		if (!device->driver_started())
			device->start_driver();

}

void Manager::start_disks() {

}
