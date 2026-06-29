/**
 * @file manager.h
 * @brief Implements the Manager class for managing drivers
 *
 * @date 11th October 2022
 * @author Max Tyson
 */

#include <libfs/file.h>
#include <libcommon/json.h>
#include <libcommon/mem.h>

#include <core/manager.h>
#include <core/pci.h>

using namespace DriverManager;
using namespace DriverManager::core;
using namespace LibFS;
using namespace LibDriver;
using namespace MaxOS;
using namespace MaxOS::KPI;
using namespace MaxOS::common;

/**
 * @brief Constructs the Driver Manager, adding any default driver selectors
 */
Manager::Manager() {

	add_device_enumerator(new PCIController);
	// add_driver_selector(new UniversalSerialBusController);

	// Read the list of inital devices
	auto jhandle = open_file("/boot/initrd/initdrivers.json");
	auto size = file_size(jhandle);
	auto jsonstr = string((uint8_t*)allocate_memory(size), size);
	file_read(jhandle, jsonstr.c_str(), size);

	// Parse the json
	JSONParser parser(&jsonstr);
	m_initial_drivers = parser.root();

	// Clean uo
	close_file(jhandle);
	delete jsonstr.c_str();
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

void Manager::start_inital_drivers() {

	// Start each driver
	auto drivers = (*m_initial_drivers)["drivers"s];
	for (int i = 0; i < drivers.array_size(); ++i) {
		auto driver = drivers[i];
		auto model = driver["device"s]["model"s];

		// Parse
		string name = driver["name"s];
		string path = driver["file"s];
		string vendor_str = model["vendor"s];
		string device_str = model["device"s];
		int vendor_id = vendor_str.to_int();
		int device_id = device_str.to_int();

		// Get the device
		Device* device = nullptr;
		for (const auto& dev : m_devices) {

			auto info = dev->id_info().model;

			if (info.vendor == vendor_id && info.device == device_id) {
				device = dev;
				break;
			}
		}

		// No device found
		if (!device)
			break;

		klog("Starting driver: %s\n", name.c_str());

	}
}
