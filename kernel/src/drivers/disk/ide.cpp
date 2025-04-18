//
// Created by Max Tyson on 18/04/2025.
//
#include <drivers/disk/ide.h>

using namespace MaxOS;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::disk;

IntegratedDriveElectronicsController::IntegratedDriveElectronicsController(PeripheralComponentInterconnectDeviceDescriptor* device_descriptor)
: primary_maser(0x1F0, true),
  primary_slave(0x1F0, false),
  secondary_maser(0x170, true),
  secondary_slave(0x170, false)
{

}

IntegratedDriveElectronicsController::~IntegratedDriveElectronicsController() = default;

/**
 * @brief Initialise the IDE controller by identifying the devices
 */
void IntegratedDriveElectronicsController::initialise()
{
  primary_maser.identify();
  primary_slave.identify();
  secondary_maser.identify();
  secondary_slave.identify();
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