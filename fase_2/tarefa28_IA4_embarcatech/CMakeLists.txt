# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(main C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# ------------------------------------------------------------------------------
# pico-tflmicro
#   Your error log shows Ninja compiling pico-tflmicro *tests* (very long paths)
#   and failing to create dependency files on Windows.
#   So we explicitly disable pico-tflmicro tests/examples/benchmarks and add the
#   subdirectory as EXCLUDE_FROM_ALL.
# ------------------------------------------------------------------------------

# Common CMake test switch
set(BUILD_TESTING OFF CACHE BOOL "Disable building tests" FORCE)

# These option names are used by pico-tflmicro in many versions; if a given
# variable is unknown it is simply ignored by CMake.
set(PICO_TFLMICRO_BUILD_TESTS OFF CACHE BOOL "Disable pico-tflmicro tests" FORCE)
set(PICO_TFLMICRO_BUILD_EXAMPLES OFF CACHE BOOL "Disable pico-tflmicro examples" FORCE)
set(PICO_TFLMICRO_BUILD_BENCHMARKS OFF CACHE BOOL "Disable pico-tflmicro benchmarks" FORCE)

# Add pico-tflmicro (vendored in ./lib/pico-tflmicro)
add_subdirectory(lib/pico-tflmicro pico-tflmicro-build EXCLUDE_FROM_ALL)

# Detect the correct pico-tflmicro library target name (varies by fork/version)
set(TFLM_TARGET "")
foreach(candidate IN ITEMS pico_tflmicro pico-tflmicro tflmicro pico_tflmicro_lib)
    if(TARGET ${candidate})
        set(TFLM_TARGET ${candidate})
        break()
    endif()
endforeach()

if(TFLM_TARGET STREQUAL "")
    message(FATAL_ERROR
        "Could not find a pico-tflmicro library target after add_subdirectory(). "
        "Expected one of: pico_tflmicro, pico-tflmicro, tflmicro, pico_tflmicro_lib."
    )
endif()

# ------------------------------------------------------------------------------
# Your application
# ------------------------------------------------------------------------------


# Add executable. Default name is the project name, version 0.1

add_executable(${PROJECT_NAME}
        src/main.c
        src/tflm_wrapper.cpp
        lib/button/button.c # Button library
        lib/led/led.c # LED library
        lib/ssd1306/ssd1306.c # SSD1306 library
        lib/ssd1306/display.c # Display library
        lib/ws2812b/ws2812b.c # WS2812B library
        lib/ws2812b/led_matrix_numbers.c # LED matrix numbers library
)

pico_set_program_name(${PROJECT_NAME} "base")
pico_set_program_version(${PROJECT_NAME} "0.1")

# Generate PIO header
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/lib/ws2812b/pio/ws2812b.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 1)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

# Add the standard library to the build
target_link_libraries(${PROJECT_NAME}
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/lib
)

# Add any user requested libraries
target_link_libraries(${PROJECT_NAME}
        hardware_i2c
        hardware_pio
        hardware_timer
        hardware_clocks
        hardware_adc
        hardware_pwm
        ${TFLM_TARGET}
        )

pico_add_extra_outputs(${PROJECT_NAME})

