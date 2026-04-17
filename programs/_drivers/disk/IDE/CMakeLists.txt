MAKE_PROG_NOSTD(IDEDriver src /boot/initrd)
TARGET_LINK_LIBRARIES(IDEDriver.elf driver)
MESSAGE(STATUS "IDE Driver built")