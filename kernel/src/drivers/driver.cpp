//
// Created by 98max on 11/10/2022.
//

#include <drivers/driver.h>
using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::memory;

Driver::Driver(OutputStream* driverMessageStream){

    // Set the driver message stream
    this -> driverMessageStream = driverMessageStream;

};

Driver::~Driver(){
    this -> driverMessageStream = 0;
};

void Driver::activate(){

}

void Driver::deactivate(){

}


void Driver::initialise() {

}


uint32_t Driver::reset(){
    return 0;
}

/**
 * @details This function writes a message to the driver message stream if it is not null
 * @param message  The message to write
 */
void Driver::errorMessage(string message) {

    // Check if the driver message stream is not null
    if( driverMessageStream != 0) {
        // Write the message to the driver message stream
        driverMessageStream -> write(message);
    }

}

/**
 * @details This function writes a character to the driver message stream if it is not null
 * @param charToWrite The character to write
 */
void Driver::errorMessage(char charToWrite) {

    // Check if the driver message stream is not null
    if( driverMessageStream != 0) {
        // Write the character to the driver message stream
        driverMessageStream -> writeChar(charToWrite);
    }

}


/**
 * @details This function writes a int to the driver message stream if it is not null
 * @param intToWrite  The int to write
 */
void Driver::errorMessage(int intToWrite) {

    // Check if the driver message stream is not null
    if( driverMessageStream != 0) {
        // Write the character to the driver message stream
        driverMessageStream -> writeInt(intToWrite);
    }
}

/**
 * @details This function writes a hex to the driver message stream if it is not null
 * @param hexToWrite  The hex to write
 */
void Driver::errorMessage(uint32_t hexToWrite) {

    // Check if the driver message stream is not null
    if( driverMessageStream != 0) {
        // Write the character to the driver message stream
        driverMessageStream -> writeHex(hexToWrite);
    }

}

string Driver::getVendorName()
{
    return "Generic";
}

string Driver::getDeviceName()
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
 * @details This function is called when a driver is selected
 * @param driver The driver that was selected
 */
void DriverSelectorEventHandler::onDriverSelected(Driver*)
{
}

DriverSelector::DriverSelector()
{
}

DriverSelector::~DriverSelector()
{
}

void DriverSelector::selectDrivers(DriverSelectorEventHandler*, MemoryManager*, hardwarecommunication::InterruptManager*, OutputStream*)
{
}

DriverManager::DriverManager() {

}

DriverManager::~DriverManager() {

    // While there are still drivers in the driver vector
    while (!drivers.empty()) {

       // Remove the driver
        removeDriver(*drivers.begin());
    }

}

/**
 * @details Adds a driver to the driver vector
 * @param driver The driver to add
 */
void DriverManager::addDriver(Driver* driver) {
    drivers.pushBack(driver);
}

/**
 * @details Removes a driver from the driver vector
 * @param driver The driver to remove
 */
void DriverManager::removeDriver(Driver* driver) {

    // Deactivate the driver
    driver -> deactivate();

    // Remove the driver from the driver vector
    drivers.erase(driver);

}

/**
 * @details When a driver is selected, add it to the driver vector
 */
void DriverManager::onDriverSelected(Driver* driver) {
    addDriver(driver);
}
