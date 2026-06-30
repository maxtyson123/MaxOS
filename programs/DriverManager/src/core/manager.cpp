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
using namespace MaxOS::KPI::processes;
using namespace MaxOS::common;


/**
 * @brief Constructs the Driver Manager, adding any default driver selectors
 */
Manager::Manager() {

	add_device_enumerator(new PCIController);
	// add_driver_selector(new UniversalSerialBusController);

	// Load the inital drivers
	load_driver_list("/0/boot/initrd/initdrivers.json");
}

/**
 * @brief Destructor for the Driver Manager, removes all drivers and driver selectors
 */
Manager::~Manager() {

	// Free the driver selectors
	for(auto& enumerator : m_device_enumerators)
		delete enumerator;

}

void Manager::load_driver_list(string path) {

	// Read the list of inital devices
	auto jhandle = open_file(path.c_str());
	auto size = file_size(jhandle);
	auto jsonstr = string((uint8_t*)allocate_memory(size), size);
	file_read(jhandle, jsonstr.c_str(), size);

	// Parse the json
	JSONParser parser(&jsonstr);
	JSONNode* m_initial_drivers = parser.root();
	parse_driver_list(m_initial_drivers);

	// Clean up
	close_file(jhandle);
	delete jsonstr.c_str();

}

void Manager::parse_driver_list(JSONNode* list) {

	// Cache each driver
	auto drivers = (*list)["drivers"s];
	for (int i = 0; i < drivers.array_size(); ++i) {

		auto& driver = drivers[i];
		auto& device_node = driver["device"s];
		auto& model = device_node["model"s];

		// Parse
		string name = driver["name"s];
		string path = driver["file"s];
		string vendor = model["vendor"s];
		string device = model["device"s];
		string revision = model["revision"s];

		// Build the device id
		device_identification_t id {};
		id.model = {
			.vendor		= (uint16_t)vendor.hex_to_uint64(),
			.device		= (uint16_t)device.hex_to_uint64(),
			.revision	= (uint16_t)revision.hex_to_uint64(),
		};

		// Build the driver id
		driver_entry_t entry {
			.name =  name,
			.path =  path,
		};

		// Cache
		m_driver_list.insert(id, entry);
	}
}

void Manager::start_driver(Device* device) {

	ASSERT(device != nullptr, "Cant start a driver without an associated device");

	// Driver already started
	if (device->driver_started)
		return;

	// Get the driver details for this device
	auto it = m_driver_list.find(device->id_info());
	if (it == m_driver_list.end())
		return;

	// Pass the ID
	const char** args =  new const char*[1];
	args[0] = string(device->id).c_str();

	// Start the driver
	auto driver = it->second;
	exec_file(driver.name.c_str(), driver.path.c_str(), args, 1);

	device->driver_started = true;
	klog("Started driver: %s (%s) for device %d\n", driver.name.c_str(), driver.path.c_str(), device->id);
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
			start_driver(device);

}