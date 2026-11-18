cmake_minimum_required(VERSION 3.20.0)

set(PRJ_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

list(APPEND BOARD_ROOT ${PRJ_ROOT})
list(APPEND DTS_ROOT   ${PRJ_ROOT})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ltc4281_hotswap_controller)

target_sources(app PRIVATE
    src/main.cpp
    src/adc_utils.c
    src/tmp101_utils.c
    src/hotswap_controller.cpp
    src/led_utils.c
    ${PRJ_ROOT}/drivers/ltc4281/ltc4281.c
)

target_include_directories(app PRIVATE ${PRJ_ROOT}/drivers/ltc4281)