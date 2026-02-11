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

Device::Device(DriverType type)
: m_driver_type(type) {

}

Device::~Device() = default;

/**
 * @brief Check if the driver has been started and associated with this device
 *
 * @return True if the driver has been started
 *
 * @note If true, can assume driver() is not nullptr
 */
bool Device::driver_started() {
    return m_driver_started && driver();
}

/**
 * @brief Gets the type of driver associated with this device
 *
 * @return The type of driver
 */
DriverType Device::driver_type() {
    return m_driver_type;
}

/**
 * @brief Get the driver for this device
 *
 * @return The driver for the device or nullptr if not found / not ready yet
 */
Driver* Device::driver() {
    return m_driver;
}

void Device::start_driver() {

    m_driver = handle_driver_start();
    m_driver_started = m_driver != nullptr;

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