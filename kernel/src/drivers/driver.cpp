//
// Created by 98max on 11/10/2022.
//

#include <drivers/driver.h>
using namespace maxOS::common;
using namespace maxOS::drivers;

Driver::Driver(OutputStream* driverMessageStream){

    // Set the driver message stream
    this -> driverMessageStream = driverMessageStream;

};

Driver::~Driver(){
    this -> driverMessageStream = 0;
};

void Driver::Activate(){

}

void Driver::Deactivate(){

}

int Driver::Reset(){

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


DriverManager::DriverManager(){
    numDrivers = 0;
}

/**
 * @details This function adds a driver to the driver manager
 *
 * @param driver The driver to add
 */
void DriverManager::AddDriver(Driver* drv){
    drivers[numDrivers] = drv;
    numDrivers++;
}
/**
 * @details This function activates all the drivers in the driver manager
 */
void DriverManager::ActivateAll(){
    for(int i = 0; i < numDrivers; i++){
        drivers[i]->Activate();
    }
}
