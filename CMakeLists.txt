cmake_minimum_required(VERSION 3.20.0)

set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/board)
set(BOARD lpcxpresso55s69_unsecure)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

SET(USE_USB_WITH_ZPHYR_RTOS FALSE)

IF (USE_USB_WITH_ZPHYR_RTOS)
add_definitions(-DFSL_RTOS_ZEPHYR=1) 
SET(SRC src/host_msd_command_task.c)
ELSE()
SET(SRC src/host_msd_command.c)
ENDIF (USE_USB_WITH_ZPHYR_RTOS)

include(${CMAKE_SOURCE_DIR}/nxp_mcux_usb_host/build.cmake)

project(lpc55-test-usb-host C CXX ASM)
target_sources(app PRIVATE 
${nxp_mcux_usb_host_SRC}
${SRC}
src/main.c
)
