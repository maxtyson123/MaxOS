/**
 * @file pci.cpp
 * @brief Implementation of a Peripheral Component Interconnect (PCI) controller and device descriptor
 *
 * @date 12th October 2022
 * @author Max Tyson
 */

#include <core/pci.h>
#include <libfs/include/file.h>
#include <assert.h>

#include "processes/thread.h"


using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::KPI;
using namespace MaxOS::KPI::processes;
using namespace DriverManager;
using namespace DriverManager::core;
using namespace LibDriver;
using namespace LibFS;

/**
 * @brief Construct a new PCI Controller object
 */
PCIController::PCIController()
: m_data_port(0xCFC),
  m_command_port(0xCF8)
{

	// Try open the pci ids
	uint64_t pci_ids_file_handle = open_file("/boot/initrd/pci.ids");
	ASSERT(pci_ids_file_handle != 0, "Cant open PCI ids file\n");

	// Read the pci ids
	auto size = file_size(pci_ids_file_handle);
	auto pci_ids = (char*)allocate_memory(size);
	file_read(pci_ids_file_handle, pci_ids, size);

	// Split into lines so easier to parse
	m_pci_id_lines = string(pci_ids).split("\n");

	// Clean up
	delete pci_ids;
	close_file(pci_ids_file_handle);

}

PCIController::~PCIController() = default;

string PCIController::get_class_string(const pci_device_descriptor_t& dev) {
	return "CLASS";
}

string PCIController::get_pci_id_string(const pci_device_descriptor_t& dev)
{

	// Get any cached names @todo store in cache
	auto vendor_cached = m_vendor_name_cache.find((uint8_t)dev.vendor_id);
	auto device_cached = m_device_name_cache.find((uint8_t)dev.device_id);
	auto subvendor_cached = m_subvendor_name_cache.find((uint8_t)dev.sub_vendor_id);

	// All found
	if (vendor_cached != m_subvendor_name_cache.end() && device_cached != m_subvendor_name_cache.end() && subvendor_cached != m_subvendor_name_cache.end())
		return vendor_cached->second + " " + device_cached->second + " " + subvendor_cached->second;

	// Convert into the expected format
	string vendor_id  = string((uint64_t)dev.vendor_id).to_lower().padleft("0",4);
	string device_id  = string((uint64_t)dev.device_id).to_lower().padleft("0",4);
	string sub_vendor_id  = string((uint64_t)dev.sub_vendor_id).to_lower().padleft("0",4);

	string vendor = "";
	string device = "";
	string subvendor = "";

	// Parse the file
	for (size_t i = 0; i < m_pci_id_lines.size(); ++i)
	{

		// Skip comments
		auto line = m_pci_id_lines[i];
		if (line.starts_with("#") || line.length() == 0)
			continue;

		// End of file
		if (line.starts_with("C"))
			break;

		// Searching for vendor
		if (vendor == ""){

			// Skip non vendor entries
			if (line.starts_with("\t"))
				continue;

			// Not the vendor
			if (!line.starts_with(vendor_id))
				continue;

			vendor = line.substring(6, line.length() - 6);
			continue;
		}

		// Vendor must have been found so now looking at devices
		if (device == "")
		{

			// No more device entries (not found)
			if (!line.starts_with("\t"))
				break;

			// Not the device
			if (!line.starts_with(string("\t") + device_id))
				continue;

			device = line.substring(7, line.length() - 7);
			continue;
		}

		// Device must have been found so now looking at subvendors (@todo subsytem)
		if (subvendor == "")
		{

			// No more subvendor entries (not found)
			if (!line.starts_with("\t\t"))
				break;

			// Not the device
			if (!line.starts_with(string("\t\t") + sub_vendor_id))
				continue;

			subvendor = line.substring(13, line.length() - 13);
			break;
		}
	}

	// Replace not found entries with the ids
	if (vendor == "")
		vendor = vendor_id;
	if (device == "")
		device = device_id;
	if (subvendor == "" && sub_vendor_id != "0000")
		subvendor = sub_vendor_id;

	return vendor + " " + device + " " + subvendor;

}

Driver* PCIDevice::handle_driver_start()
{
	//@todo
	return nullptr;
}

PCIDevice::PCIDevice(pci_device_descriptor_t device_descriptor)
	: Device(get_driver_type(device_descriptor)),
	  m_device_descriptor(device_descriptor)
{
}

PCIDevice::~PCIDevice() = default;

bool PCIDevice::builtin_driver()
{
	return false;
}

DriverType PCIDevice::get_driver_type(const pci_device_descriptor_t& device_descriptor)
{
	return DriverType::UNKNOWN;
}

/**
 * @brief read data from the PCI Controller
 *
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @param register_offset Register offset
 * @return data from the PCI Controller
 */
uint32_t PCIController::read(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset)
{
	// Construct the id
	uint32_t id = 0x1 << 31
		| ((bus & 0xFF) << 16)
		| ((device & 0x1F) << 11)
		| ((function & 0x07) << 8)
		| (register_offset & 0xFC);
	m_command_port.write(id);

	// Read the data from the port
	uint32_t result = m_data_port.read();
	return result >> (8 * (register_offset % 4));
}

/**
 * @brief write data to the PCI Controller
 *
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @param register_offset Register offset
 * @param value Value to write
 */
void PCIController::write(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset, uint32_t value)
{
	// Construct the id
	uint32_t id = 0x1 << 31
		| ((bus & 0xFF) << 16)
		| ((device & 0x1F) << 11)
		| ((function & 0x07) << 8)
		| (register_offset & 0xFC);
	m_command_port.write(id);

	// Write the data to the port
	m_data_port.write(value);
}

/**
 * @brief Check if the device has a function
 *
 * @param bus Bus number
 * @param device Device number
 * @return true if the device has a function
 */
bool PCIController::device_has_functions(uint16_t bus, uint16_t device)
{
	return read(bus, device, 0, 0x0E) & (1 << 7);
}

/**
 * @brief Select the driver for the device
 *
 * @param handler device driver event manager
 */
void PCIController::enumerate_devices(DeviceEnumeratorEventHandler* handler)
{
	for (int bus = 0; bus < 8; ++bus)
	{
		for (int device = 0; device < 32; ++device)
		{
			int num_functions = (device_has_functions(bus, device)) ? 8 : 1;

			for (int function = 0; function < num_functions; ++function)
			{
				// Get the device descriptor, if the vendor id is 0x0000 or 0xFFFF, the device is not present/ready
				PCIDeviceDescriptor device_descriptor = get_device_descriptor(bus, device,
				                                                              function);
				if (device_descriptor.vendor_id == 0x0000 || device_descriptor.vendor_id == 0x0001 ||
					device_descriptor.vendor_id == 0xFFFF)
					continue;

				// Get the earliest port number
				for (int bar_num = 5; bar_num >= 0; bar_num--)
				{
					BaseAddressRegister bar = get_base_address_register(bus, device, function, bar_num);
					if (bar.address && (bar.type == BARType::InputOutput))
						device_descriptor.port_base = (uint64_t)bar.address;
				}

				klog("DEVICE FOUND: %s - %s\n", get_class_string(device_descriptor).c_str(), get_pci_id_string(device_descriptor).c_str());
				handler->on_device_enumerated(new PCIDevice(device_descriptor));
			}
		}
	}
}

/**
 * @brief Get the device descriptor
 *
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @return Device descriptor
 */
PCIDeviceDescriptor PCIController::get_device_descriptor(uint16_t bus, uint16_t device, uint16_t function)
{
	PCIDeviceDescriptor result;

	result.bus = bus;
	result.device = device;
	result.function = function;

	result.vendor_id = read(bus, device, function, 0x00);
	result.device_id = read(bus, device, function, 0x02);
	result.sub_vendor_id = read(bus, device, function, 0x2C);

	result.class_id = read(bus, device, function, 0x0B);
	result.subclass_id = read(bus, device, function, 0x0A);
	result.interface_id = read(bus, device, function, 0x09);

	result.revision = read(bus, device, function, 0x8);
	result.interrupt = read(bus, device, function, 0x3C);

	return result;
}

/**
 * @brief Get the base address register
 *
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @param barNum base address register number
 * @return base address register
 */
BaseAddressRegister PCIController::get_base_address_register(uint16_t bus, uint16_t device, uint16_t function,
                                                             uint16_t bar)
{
	BaseAddressRegister result{};

	// Only types 0x00 (normal devices) and 0x01 (PCI-to-PCI bridges) are supported:
	uint32_t header_type = read(bus, device, function, 0x0E);
	if (header_type & 0x3F)
		return result;

	// read the base address register
	uint64_t bar_value = read(bus, device, function, 0x10 + 4 * bar);
	result.type = (bar_value & 0x1) ? BARType::InputOutput : BARType::MemoryMapped;
	result.address = (uint8_t*)(bar_value & ~0xF);

	// Read the size of the base address register
	write(bus, device, function, 0x10 + 4 * bar, 0xFFFFFFF0 | (int)result.type);
	result.size = read(bus, device, function, 0x10 + 4 * bar);
	result.size = (~result.size | 0xF) + 1;

	// Restore the original value of the base address register
	write(bus, device, function, 0x10 + 4 * bar, bar_value);

	return result;
}