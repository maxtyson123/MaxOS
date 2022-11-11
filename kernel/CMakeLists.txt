cmake_minimum_required(VERSION 3.13)
project(maxOS)
set(CMAKE_CXX_STANDARD 14)

include_directories(SYSTEM ..)

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

#Include the directories and now your cpp files will recognize your headers
include_directories(include)

set(
        SOURCE_FILES
        src/kernel.cpp
        include/gdt.h src/gdt.cpp
        include/multitasking.h src/multitasking.cpp
        include/memorymanagement.h src/memorymanagement.cpp
        include/syscalls.h src/syscalls.cpp

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

)
add_executable(maxOS ${SOURCE_FILES})