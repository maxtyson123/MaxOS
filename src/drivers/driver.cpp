//
// Created by 98max on 11/10/2022.
//

#include <drivers/driver.h>
//using namespace maxos::common;
using namespace maxos::drivers;

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

void DriverManager::AddDriver(Driver* drv){
    drivers[numDrivers] = drv;
    numDrivers++;
}

void DriverManager::ActivateAll(){
    for(int i = 0; i < numDrivers; i++){
        drivers[i]->Activate();
    }
}
