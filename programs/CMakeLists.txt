#TODO: now that have the STD lib no need for this, just do MAKE_PROG
FUNCTION(MAKE_PROG_NOSTD TARGET_NAME SRC_DIR)

    # Get the location arg
    SET(_LOCATION "/os/apps")
    IF(ARGC GREATER 2)
        SET(_LOCATION "${ARGV2}")
    ENDIF()

    # Set compiler flags freestanding no stdlib
    SET(CMAKE_CXX_FLAGS "-ffreestanding -fno-exceptions -fno-rtti -fpermissive -nostdlib -Wall -Wextra -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel -g -fno-use-cxa-atexit" CACHE STRING "" FORCE)

    # Find all .cpp and .s files recursively in the given source directory
    FILE(GLOB_RECURSE PROJ_SRCS ${SRC_DIR}/*.cpp ${SRC_DIR}/*.s)

    # Create the executable
    ADD_EXECUTABLE(${TARGET_NAME}.elf ${PROJ_SRCS})

    # Set the linker script path
    SET(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/linker.ld)

    # Ensure the target rebuilds if the linker script changes
    SET_TARGET_PROPERTIES(${TARGET_NAME}.elf PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

    # Link libraries and pass linker options
    TARGET_LINK_LIBRARIES(${TARGET_NAME}.elf gcc kpi)
    TARGET_LINK_OPTIONS(${TARGET_NAME}.elf PRIVATE -T ${LINKER_SCRIPT} -nostdlib -n)
    TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME}.elf PRIVATE include)

    # Install the executable to the boot filesystem
    INSTALL(TARGETS ${TARGET_NAME}.elf DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/${_LOCATION})
ENDFUNCTION()

FUNCTION(GENERATE_SYMBOLS TARGET_NAME)

    SET(SYM_ELF ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.sym)
    ADD_CUSTOM_COMMAND(
            TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}  --only-keep-debug $<TARGET_FILE:${TARGET_NAME}> ${SYM_ELF}
            VERBATIM
    )

ENDFUNCTION()

ADD_SUB_DIRS(${CMAKE_CURRENT_SOURCE_DIR})