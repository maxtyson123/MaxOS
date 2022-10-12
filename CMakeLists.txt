cmake_minimum_required(VERSION 3.13)
project(maxos)
set(CMAKE_CXX_STANDARD 14)

include_directories(SYSTEM .)

set(harwardCom_c ./src/hardwarecommunication)
set(harwardCom_h ./include/hardwarecommunication)

set(drivers_c ./src/drivers)
set(drivers_h ./include/drivers)

set(common_c ./src/common)
set(common_h ./include/common)

#Include the directories and now your cpp files will recognize your headers
include_directories(./include)

set(
        SOURCE_FILES
        ./src/kernel.cpp
        ./src/gdt.cpp
        ./include/gdt.h
        ${harwardCom_h}/pci.h
        ${harwardCom_c}/pci.cpp
        ${harwardCom_h}/port.h
        ${harwardCom_c}/port.cpp
        ${harwardCom_h}/interrupts.h
        ${harwardCom_c}/interrupts.cpp
        ${drivers_h}/driver.h
        ${drivers_c}/driver.cpp
        ${drivers_h}/mouse.h
        ${drivers_c}/mouse.cpp
        ${drivers_h}/keyboard.h
        ${drivers_c}/keyboard.cpp
        ${common_h}/types.h


)
add_executable(untitled ${SOURCE_FILES})