cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Implantables_Nova_App_main)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME} C ASM)

# Additional compile options beyond what toolchain provides
add_compile_options(
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
)

# Include directories
include_directories(
    Core/App
    Core/App/module
    Core/Inc
    Core/Inc/st32
    Drivers/CMSIS/Device/ST/STM32U5xx/Include
    Drivers/CMSIS/Include
    Drivers/STM32U5xx_HAL_Driver/Inc
    Drivers/STM32U5xx_HAL_Driver/Inc/Legacy
)

# Project sources
set(PROJECT_SOURCES
    # Application sources
    Core/App/module/app.c
    Core/App/module/eedata.c
    Core/App/module/sampling.c
    Core/App/module/scan_data_process.c
    Core/App/module/serial_interface.c
    Core/App/ObjDict.c

    # Core sources
    Core/Src/st32/can_stm.c
    Core/Src/st32/timer_stm.c
    Core/Src/boot.c
    Core/Src/emcy.c
    Core/Src/lifegrd.c
    Core/Src/lss.c
    Core/Src/main.c
    Core/Src/nmtMaster.c
    Core/Src/nmtSlave.c
    Core/Src/objacces.c
    Core/Src/pdo.c
    Core/Src/sdo.c
    Core/Src/states.c
    Core/Src/stm32u5xx_hal_msp.c
    Core/Src/stm32u5xx_it.c
    Core/Src/sync.c
    Core/Src/syscalls.c
    Core/Src/sysmem.c
    Core/Src/system_stm32u5xx.c
    Core/Src/timer.c

    # HAL Driver sources
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_cortex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_dma.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_dma_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_exti.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_fdcan.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_flash.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_flash_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_gpio.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_gtzc.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_i2c.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_i2c_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_icache.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_pwr.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_pwr_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rcc.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rcc_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_spi.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_spi_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_tim.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_tim_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_uart.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_uart_ex.c

    # Startup file
    startup_stm32u5a5xx.s
)

# Compile definitions
add_compile_definitions(
    USE_HAL_DRIVER
    STM32U5A5xx
    $<$<CONFIG:Debug>:DEBUG>
)

# Create the executable
add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES})

# Generate hex and binary files
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
    COMMENT "Building ${CMAKE_PROJECT_NAME}.hex and ${CMAKE_PROJECT_NAME}.bin"
)

# Print executable size
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
)
