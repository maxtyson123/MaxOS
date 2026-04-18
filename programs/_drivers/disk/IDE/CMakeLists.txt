MAKE_PROG_NOSTD(IDEDriver src /boot/initrd)
TARGET_LINK_LIBRARIES(IDEDriver.elf driver fs kpi fs)
