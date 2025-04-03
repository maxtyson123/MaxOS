//
// Created by 98max on 11/10/2022.
//

#include <drivers/driver.h>
using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::memory;

Driver::Driver(OutputStream* driverMessageStream)
: m_driver_message_stream(driverMessageStream) {

}

Driver::~Driver(){
    this ->m_driver_message_stream = nullptr;
}

/**
 * @brief activate the driver
 */
void Driver::activate(){

}

/**
 * @brief deactivate the driver
 */
void Driver::deactivate(){

}

/**
 * @brief Initialise the driver
 */
void Driver::initialise() {

}

/**
 * @brief Reset the driver
 *
 * @return How long in milliseconds it took to reset the driver
 */
uint32_t Driver::reset(){
    return 0;
}

/**
 * @brief write a message to the driver message stream if it is not null
 *
 * @param message The message to write
 */
void Driver::error_message(const string& message) const {

    // If there is a driver message stream write the message to it
    if(m_driver_message_stream != nullptr)
        m_driver_message_stream-> write(message);

}

/**
 * @brief write a character to the driver message stream if it is not null
 *
 * @param char_to_write The character to write
 */
void Driver::error_message(char char_to_write) const {

    // If there is a driver message stream write the character to it
    if(m_driver_message_stream != nullptr)
      m_driver_message_stream-> write_char(char_to_write);

}


/**
 * @brief write an integer to the driver message stream if it is not null
 *
 * @param int_to_write The integer to write
 */
void Driver::error_message(int int_to_write) const {

    // If there is a driver message stream write the integer to it
    if(m_driver_message_stream != nullptr)
            m_driver_message_stream-> write_int(int_to_write);
}

/**
 * @brief write a hex to the driver message stream if it is not null
 *
 * @param hex_to_write The hex to write
 */
void Driver::error_message(uint32_t hex_to_write) const {

    // If there is a driver message stream write the hex to it
    if(m_driver_message_stream != nullptr)
      m_driver_message_stream->write_hex(hex_to_write);

}

/**
 * @brief Get the vendor name of the driver
 *
 * @return The vendor name of the driver
 */
string Driver::vendor_name()
{
    return "Generic";
}

/**
 * @brief Get the device name of the driver
 *
 * @return The device name of the driver
 */
string Driver::device_name()
{
    return "Unknown Driver";
}

DriverSelectorEventHandler::DriverSelectorEventHandler()
= default;

DriverSelectorEventHandler::~DriverSelectorEventHandler()
= default;

/**
 * @brief This function is called when a driver is selected
 *
 * @param driver The driver that was selected
 */
void DriverSelectorEventHandler::on_driver_selected(Driver*)
{
}

DriverSelector::DriverSelector()
= default;

DriverSelector::~DriverSelector()
= default;

/**
 * @brief Select the drivers
 */
void DriverSelector::select_drivers(DriverSelectorEventHandler*)
{
}

DriverManager::DriverManager()
{

}

DriverManager::~DriverManager() {

    // Remove any drivers that are still attached
    while (!m_drivers.empty())
       remove_driver(*m_drivers.begin());

}

/**
 * @brief Adds a driver to the manager
 *
 * @param driver The driver to add
 */
void DriverManager::add_driver(Driver* driver){
  m_drivers.push_back(driver);
}

/**
 * @brief Removes a driver from the driver vector
 *
 * @param driver The driver to remove
 */
void DriverManager::remove_driver(Driver* driver) {

    // deactivate the driver
    driver -> deactivate();

    // Remove the driver
    m_drivers.erase(driver);

}

/**
 * @brief When a driver is selected add it to the manager
 */
void DriverManager::on_driver_selected(Driver* driver) {
  add_driver(driver);
}

/**
 * @brief Add a driver selector to the manager
 */
void DriverManager::add_driver_selector(DriverSelector* driver_selector) {

  // Add the driver selector
  m_driver_selectors.push_back(driver_selector);

}

/**
 * @brief Remove a driver selector from the manager
 */
void DriverManager::remove_driver_selector(DriverSelector* driver_selector) {

  // Remove the driver selector
  m_driver_selectors.erase(driver_selector);

}

/**
 * @brief Find the drivers
 */
void DriverManager::find_drivers() {

    // Select the drivers
    for(auto & driver_selector : m_driver_selectors)
        driver_selector -> select_drivers(this);
}

/**
 * @brief Reset all the devices
 *
 * @return The longest time it takes to reset a device
 */
uint32_t DriverManager::reset_devices() {

  uint32_t resetWaitTime = 0;
  for(auto & driver : m_drivers)
  {
    // Reset the driver
    uint32_t waitTime = driver->reset();

    // If the wait time is longer than the current longest wait time, set it as the new longest wait time
    if(waitTime > resetWaitTime)
      resetWaitTime = waitTime;
  }

  return resetWaitTime;
}

/**
 * @brief Initialise the drivers
 */
void DriverManager::initialise_drivers() {

  // Initialise the drivers
  for(auto & driver : m_drivers)
      driver->initialise();

}

/**
 * @brief Deactivate the drivers
 */
void DriverManager::deactivate_drivers() {

  // Deactivate the drivers
  for(auto & driver : m_drivers)
    driver->deactivate();

}

/**
 * @brief Activate the drivers
 */
void DriverManager::activate_drivers() {

  // Activate the drivers
  for(auto & driver : m_drivers)
      driver->activate();

}
