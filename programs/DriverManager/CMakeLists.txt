MAKE_PROG_NOSTD(DriverManager src /boot/initrd)
TARGET_LINK_LIBRARIES(DriverManager.elf driver)
TARGET_LINK_LIBRARIES(DriverManager.elf fs)
GENERATE_SYMBOLS(DriverManager.elf)