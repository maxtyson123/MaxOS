MAKE_PROG_NOSTD(ATADriver src /boot/initrd)
TARGET_LINK_LIBRARIES(ATADriver.elf driver fs kpi fs)
