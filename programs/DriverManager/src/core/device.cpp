/**
 * @file device.cpp
 * @brief Implements classes for device enumeration and handling in the Driver Manager
 *
 * @date 24th November 2025
 * @author Max Tyson
 */

#include <core/device.h>

using namespace DriverManager;
using namespace DriverManager::core;
using namespace LibDriver;
using namespace LibDriver::HardwareCommunication;

/**
 * @brief Internally start the driver for this device
 *
 * @return The driver for the device or nullptr if it failed to start
 *
 * @note This should be overridden by the device implementation
 */
Driver* Device::handle_driver_start() {
    return nullptr;
}

Device::Device(device_identification_t info, hardware_mapping_t hmap)
: m_id_info(info),
  m_hmap(hmap)
{

}

Device::~Device() = default;

device_identification_t Device::id_info() {
    return m_id_info;
}


/**
 * @brief Get the driver for this device
 *
 * @return The driver for the device or nullptr if not found / not ready yet
 */
Driver* Device::driver() {
    return m_driver;
}

hardware_mapping_t Device::hardware_mapping() {
    return m_hmap;
}

void Device::start_driver() {

    m_driver = handle_driver_start();

}

DeviceEnumeratorEventHandler::DeviceEnumeratorEventHandler() = default;

DeviceEnumeratorEventHandler::~DeviceEnumeratorEventHandler() = default;

/**
 * @brief This function is called when a device is found during enumeration
 *
 * @param device The driver that was found
 */
void DeviceEnumeratorEventHandler::on_device_enumerated(Device* device) {
}

DeviceEnumerator::DeviceEnumerator() = default;

DeviceEnumerator::~DeviceEnumerator() = default;

/**
 * @brief Enumerate the devices available on the system
 *
 * @param handler The event handler to notify when a device is enumerated
 */
void DeviceEnumerator::enumerate_devices(DeviceEnumeratorEventHandler* handler) {
}