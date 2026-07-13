cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Hostboard)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/dwin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/dwin_tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/freertos_hooks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/hostboard_registers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/detail_view_data.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/uart1_modbus_master.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/modbus_master_tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/modbus_polling.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/modbus_callbacks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/FreeRTOS/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/FreeRTOS/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/FreeRTOS/tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/FreeRTOS/timers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/FreeRTOS/event_groups.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/FreeRTOS/stream_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/port.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/FreeRTOS/portable/MemMang/heap_4.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/FreeRTOS/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/FreeRTOS/portable/GCC/ARM_CM4F
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# Post-build: generate .hex and .bin from .elf
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${CMAKE_PROJECT_NAME}> $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/${CMAKE_PROJECT_NAME}.bin
    COMMENT "Generating .hex and .bin from .elf"
)

# Flash target: cmake --build build/Debug --target flash
add_custom_target(flash
    COMMAND ${CMAKE_SOURCE_DIR}/flash.sh
    DEPENDS ${CMAKE_PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Flashing ${CMAKE_PROJECT_NAME}.elf via OpenOCD + ST-LINK/V2"
)

