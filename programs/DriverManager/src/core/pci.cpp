/**
 * @file pci.cpp
 * @brief Implementation of a Peripheral Component Interconnect (PCI) controller and device descriptor
 *
 * @date 12th October 2022
 * @author Max Tyson
 */

#include <libfs/file.h>
#include <libcommon/assert.h>

#include <core/pci.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::KPI;
using namespace MaxOS::KPI::processes;
using namespace DriverManager;
using namespace DriverManager::core;
using namespace LibDriver;
using namespace LibDriver::HardwareCommunication;
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

HardwareRangeType PCIController::get_range_type(BaseAddressRegister& bar) {

	// Bar empty
	if (bar.address == nullptr || bar.size == 0)
		return HardwareRangeType::NONE;

	switch (bar.type) {

		case BARType::InputOutput:
			return HardwareRangeType::PORT;

		case BARType::MemoryMapped:
			return HardwareRangeType::MEMORY;

	}

	return HardwareRangeType::NONE;

}

Vector<string> PCIController::get_class_string_parts(const pci_device_descriptor_t &dev) {


	// Convert into the expected format
	string class_id			= string((uint64_t)dev.class_id).to_lower().padleft("0",2);
	string subclass_id		= string((uint64_t)dev.subclass_id).to_lower().padleft("0",2);
	string interface_id		= string((uint64_t)dev.interface_id).to_lower().padleft("0",2);

	string sclass = "";
	string subclass = "";
	string interface = "";

	size_t start_line = 0;

	// Get any cached names
	if (check_cache_entry(m_class_name_cache, dev.class_id, sclass, start_line))
		if (check_cache_entry(m_subclass_name_cache, dev.subclass_id, subclass, start_line))
			check_cache_entry(m_interface_name_cache, dev.interface_id, interface, start_line);

	// Find the start of the class section
	if (start_line == 0)
		for (size_t i = m_pci_id_lines.size(); i > 0; --i)
			if (m_pci_id_lines[i].starts_with("C 00")) {
				start_line = i;
				break;
			}

	// Parse the file
	for (size_t i = start_line; i < m_pci_id_lines.size(); ++i)
	{

		// Skip comments
		auto line = m_pci_id_lines[i];
		if (line.starts_with("#") || line.length() == 0)
			continue;

		// Searching for vendor
		if (sclass == ""){

			// Skip non vendor entries
			if (!line.starts_with("C"))
				continue;

			// Not the vendor
			if (!line.starts_with("C "s + class_id))
				continue;

			// Extract the vendor
			sclass = line.substring(6, line.length() - 6);
			m_class_name_cache.insert(dev.class_id, {i, sclass});

			continue;
		}

		// Vendor must have been found so now looking at devices
		if (subclass == "")
		{

			// No more device entries (not found)
			if (!line.starts_with("\t"))
				break;

			// Not the device
			if (!line.starts_with("\t"s + subclass_id))
				continue;

			subclass = line.substring(5, line.length() - 5);
			m_subclass_name_cache.insert(dev.subclass_id, {i, subclass});

			continue;
		}

		// Device must have been found so now looking at subvendors
		if (interface == "")
		{

			// No more subvendor entries (not found)
			if (!line.starts_with("\t\t"))
				break;

			// Not the device
			if (!line.starts_with("\t\t"s + interface_id))
				continue;

			interface = line.substring(6, line.length() - 6);
			m_interface_name_cache.insert(dev.sub_vendor_id, {i, interface});

			break;
		}
	}

	// Replace not found entries with the ids
	if (sclass == "")
		sclass = class_id;
	if (subclass == "")
		subclass = subclass_id;
	if (interface == "" && interface_id != "00")
		interface = interface_id;

	return {sclass, subclass, interface};

}

string PCIController::get_class_string(const pci_device_descriptor_t& dev) {
	auto parts = get_class_string_parts(dev);
	return parts[0] + ": " + parts[1];
}

Vector<string> PCIController::get_pci_id_string_parts(const pci_device_descriptor_t &dev) {

	// Convert into the expected format
	string vendor_id  = string((uint64_t)dev.vendor_id).to_lower().padleft("0",4);
	string device_id  = string((uint64_t)dev.device_id).to_lower().padleft("0",4);
	string sub_vendor_id  = string((uint64_t)dev.sub_vendor_id).to_lower().padleft("0",4);

	string vendor = "";
	string device = "";
	string subvendor = "";

	size_t start_line = 0;

	// Get any cached names
	if (check_cache_entry(m_vendor_name_cache, dev.vendor_id, vendor, start_line))
		if (check_cache_entry(m_device_name_cache, dev.device_id, device, start_line))
			check_cache_entry(m_subvendor_name_cache, dev.sub_vendor_id, subvendor, start_line);


	// Parse the file
	for (size_t i = start_line; i < m_pci_id_lines.size(); ++i)
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

			// Extract the vendor
			vendor = line.substring(6, line.length() - 6);
			m_vendor_name_cache.insert(dev.vendor_id, {i, vendor});

			continue;
		}

		// Vendor must have been found so now looking at devices
		if (device == "")
		{

			// No more device entries (not found)
			if (!line.starts_with("\t"))
				break;

			// Not the device
			if (!line.starts_with("\t"s + device_id))
				continue;

			device = line.substring(7, line.length() - 7);
			m_device_name_cache.insert(dev.device_id, {i, device});

			continue;
		}

		// Device must have been found so now looking at subvendors
		if (subvendor == "")
		{

			// No more subvendor entries (not found)
			if (!line.starts_with("\t\t"))
				break;

			// Not the device
			if (!line.starts_with("\t\t"s + sub_vendor_id))
				continue;

			subvendor = line.substring(13, line.length() - 13);
			m_subvendor_name_cache.insert(dev.sub_vendor_id, {i, subvendor});

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

	return {vendor, device, subvendor};

}

string PCIController::get_pci_id_string(const pci_device_descriptor_t& dev)
{
	auto parts = get_pci_id_string_parts(dev);
	return parts[0] + " " + parts[1] + " " + parts[2];

}

device_identification_t PCIController::pci_desc_to_dev_info(const pci_device_descriptor_t &dev) {

	auto parts = get_pci_id_string_parts(dev);

	return {

		.model = {
			.vendor = dev.vendor_id,
			.device = dev.device_id,
			.revision = dev.sub_vendor_id,
		},

		.class_info = {
			.base		= dev.class_id,
			.sub		= dev.subclass_id,
			.interface	= dev.interface_id,
		},

		.driver_type = DriverType::UNKNOWN,

		.vendor_name = parts[0],
		.device_name = parts[1] + " ("  + parts[2] +  ")",

		.class_string = get_class_string(dev),
	};
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
				PCIDeviceDescriptor device_descriptor = get_device_descriptor(bus, device, function);
				if (device_descriptor.vendor_id == 0x0000 || device_descriptor.vendor_id == 0x0001 ||
					device_descriptor.vendor_id == 0xFFFF)
					continue;

				// Build the hardware ranges from the BAR
				hardware_mapping_t hmap;
				for (int bar_num = 0; bar_num < 6; bar_num++)
				{
					BaseAddressRegister bar = get_base_address_register(bus, device, function, bar_num);
					hardware_range_t* range = &hmap.ranges[bar_num];

					range -> address_base		= (uint64_t)bar.address;
					range -> address_length		= bar.size;
					range -> type				= get_range_type(bar);
				}
				hmap.irq = device_descriptor.interrupt;

				// Store the device
				auto info = pci_desc_to_dev_info(device_descriptor);
				auto device = new Device(info, hmap);
				handler->on_device_enumerated(device);

				klog("DEVICE FOUND: %s - %s %s\n", info.class_string.c_str(), info.vendor_name.c_str(), info.device_name.c_str());
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