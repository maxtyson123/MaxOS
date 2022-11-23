cmake_minimum_required(VERSION 3.13)
project(maxOS)
set(CMAKE_CXX_STANDARD 14)

include_directories(SYSTEM ..)

#Define shorthands for the source files
set(harwardCom_c src/hardwarecommunication)
set(harwardCom_h include/hardwarecommunication)

set(drivers_c src/drivers)
set(drivers_h include/drivers)

set(common_c src/common)
set(common_h include/common)

set(gui_c src/gui)
set(gui_h include/gui)

set(net_c src/net)
set(net_h include/net)

#Include all the files
include_directories(include)
set(
        SOURCE_FILES
        src/kernel.cpp
        include/system/gdt.h src/system/gdt.cpp
        include/system/multitasking.h src/system/multitasking.cpp
        include/system/memorymanagement.h src/system/memorymanagement.cpp
        include/system/syscalls.h src/system/syscalls.cpp

        ${harwardCom_h}/pci.h ${harwardCom_c}/pci.cpp
        ${harwardCom_h}/port.h ${harwardCom_c}/port.cpp
        ${harwardCom_h}/interrupts.h ${harwardCom_c}/interrupts.cpp

        ${drivers_h}/driver.h ${drivers_c}/driver.cpp
        ${drivers_h}/mouse.h ${drivers_c}/mouse.cpp
        ${drivers_h}/keyboard.h ${drivers_c}/keyboard.cpp
        ${drivers_h}/amd_am79c973.h ${drivers_c}/amd_am79c973.cpp
        ${drivers_h}/ata.h ${drivers_c}/ata.cpp
        ${drivers_h}/vga.h ${drivers_c}/vga.cpp

        ${gui_h}/widget.h ${gui_c}/widget.cpp
        ${gui_h}/window.h ${gui_c}/window.cpp
        ${gui_h}/desktop.h ${gui_c}/desktop.cpp
        ${gui_h}/render.h ${gui_c}/render.cpp
        ${gui_h}/widgets/text.h ${gui_c}/widgets/text.cpp

        ${net_h}/etherframe.h ${net_c}/etherframe.cpp
        ${net_h}/arp.h ${net_c}/arp.cpp

        ${common_h}/types.h
        ${common_h}/types.h
        ${common_h}/graphicsContext.h
        ${common_h}/printf.h ${common_c}/printf.cpp

        src/hardwarecommunication/serial.cpp include/hardwarecommunication/serial.h include/system/multithreading.h src/system/multithreading.cpp include/system/process.h src/system/process.cpp src/net/ipv4.cpp include/net/ipv4.h src/common/timer.cpp include/common/timer.h)

#Ignore standard library
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -Wl,nostdlib -Wl,nodefaultlibs")

add_executable(maxOS ${SOURCE_FILES})