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

set(sys_c src/system)
set(sys_h include/system)

set(mem_c src/memory)
set(mem_h include/memory)

#Include all the files
include_directories(include)
set(
        SOURCE_FILES
        src/kernel.cpp

        ${sys_h}/gdt.h ${sys_c}/gdt.cpp
        ${sys_h}/multitasking.h ${sys_c}/multitasking.cpp
        ${sys_h}/syscalls.h ${sys_c}/syscalls.cpp
        ${sys_h}/multithreading.h ${sys_c}/multithreading.cpp
        ${sys_h}/process.h ${sys_c}/process.cpp


        ${harwardCom_h}/pci.h ${harwardCom_c}/pci.cpp
        ${harwardCom_h}/port.h ${harwardCom_c}/port.cpp
        ${harwardCom_h}/interrupts.h ${harwardCom_c}/interrupts.cpp
        ${harwardCom_h}/serial.h ${harwardCom_c}/serial.cpp

        ${drivers_h}/driver.h ${drivers_c}/driver.cpp
        ${drivers_h}/peripherals/mouse.h ${drivers_c}/peripherals/mouse.cpp
        ${drivers_h}/peripherals/keyboard.h ${drivers_c}/peripherals/keyboard.cpp
        ${drivers_h}/ethernet/amd_am79c973.h ${drivers_c}/ethernet/amd_am79c973.cpp
        ${drivers_h}/ata.h ${drivers_c}/ata.cpp
        ${drivers_h}/vga.h ${drivers_c}/vga.cpp
        ${drivers_h}/ethernet/intel_i217.h ${drivers_c}/ethernet/intel_i217.cpp
        ${drivers_h}/ethernet/ethernet.h ${drivers_c}/ethernet/ethernet.cpp
        ${drivers_h}/ethernet/rawdatahandler.h ${drivers_c}/ethernet/rawdatahandler.cpp

        ${gui_h}/widget.h ${gui_c}/widget.cpp
        ${gui_h}/window.h ${gui_c}/window.cpp
        ${gui_h}/desktop.h ${gui_c}/desktop.cpp
        ${gui_h}/render.h ${gui_c}/render.cpp
        ${gui_h}/widgets/text.h ${gui_c}/widgets/text.cpp

        ${net_h}/etherframe.h ${net_c}/etherframe.cpp
        ${net_h}/arp.h ${net_c}/arp.cpp
        ${net_h}/ipv4.h ${net_c}/ipv4.cpp
        ${net_h}/icmp.h ${net_c}/icmp.cpp
        ${net_h}/udp.h ${net_c}/udp.cpp
        ${net_h}/tcp.h ${net_c}/tcp.cpp

        ${common_h}/types.h
        ${common_h}/types.h
        ${common_h}/graphicsContext.h
        ${common_h}/printf.h ${common_c}/printf.cpp
        ${common_h}/timer.h ${common_c}/timer.cpp
        ${common_h}/vector.h ${common_c}/vector_templates.cpp

        ${mem_h}/memorymanagement.h ${mem_c}/memorymanagement.cpp
        ${mem_h}/memoryinputoutput.h ${mem_c}/memoryinputoutput.cpp

        src/filesystem/msdospart.cpp include/filesystem/msdospart.h include/filesystem/fat32.h src/filesystem/fat32.cpp)
#Ignore standard library
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -Wl,nostdlib -Wl,nodefaultlibs")

add_executable(maxOS ${SOURCE_FILES})