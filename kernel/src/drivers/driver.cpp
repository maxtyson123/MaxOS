//
// Created by 98max on 11/10/2022.
//

#include <drivers/driver.h>
//using namespace maxOS::common;
using namespace maxOS::drivers;

Driver::Driver(){

};

Driver::~Driver(){

};

void Driver::Activate(){

}

void Driver::Deactivate(){

}

int Driver::Reset(){

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
