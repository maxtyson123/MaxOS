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


using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::KPI;
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
	m_pci_ids_file_handle = open_file("/initrd/pci.ids");
	ASSERT(m_pci_ids_file_handle != 0, "Cant open PCI ids file\n");

	// Read the pci ids
	auto size = file_size(m_pci_ids_file_handle);
	m_pci_ids = (char*)allocate_memory(size);
	klog("Allocated PCI ids file at 0x%x with size of 0x%x\n", m_pci_ids, size);

	file_read(m_pci_ids_file_handle, m_pci_ids, size);

	klog("PCI IDS: %s", m_pci_ids);
}

PCIController::~PCIController()
{
	// Clean up
	delete m_pci_ids;
	close_file(m_pci_ids_file_handle);
}

string PCIController::get_pci_id_string(const pci_device_descriptor_t& dev) const
{
	return "";
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

LibDriver::DriverType PCIDevice::get_driver_type(const pci_device_descriptor_t& device_descriptor)
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

				klog("DEVICE FOUND: %s - %s\n", get_class_string(device_descriptor).c_str(),
				     get_pci_id_string(device_descriptor).c_str());
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
 * @brief Get the driver for the device
 *
 * @param dev Device descriptor
 * @return Driver for the device, null pointer if there is no driver
 */
// Driver* PCIController::get_driver(PCIDeviceDescriptor dev) {
//
// 	switch (dev.vendor_id) {
// 		case 0x1022:    //AMD
// 		{
// 			switch (dev.device_id) {
// 				case 0x2000: {
// 					return new AMD_AM79C973(&dev);
//
// 				}
// 				default:
// 					break;
// 			}
// 			break;
// 		}
// 		case 0x8086:  //Intel
// 		{
// 			switch (dev.device_id) {
//
// 				case 0x100E: //i217 (Ethernet Controller)
// 				{
// 					return new IntelI217(&dev);
// 				}
//
// 				case 0x7010: // PIIX4 (IDE Controller)
// 				{
// 					return new IntegratedDriveElectronicsController(&dev);
// 				}
//
// 				default:
// 					break;
// 			}
// 			break;
// 		}//End Intel
// 	}
//
// 	//If there is no driver for the particular device, go into generic devices
// 	switch (dev.class_id) {
// 		case 0x03: //Graphics
// 		{
//
// 			switch (dev.subclass_id) {
// 				case 0x00:  //VGA
// 				{
// 					return new VideoGraphicsArray();
// 				}
// 			}
// 			break;
// 		}
// 	}
//
// 	return nullptr;
// }

/**
 * @brief Print the vednor and device id of known devices, or "Unknown" + their ids if not known.
 *
 * @param dev The device to print
 */
// void PCIController::list_known_device( const PCIDeviceDescriptor& dev) {
//
// 	switch (dev.vendor_id) {
// 		case 0x1022: {
// 			// The vendor is AMD
// 			Logger::Out() << "AMD ";
//
// 			// List the device
// 			switch (dev.device_id) {
// 				default:
// 					Logger::Out() << "0x%x" << dev.device_id;
// 					break;
// 			}
// 			break;
// 		}
//
// 		case 0x106B: {
// 			// The vendor is Apple
// 			Logger::Out() << "Apple ";
//
// 			// List the device
// 			switch (dev.device_id) {
// 				case 0x003F: {
// 					Logger::Out() << "KeyLargo/Intrepid USB";
// 					break;
// 				}
//
// 				default:
// 					Logger::Out() << "0x%x" << dev.device_id;
// 					break;
// 			}
// 			break;
// 		}
//
// 		case 1234: {
// 			// The vendor is QEMU
// 			Logger::Out() << "QEMU ";
//
// 			// List the device
// 			switch (dev.device_id) {
//
// 				case 0x1111: {
// 					Logger::Out() << "Virtual Video Controller";
// 					break;
// 				}
// 			}
// 			break;
// 		}
//
// 		case 0x8086: {
// 			// The vendor is Intel
// 			Logger::Out() << "Intel ";
//
// 			// List the device
// 			switch (dev.device_id) {
//
// 				case 0x1237: {
// 					Logger::Out() << "440FX";
// 					break;
// 				}
//
// 				case 0x2415: {
// 					Logger::Out() << "AC'97";
// 					break;
// 				}
//
// 				case 0x7000: {
// 					Logger::Out() << "PIIX3";
// 					break;
//
// 				}
//
// 				case 0x7111: {
// 					Logger::Out() << "PIIX3 ACPI";
// 					break;
// 				}
//
// 				case 0x7113: {
// 					Logger::Out() << "PIIX4 ACPI";
// 					break;
// 				}
//
// 				default:
// 					Logger::Out() << "0x%x" << dev.device_id;
// 					break;
//
// 			}
// 			break;
// 		}
//
// 		case 0x80EE: {
//
// 			// The vendor is VirtualBox
// 			Logger::Out() << "VirtualBox ";
//
// 			// List the device
// 			switch (dev.device_id) {
//
// 				case 0xBEEF: {
// 					Logger::Out() << "Graphics Adapter";
// 					break;
// 				}
//
// 				case 0xCAFE: {
// 					Logger::Out() << "Guest Service";
// 					break;
// 				}
// 			}
// 			break;
// 		}
//
// 			// Unknown
// 		default:
// 			Logger::Out() << "Unknown (0x" << dev.vendor_id << ":0x" << dev.device_id << ")";
// 			break;
//
// 	}
// }

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


/**
 * @brief Get the type of the device
 *
 * @todo See wiki for more types to add
 *
 * @return Type of the device as a string (or Unknown if the type is not known)
 */
string DriverManager::core::PCIController::get_class_string(const pci_device_descriptor_t& dev) const
{
	switch (dev.class_id)
	{
	case 0x00:
		return (dev.subclass_id == 0x01) ? "VGA" : "Legacy";
	case 0x01:
		switch (dev.subclass_id)
		{
		case 0x01:
			return "IDE interface";
		case 0x06:
			return "SATA controller";
		default:
			return "Storage";
		}
	case 0x02:
		return "Network";
	case 0x03:
		return "Display";
	case 0x04:
		switch (dev.subclass_id)
		{
		case 0x00:
			return "Video";
		case 0x01:
		case 0x03:
			return "Audio";
		default:
			return "Multimedia";
		}
	case 0x06:
		switch (dev.subclass_id)
		{
		case 0x00:
			return "Host bridge";
		case 0x01:
			return "ISA bridge";
		case 0x04:
			return "PCI bridge";
		default:
			return "Bridge";
		}
	case 0x07:
		switch (dev.subclass_id)
		{
		case 0x00:
			return "Serial controller";
		case 0x80:
			return "Communication controller";
		}
		break;
	case 0x0C:
		switch (dev.subclass_id)
		{
		case 0x03:
			return "USB";
		case 0x05:
			return "System Management Bus";
		}
		break;
	}
	return "Unknown";
}
