//
// Created by Max Tyson on 18/04/2025.
//
#include <drivers/disk/ide.h>


using namespace MaxOS;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::disk;
using namespace MaxOS::filesystem;
using namespace MaxOS::filesystem::partition;

IntegratedDriveElectronicsController::IntegratedDriveElectronicsController(PeripheralComponentInterconnectDeviceDescriptor* device_descriptor)
{
  // TODO: Use the device descriptor to get the port base and add the devices dynamically

  // Primary
  auto primary_maser = new AdvancedTechnologyAttachment(0x1F0, true);
  auto primary_slave = new AdvancedTechnologyAttachment(0x1F0, false);
  devices.insert(primary_maser, true);
  devices.insert(primary_slave, false);

  // Secondary
  auto secondary_maser = new AdvancedTechnologyAttachment(0x170, true);
  auto secondary_slave = new AdvancedTechnologyAttachment(0x170, false);
  devices.insert(secondary_maser, true);
  devices.insert(secondary_slave, false);

}

IntegratedDriveElectronicsController::~IntegratedDriveElectronicsController() = default;

/**
 * @brief Initialise the IDE controller by identifying the devices
 */
void IntegratedDriveElectronicsController::initialise()
{

  // Loop through the devices and identify them
  for(auto& device : devices)
  {

    // Get the ata device
    auto ata_device = device.first;

    // Check if the device is present
    if(ata_device == nullptr)
      continue;

    // Identify the device
    bool exists = ata_device->identify();

    // Remove the device if it does not exist
    if(!exists)
    {
      devices.erase(ata_device);
      delete ata_device;
      continue;
    }
  }

  // Log the init done
  Logger::DEBUG() << "IDE Controller: Initialised " << devices.size() << " devices\n";
}

/**
 * @brief Activate the IDE controller by mounting the devices to the virtual file system
 */
void IntegratedDriveElectronicsController::activate()
{


  // Loop through the devices and load the partitions
  for(auto& device : devices)
  {

    // Ensure there is a device and that it is the master
    if(device.first == nullptr || !device.second)
      continue;

    // Mount the device
    MSDOSPartition::mount_partitions(device.first);

  }
}


/**
 * @brief Get the vendor name
 */
string IntegratedDriveElectronicsController::vendor_name()
{
  return "Intel";
}

/**
 * @brief Get the device name
 */
string IntegratedDriveElectronicsController::device_name()
{
  return "PIIX4";
}