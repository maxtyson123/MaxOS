MAKE_PROG_NOSTD(Init src /boot)
TARGET_LINK_LIBRARIES(Init.elf fs)

# Generate the symbols for debugging
SET(SYM_ELF ${CMAKE_CURRENT_BINARY_DIR}/Init.sym)
ADD_CUSTOM_COMMAND(
        TARGET Init.elf
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}  --only-keep-debug $<TARGET_FILE:Init.elf> ${SYM_ELF}
        COMMENT "Extracting debug symbols to ${SYM_ELF}"
        VERBATIM
)