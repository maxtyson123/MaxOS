cmake_minimum_required(VERSION 3.13)
project(maxOS)
set(CMAKE_CXX_STANDARD 14)

include_directories(SYSTEM .)

set(harwardCom_c kernel/src/hardwarecommunication)
set(harwardCom_h kernel/include/hardwarecommunication)

set(drivers_c kernel/src/drivers)
set(drivers_h kernel/include/drivers)

set(common_c kernel/src/common)
set(common_h kernel/include/common)

#Include the directories and now your cpp files will recognize your headers
include_directories(kernel/include)

set(
        SOURCE_FILES
        kernel/src/kernel.cpp
        kernel/src/gdt.cpp
        kernel/include/gdt.h
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


        kernel/include/drivers/vga.h kernel/src/drivers/vga.cpp kernel/include/gui/widget.h kernel/include/gui/window.h kernel/include/gui/desktop.h kernel/src/gui/desktop.cpp kernel/src/gui/widget.cpp kernel/src/gui/window.cpp kernel/include/common/graphicsContex.h kernel/include/gui/render.h kernel/src/gui/render.cpp kernel/include/gui/widgets/text.h kernel/src/gui/widgets/text.cpp kernel/include/multitasking.h kernel/src/multitasking.cpp)
add_executable(untitled ${SOURCE_FILES})