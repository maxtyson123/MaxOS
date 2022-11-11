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

#Include the directories and now your cpp files will recognize your headers
include_directories(include)

set(
        SOURCE_FILES
        src/kernel.cpp
        src/gdt.cpp
        include/gdt.h
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


        include/drivers/amd_am79c973.h src/drivers/amd_am79c973.cpp include/drivers/ata.h src/drivers/ata.cpp include/drivers/vga.h src/drivers/vga.cpp include/gui/widget.h include/gui/window.h include/gui/desktop.h src/gui/desktop.cpp src/gui/widget.cpp src/gui/window.cpp include/common/graphicsContext.h include/gui/render.h src/gui/render.cpp include/gui/widgets/text.h src/gui/widgets/text.cpp include/multitasking.h src/multitasking.cpp include/memorymanagement.h src/memorymanagement.cpp include/syscalls.h src/syscalls.cpp include/net/etherframe.h src/net/etherframe.cpp)
add_executable(untitled ${SOURCE_FILES})