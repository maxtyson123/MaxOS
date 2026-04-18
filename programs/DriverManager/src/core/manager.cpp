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

	// Free the driver selectors
	for(auto& enumerator : m_device_enumerators)
		delete enumerator;

}


/**
 * @brief  Check if all the found drivers have been started
 *
 * @return True if all the drivers found by the manager have been started
 */
bool Manager::all_drivers_started() {

	for (const auto& device : m_devices)
		if (!device->driver_started)
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
 * @brief When a driver is selected add it to the manager
 *
 * @param driver The driver that was selected
 */
void Manager::on_device_enumerated(Device* driver) {
	register_device(driver);
}

Device * Manager::get_device(int id) {

	for (const auto& device : m_devices)
		if (device -> id == id)
			return device;

	return nullptr;
}


void Manager::register_device(Device *device) {

	device -> id = m_next_device_id++;
	m_devices.push_back(device);

}

/**
 * @brief Start the drivers any devices that haven't been started
 */
void Manager::start_drivers() {

	for (const auto& device : m_devices)
		if (!device->driver_started)
			device->start_driver();

}

void Manager::start_disks() {

}
