MAKE_PROG_NOSTD(FileServer src /boot)
TARGET_LINK_LIBRARIES(FileServer.elf driver)
TARGET_LINK_LIBRARIES(FileServer.elf fs)