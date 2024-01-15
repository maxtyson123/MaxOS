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

};

Driver::~Driver(){
    this ->m_driver_message_stream = 0;
};

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
void Driver::error_message(string message) {

    // If there is a driver message stream write the message to it
    if(m_driver_message_stream != 0)
        m_driver_message_stream-> write(message);

}

/**
 * @brief write a character to the driver message stream if it is not null
 *
 * @param char_to_write The character to write
 */
void Driver::error_message(char char_to_write) {

    // If there is a driver message stream write the character to it
    if(m_driver_message_stream != 0)
      m_driver_message_stream-> write_char(char_to_write);

}


/**
 * @brief write an integer to the driver message stream if it is not null
 *
 * @param int_to_write The integer to write
 */
void Driver::error_message(int int_to_write) {

    // If there is a driver message stream write the integer to it
    if(m_driver_message_stream != 0)
            m_driver_message_stream-> write_int(int_to_write);
}

/**
 * @brief write a hex to the driver message stream if it is not null
 *
 * @param hex_to_write The hex to write
 */
void Driver::error_message(uint32_t hex_to_write) {

    // If there is a driver message stream write the hex to it
    if(m_driver_message_stream != 0)
      m_driver_message_stream->write_hex(hex_to_write);

}

/**
 * @brief Get the vendor name of the driver
 *
 * @return The vendor name of the driver
 */
string Driver::get_vendor_name()
{
    return "Generic";
}

/**
 * @brief Get the device name of the driver
 *
 * @return The device name of the driver
 */
string Driver::get_device_name()
{
    return "Unknown Driver";
}

DriverSelectorEventHandler::DriverSelectorEventHandler()
{
}

DriverSelectorEventHandler::~DriverSelectorEventHandler()
{
}

/**
 * @brief This function is called when a driver is selected
 *
 * @param driver The driver that was selected
 */
void DriverSelectorEventHandler::on_driver_selected(Driver*)
{
}

DriverSelector::DriverSelector()
{
}

DriverSelector::~DriverSelector()
{
}

/**
 * @brief Select the drivers
 */
void DriverSelector::select_drivers(DriverSelectorEventHandler*, hardwarecommunication::InterruptManager*)
{
}

DriverManager::DriverManager() {

}

DriverManager::~DriverManager() {

    // Remove any drivers that are still attached
    while (!drivers.empty())
       remove_driver(*drivers.begin());

}

/**
 * @brief Adds a driver to the manager
 *
 * @param driver The driver to add
 */
void DriverManager::add_driver(Driver* driver){
  drivers.push_back(driver);
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
    drivers.erase(driver);

}

/**
 * @brief When a driver is selected add it to the manager
 */
void DriverManager::on_driver_selected(Driver* driver) {
  add_driver(driver);
}